/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Polygon.h"

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/Faceter.h"
#include "Active/Geometry/XList.h"
#include "Active/Geometry/LinEquation.h"
#include "Active/Geometry/PolyEdge.h"
#include "Active/Geometry/Rotater.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>

using namespace active::geometry;
using namespace active::math;

using enum XPoint::Role;

namespace {

	/*--------------------------------------------------------------------
		Get the top polygon vertex ID

		source: The target polygon
		
		return: The top vertex ID
	  --------------------------------------------------------------------*/
	vertex_id getTopVertexID(Polygon& source) {
		vertex_id maxID = 0;
		for (auto hole = source.getHoleSize() + 1; hole--; ) {
			const auto& poly = source.getShape(hole);
			for (auto vertex = poly.vertSize(); vertex--; ) {
				if (poly[vertex]->id > maxID)
					maxID = poly[vertex]->id;
			}
		}
		return maxID;
	} //getTopVertexID


	/*--------------------------------------------------------------------
		Get offsets along the edges at specified polygon vertices
		
		poly: The target polygon
		next: The index of the next vertex index
		prev: The index of the pervious vertex index
		ref: The equation of the reference line
		prec: The required precision
	  --------------------------------------------------------------------*/
	std::pair<PolyPoint, PolyPoint> getVertexOffsets(const Polygon& poly, vertex_index next, vertex_index prev,
						const LinEquation* ref = nullptr, double prec = eps) {
			//Default to the next/prec vertices (valid when the edge is not an arc)
		PolyPoint nextOffset = *poly[next];
		PolyPoint prevOffset = *poly[prev];
			//Otherwise (for arc edges) we need a point along the arc not colinear with the ref line
		if (poly[prev + 1]->isArc(prec)) {
				//Find a non-colinear point starting from the previous arc
			Arc refArc{prevOffset, *poly[prev + 1]};
			double refAngle = refArc.startAngle + refArc.sweep +
					((refArc.sweep < 0) ? pi / 2.0 : -pi / 2.0);
			if ((ref != nullptr) && isZero(fMod(ref->azimuthAngle() - angleMod(refAngle), pi, prec), prec))
				prevOffset = refArc.midpoint();
			else
				prevOffset = *poly[prev + 1] + Point(cos(refAngle), sin(refAngle));
		}
		if (poly[next]->isArc(prec)) {
				//Find a non-colinear point starting from the next arc
			Arc refArc{*poly[next - 1], nextOffset};
			double refAngle = refArc.startAngle + ((refArc.sweep < 0) ? -pi / 2.0 : pi / 2.0);
			if ((ref != nullptr) && isZero(fMod(ref->azimuthAngle() - angleMod(refAngle), pi, prec), prec))
				nextOffset = refArc.midpoint();
			else
				nextOffset = *poly[next - 1] + Point(cos(refAngle), sin(refAngle));
		}
		return std::make_pair(nextOffset, prevOffset);
	} //getEdgeVectors

	
	/*--------------------------------------------------------------------
		Get a polygon edge midpoint
		
		poly: The target polygon
		vertex: The index of the edge end vertex
		
		return: The edge midpoint
	  --------------------------------------------------------------------*/
	Point getEdgeMidpoint(const Polygon& poly, vertex_index vertex, vertex_index inc = 1) {
		auto endVertex = vertex + inc;
		if (inc < 0)
			std::swap(vertex, endVertex);
		return PolyEdge{*poly[vertex], *poly[endVertex]}.midpoint();
	} //getEdgeMidpoint


	/*--------------------------------------------------------------------
		Determine if a polygon reflects from a line at a specified point (NB: edges must be split at the intersection)
		
		poly: The target polygon
		ref: The reference line
		interPt: The intersection point
		prec: The required precision
	 
		return: True if the polygon reflects at the intersection
	  --------------------------------------------------------------------*/
	bool isPolyReflection(const Polygon& poly, const LinEquation& ref, const XPoint& interPt, double prec = eps) {
		if ((interPt.getPos(target) == Point::along) && interPt.getVertex(::target))
			return false;
		auto obj = interPt.getPart(target).value_or(0);
		const auto& target = poly.getShape(obj);
		auto index = target.findVertexById(*interPt.getVertex(::target));
		if (!index)
			return true;
			//Test the position of the edges with respect to the reference line
		auto former(getEdgeMidpoint(target, index->vertex, -1)), latter(getEdgeMidpoint(target, index->vertex));
		auto pos1 = ref.positionOf(former, prec / 10), pos2 = ref.positionOf(latter, prec / 10);
		bool isAlong1 = (pos1 == Point::along), isAlong2 = (pos2 == Point::along);
			//The edge positions will match for a reflection
		if (isAlong1 && isAlong2)
			return true;
			//Opposite directions is not a reflection
		if (!(isAlong1 || isAlong2) && ((pos1 != pos2) || target.isHole))
			return false;
		auto ref2 = LinEquation::create(former, interPt);
		if (!ref2)
			return true;
		return (ref2->positionOf(latter) == ((obj == 0) ? Point::right : Point::left));
	} //isPolyReflection
	
	
	/*--------------------------------------------------------------------
		Determine if two polygons reflect at a specified point (NB: edges must be split at the intersection)
		
		poly1: A target polygon
		poly2: A blade polygon
		interPt: The intersection point
		prec: The required precision
	 
		return: True if the polygons reflect at a specified point
	  --------------------------------------------------------------------*/
	bool isPolyReflection(const Polygon& target, const Polygon& blade, const XPoint& interPt, double prec = eps) {
		if (!interPt.getPart(::blade) || !interPt.getVertex(::blade))
			return false;
		auto obj = *interPt.getPart(::blade);
		const auto& bladeObject = blade.getShape(obj);
		auto index = bladeObject.findVertexById(*interPt.getVertex(::blade));
		if (!index)
			return true;
		auto whereBefore = target.positionOf(getEdgeMidpoint(bladeObject, index->vertex, -1), prec / 2),
				whereAfter = target.positionOf(getEdgeMidpoint(bladeObject, index->vertex), prec / 2);
		return !((whereBefore == Point::inside) ^ (whereAfter == Point::inside));
	} //isPolyReflection
	
	
	/*--------------------------------------------------------------------
		Extract a polygon path by travelling along a specified polygon until an intersection is reached
		
		source: The source polygon
		vert: The start vertex
		startPt: The starting point
		inc: The polygon iterator increment (direction)
		poly: The polygon to build
		inter: A list of intersections
		prec: The required precision
		
		return: True if path is extracted
	  --------------------------------------------------------------------*/
	bool extractPolygonPath(const Polygon& source, vertex_index vert, XList::iterator& startPt,
							vertex_index inc, Polygon& poly, XList& inter, double prec) {
		bool isOpen = false;
			//Repeat until an intersection is reached
		do {
				//Get the next point along the polygon
			auto pt = clone(*source[vert += inc]);
			if (inc < 0.0)
				pt->sweep = -source[vert - inc]->sweep;
			isOpen = !pt->isEqual2D(*poly[0], prec);
			if (isOpen) {
				poly.emplace_back(pt);
					//Test for a matching	 intersection
				for (startPt = inter.begin(); (startPt != inter.end()) && !pt->isEqual2D(**startPt, prec); ++startPt) {}
			} else {
				poly[0]->sweep = pt->sweep;
			}
		} while (isOpen && (startPt == inter.end()));
		return !isOpen;
	} //extractPolygonPath
	
	
	/*--------------------------------------------------------------------
		Determine which direction a polygon offcut should be travelled

		startPt: The intersection starting point
		target: The target polygon
		blade: The blade polygon
			
		return: The travel direction (Point::undefined if no solution)
	  --------------------------------------------------------------------*/
	Position getTravelDirection(const XPoint& startPt, Polygon& target, Polygon& blade, double prec) {
		auto vert = startPt.getVertex(::target);
		auto part = startPt.getPart(::target);
		if (!vert || !part)
			return Point::undefined;
		auto& targetSource = target.getShape(*part);
			//NB: The intersection vertex is usually an index, but in this case has been swapped for the vertex ID
		auto index = targetSource.findVertexById(*vert);
		if (!index)
			return Point::undefined;
		auto targetIn = getEdgeMidpoint(targetSource, index->vertex, -1),
				targetOut = getEdgeMidpoint(targetSource, index->vertex, 1);
		XPoint offsetIn{blade.closestPointAlong2D(targetIn, true, prec)},
				offsetOut{blade.closestPointAlong2D(targetOut, true, prec)};
		bool isIn = (offsetIn.getPos(::target) != Point::undefined),
				isOut = (offsetOut.getPos(::target) != Point::undefined);
		if (!isIn && !isOut)
			return Position::undefined;
		if (isOut && isIn)
			isOut = !(targetIn.lengthFrom2D(offsetIn) > targetOut.lengthFrom2D(offsetOut));
		return (isOut) ? blade.positionOf(targetOut, prec / 10) :
				((blade.positionOf(targetIn, prec / 10) == Point::inside) ? Position::outside : Position::inside);
	} //getTravelDirection
		
	
	/*--------------------------------------------------------------------
		Travel through the vertices of a polygon to build a new polygon
		
		sourcePoly: The source polygon
		refPoly: The reference polygon
		startPt: The starting point
		part: The polygon part we are using
		inc: The polygon iterator increment (direction)
		poly: The polygon to build
		inter: A list of intersections which stop travel
		usage: The travel direction we are using
		
		return: True if the polygon is finished
	  --------------------------------------------------------------------*/
	bool travelPolygon(const Polygon& sourcePoly, const LinEquation& ref, XList::iterator& startPt,
					   vertex_index inc, Polygon& poly, XList& inter, double prec) {
			//Get the source polygon
		auto obj = (*startPt)->getPart(target).value_or(0);
		const auto& source = sourcePoly.getShape(obj);
			//Find the start vertex index
		auto index = source.findVertexByLocation(**startPt);
		if (!index)
			return true;
		(*startPt)->setVertex(target, std::nullopt);
		return extractPolygonPath(source, index->vertex, startPt, inc, poly, inter, prec);
	} //travelPolygon
	
	
	/*--------------------------------------------------------------------
		Travel through the vertices of a polygon to build a new polygon
		
		sourcePoly: The source polygon
		refPoly: The reference polygon
		startPt: The starting point
		part: The polygon part we are using
		inc: The polygon iterator increment (direction)
		poly: The polygon to build
		inter: A list of intersections which stop travel
		usage: The travel direction we are using
		
		return: True if the polygon is finished
	  --------------------------------------------------------------------*/
	bool travelPolygon(const Polygon& sourcePoly, const Polygon& refPoly, XList::iterator& startPt,
					   XPoint::Role role, vertex_index inc, Polygon& poly, XList& inter, double prec) {
			//Get the source polygon and vertex
		auto objIndex = (*startPt)->getPart(role), vertexIndex = (**startPt).getVertex(role);
		if (!objIndex || !vertexIndex)
			return false;
		const auto& source = sourcePoly.getShape(*objIndex);
			//NB: The intersection vertex is usually an index, but in this case has been swapped for the vertex ID
		auto index = source.findVertexById(*vertexIndex);
		if (!index)
			return true;
		if (role == target)
			(*startPt)->setVertex(target, std::nullopt);
		return extractPolygonPath(source, index->vertex, startPt, inc, poly, inter, prec);
	} //travelPolygon
	
	
	/*--------------------------------------------------------------------
		Insert holes which fit in the specified polygon

		target: The polygon to receive the holes
		holes: The holes to be tested
		prec: The required precision
	  --------------------------------------------------------------------*/
	void insertHoles(Polygon& target, PolyVector& holes, double prec = eps) {
		for (auto hole = holes.begin(); hole != holes.end(); ) {
			if (target.encloses(**hole, prec))
				target.emplaceHole(holes.release(hole));
			else
				++hole;
		}
	} //insertHoles
	
	
	/*--------------------------------------------------------------------
		Calculate non-reflecting intersections between two polygons

		target: The target polygon
		blade: The blade polygon
		inter: The intersections between blade and target
		prec: The required precision
		
		return: True if the outer perimeters of the polygons touch anywhere
	  --------------------------------------------------------------------*/
	bool intersectPolyWithPoly(Polygon& target, Polygon& blade, XList& inter, double prec = eps) {
		target.intersectionWith(blade, inter, prec);
		XList::iterator xpoint;
		bool isTouching = false;
		for (xpoint = inter.begin(); !isTouching && (xpoint != inter.end()); ++xpoint)
			isTouching = (((**xpoint).getPart(::target) == 0) && ((**xpoint).getPart(::blade) == 0));
		auto* poly = &target;
		auto which = ::target;
			//Replace the vertex index with the vertex ID
		for (auto order = 0; order < 2; ++order) {
			for (xpoint = inter.begin(); xpoint != inter.end(); ++xpoint) {
				auto obj = (**xpoint).getPart(which).value_or(0);
				auto& dest = poly->getShape(obj);
				(**xpoint).setVertex(which, dest[(**xpoint).getVertex(which).value_or(0)]->id);
			}
			poly = &blade;
			which = ::blade;
		}
		inter.removeDuplicates();
		poly = &target;
		which = ::target;
			//Split the polygons at the intersections
		for (auto order = 0; order < 2; ++order) {
			for (xpoint = inter.begin(); xpoint != inter.end(); ++xpoint) {
				auto* pt = xpoint->get();
				auto obj = pt->getPart(which).value_or(0);
				auto& dest = poly->getShape(obj);
				auto vert = dest.addNodeAlong(pt->getVertex(which).value_or(0), *pt, prec);
				if (vert != 0)
					pt->setVertex(which, vert);
			}
			poly = &blade;
			which = ::blade;
		}
			//Remove reflected intersections
		for (xpoint = inter.begin(); xpoint != inter.end(); ) {
			if (isPolyReflection(target, blade, **xpoint, prec))
				xpoint = inter.erase(xpoint);
			else
				++xpoint;
		}
		return isTouching;
	} //intersectPolyWithPoly
	
		

	/*--------------------------------------------------------------------
		Split a polygon against another polygon
		(The blade polygon is assumed to have no holes)

		target: The target polygon (may have holes)
		blade: The blade polygon (no holes)
		polyIn: The split polygons inside the blade polygon (nullptr = discard)
		polyOut: The split polygons outside the blade polygon (nullptr = discard)
		prec: The required precision
	  --------------------------------------------------------------------*/
	void splitPolyWithPoly(Polygon& target, Polygon& blade, PolyVector* polyIn, PolyVector* polyOut, double prec) {
		target.setDirection(Polygon::clockwise);
		for (auto hole = 0; hole < target.getHoleSize(); ++hole)
			target.getHole(hole)->setDirection(Polygon::anticlockwise);
		target.renumber();
		blade.setDirection(Polygon::clockwise);
		blade.isHole = false;
		blade.setHoles();
		blade.renumber();
		XInfo spec1{Point::within}, spec2{Point::within};
		XList inter{spec1, spec2};
		XList::iterator i;
		bool isInternal = !intersectPolyWithPoly(target, blade, inter, prec);
		if (inter.size() > 0) {
			auto basePoly = clone(target);
			basePoly->setHoles();
			auto exterior = (isInternal) ? clone(*basePoly) : nullptr;
			basePoly->clear();
			PolyVector myHoles;
				//Find holes in this polygon not intersecting the reference polygon
			for (auto hole = target.getHoleSize(); hole--; ) {
				i = std::find_if(inter.begin(), inter.end(), [&](const auto& val) { return val->getPart(::target) == partOption(hole + 1); });
				if (i == inter.end()) {
					if (isInternal)
						exterior->emplaceHole(clone(*target.getHole(hole)));
					else
						myHoles.emplace_back(clone(*target.getHole(hole)));
				}
			}
				//Keep going until we run out of unused intersections
			for (;;) {
					//Find an unused starting point for the new polygon
				XList::iterator nextPt;
				auto usage = Point::undefined;
					//Determine where the new polygon will be in relation to the blade polygon
				for (nextPt = inter.begin(); nextPt != inter.end(); ++nextPt) {
					usage = getTravelDirection(**nextPt, target, blade, prec);
					if (usage != Point::undefined)
						break;
				}
				if (nextPt == inter.end())
					break;
					//Initialise a new polygon
				auto poly = clone(*basePoly);
				poly->emplace_back(**nextPt);
					//Travel through the relevant polygon edges until closure
				for (;;) {
					if (travelPolygon(target, blade, nextPt, ::target, 1, *poly, inter, prec))
						break;
					if (travelPolygon(blade, target, nextPt, ::blade,
							(usage == Point::inside) ? 1 : -1, *poly, inter, prec))
						break;
				}
				if (poly->Polygon::isValid()) {
					bool polyHeld = (isInternal) ? blade.encloses(*poly) : false;
					if (isInternal && (!polyHeld || ((usage != Point::inside) && polyHeld)))
						exterior->emplaceHole(std::move(poly));
					else if (((usage == Point::inside) && polyIn) ||
							 ((usage != Point::inside) && polyOut)) {
						insertHoles(*poly, myHoles);
							//Add the new polygon to the relevant list
						if (usage == Point::inside)
							polyIn->emplace_back(poly);
						else 
							polyOut->emplace_back(poly);
					}
				}
			}
			if (isInternal) {
				if (polyOut != nullptr)
					polyOut->emplace_back(exterior);
			}
		} else {
				//Add this polygon to the relevant list
			if (blade.encloses(target, prec)) {
				if (polyIn != nullptr)
					polyIn->emplace_back(target);
			} else {
				bool bladeHeld = target.encloses(blade);
				if (polyOut != nullptr) {
					auto outer = clone(target);
						//If the blade is entirely contained by the target, it becomes a hole
					if (bladeHeld)
						outer->emplaceHole(clone(blade));
						//If there is an overlap, we need to discover the extent
					else if ((outer->getHoleSize() > 0) && target.overlaps(blade)) {
						auto inner = (polyIn == nullptr) ? nullptr : clone(blade);
						auto holes = outer->releaseHoles();
							//If holes are outside the blade, they return to the outer polygon
							//Otherwise, they become a hole in the inner polygon (when required)
						for (auto hole = holes->begin(); hole != holes->end(); ) {
							if (blade.overlaps(**hole)) {
								if ((**hole).encloses(blade)) {
									inner.reset();
								} else {
									++hole;
									continue;
								}
							}
							outer->emplaceHole(holes->release(hole));
						}
						if (inner && (holes->size() > 0)) {
							outer->emplaceHole(clone(blade));
							inner->setHoles(std::move(holes));
						}
						if (inner)
							polyIn->emplace_back(inner);
					}
					polyOut->emplace_back(outer);
				}
				
				if (bladeHeld && (polyIn != nullptr))
					polyIn->emplace_back(clone(blade));
			}
		}
	} //splitPolyWithPoly
	
	
	/*--------------------------------------------------------------------
		Resolve a polygon into a list of polygons with no self-intersection
		
		poly: The polygon to resolve
		resolved: The list of resolved polygons
		prec: The required precision
		
		return: True if the polygon required resolution
	  --------------------------------------------------------------------*/
	bool resolvePolyIntersect(const Polygon& poly, PolyVector& processed, double prec) {
		auto poly2 = clone(poly);
			//Can't deal with holes in self-intersecting polygons yet
		poly2->releaseHoles();
		bool reduced = poly2->removeDuplicates2D(prec);
			//We need ID's on the nodes so we can track them
		poly2->renumber();
			//Search for self-intersections
		auto info(Point::within);
		for (auto vertex = poly2->vertSize(); vertex--; ) {
			XList inter(info, info);
			if ((*poly2)[vertex]->isArc())
				poly2->intersectionWith(Arc(*(*poly2)[vertex - 1], *(*poly2)[vertex]), inter, prec);
			else
				poly2->intersectionWith(Line(*(*poly2)[vertex - 1], *(*poly2)[vertex]), inter, prec);
				//One intersection is normal (with the preceeding edge)
			if (inter.size() > 1) {
				for (auto xpoint = inter.begin(); xpoint != inter.end(); ) {
						//Filter out the necessary intersection with the preceeding edge
					if (((**xpoint).getPos(target) == Point::end) &&
							(((**xpoint).getVertex(target).value_or(0) == poly2->wrapIndex(vertex - 1)) ||
							(((**xpoint).getPos(target) == Point::origin) &&
							((**xpoint).getVertex(target) == poly2->wrapIndex(vertex + 1)))))
						++xpoint;
					else {
						auto id1 = (*poly2)[vertex]->id,
								id2 = (*poly2)[(**xpoint).getVertex(target).value_or(0)]->id;
							//Add nodes at the intersection point
						id1 = poly2->addNodeAlong(id1, **xpoint, prec);
						id2 = poly2->addNodeAlong(id2, **xpoint, prec);
							//Get the index to each vertex
						auto index1 = poly2->findVertexById(id1), index2 = poly2->findVertexById(id2);
						if (!index1 || !index2)
							break;	//Skip to the next edge on failure
							//Create two polygons either side of the intersection
						auto procPoly = clone(*poly2);
						procPoly->clear();
						auto toStrip = poly2->wrapIndex(index2->vertex - index1->vertex + poly2->vertSize());
						auto vertex1 = poly2->wrapIndex(index1->vertex + 1);
							//Construct the new polygon from the vertices stripped off the existing
						for ( ; toStrip--; ) {
							auto ptIter = poly2->begin() + vertex1;
							procPoly->emplace_back(poly2->release(ptIter));
							if (vertex1 >= static_cast<vertex_index>(poly2->vertSize()))
								vertex1 = 0;
						}
						if (procPoly->isValid())
							processed.emplace_back(procPoly);
						if (poly2->isValid())
							processed.push_back(poly2.release());
						return true;
					}
				}
			}
		}
		if (reduced && poly2->isValid())
			processed.push_back(poly2.release());
		return reduced;
	} //resolvePolyIntersect
	
}  // namespace

//MARK: - Constructors
		
/*--------------------------------------------------------------------
	Constructor

	size: The number of vertices to reserve space for
	holeSize: The number of holes to reserve space for
	isClosed: Set to true if this polygon is closed
  --------------------------------------------------------------------*/
Polygon::Polygon(vertex_index size, part_index holeSize, bool isClosed) : base() {
	if (size != 0)
		reserve(size);
	m_hole = (holeSize == 0) ? nullptr : std::make_unique<PolyVector>(holeSize);
	isHole = false;
	isClosed = isClosed;
} //Polygon::Polygon


/*--------------------------------------------------------------------
	Constructor

	source: The box to base the construction of this polygon on
	z: The z coordinate of the polygon
	rotate: The rotation of the polygon
  --------------------------------------------------------------------*/
Polygon::Polygon(const Box& source, double angle) : base() {
	reserve(4);
	double z = minVal(source.origin.z, source.end.z);
	emplace_back(PolyPoint{source.origin.x, source.origin.y, z});
	emplace_back(PolyPoint{source.end.x, source.origin.y, z});
	emplace_back(PolyPoint{source.end.x, source.end.y, z});
	emplace_back(PolyPoint{source.origin.x, source.end.y, z});
	isHole = false;
	isClosed = true;
	m_hole = nullptr;
	if (!isZero(angle))
		ZRotater{angle}.transform(*this);
} //Polygon::Polygon


/*--------------------------------------------------------------------
	Copy constructor

	source: The polygon to be copied
  --------------------------------------------------------------------*/
Polygon::Polygon(const Polygon& source) : base(source) {
	if (source.m_hole)
		m_hole = std::make_unique<PolyVector>(*source.m_hole);
	m_topID = source.m_topID;
	isHole = source.isHole;
	isClosed = source.isClosed;
} //Polygon::Polygon


/*--------------------------------------------------------------------
	Move constructor
 
	source: The object to move
  --------------------------------------------------------------------*/
Polygon::Polygon(Polygon&& source) noexcept : base(std::move(source)) {
	m_hole = (std::exchange(source.m_hole, nullptr));
	m_topID = source.m_topID;
	isHole = source.isHole;
	isClosed = source.isClosed;
}


/*--------------------------------------------------------------------
	Clone method, copying just the raw geometry (all vertices will be reduced to PolyPoints)
 
	return: A duplicate of this object's geometry
  --------------------------------------------------------------------*/
Polygon::Unique Polygon::cloneGeometry() const {
	auto result = std::make_unique<Polygon>();
	for (const auto& vertex : *this)
		result->push_back(new PolyPoint{*vertex});
	result->m_topID = m_topID;
	result->isHole = isHole;
	result->isClosed = isClosed;
	return result;
} //Polygon::cloneGeometry

//MARK: - Operators
		
/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator= (const Polygon& source) {
	if (this != &source) {
		clear();
		reserve(source.vertSize());
		for (auto vertex = 0; vertex < source.vertSize(); ++vertex)
			emplace_back(clone(*source[vertex]));
		m_topID = source.m_topID;
		isHole = source.isHole;
		isClosed = source.isClosed;
		if (source.m_hole)
			m_hole = std::make_unique<PolyVector>(*source.m_hole);
	}
	return *this;
} //Polygon::operator=
		
/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to move
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator= (Polygon&& source) noexcept {
	if (this != &source) {
		base::operator=(source);
		clear();
		reserve(source.vertSize());
		m_topID = source.m_topID;
		isHole = source.isHole;
		isClosed = source.isClosed;
		m_hole = std::move(source.m_hole);
	}
	return *this;
} //Polygon::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to offset the polygon by
	
	return: An offset copy of this
  --------------------------------------------------------------------*/
Polygon Polygon::operator+ (const Point& offset) const {
	return Polygon{*this} += offset;
} //Polygon::operator+


/*--------------------------------------------------------------------
	Addition with assignment operator
	
	offset: The amount to offset the polygon by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator+= (const Point& offset) {
	for (auto hole = getHoleSize() + 1; hole--; ) {
		auto& poly = getShape(hole);
		for (auto vertex = poly.vertSize(); vertex--; )
			*(poly[vertex]) += offset;
	}
	return *this;
} //Polygon::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	offset: The amount to offset the polygon by
	
	return: An offset copy of this
  --------------------------------------------------------------------*/
Polygon Polygon::operator- (const Point& offset) const {
	return *clone(*this) -= offset;
} //Polygon::operator-


/*--------------------------------------------------------------------
	Subtraction with assignment operator
	
	offset: The amount to offset the polygon by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator-= (const Point& offset) {
	for (auto hole = getHoleSize() + 1; hole--; ) {
		auto& poly = getShape(hole);
		for (auto vertex = poly.vertSize(); vertex--; )
			*(poly[vertex]) -= offset;
	}
	return *this;
} //Polygon::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	mult: The amount to scale the polygon by
	
	return: A scaled copy of this
  --------------------------------------------------------------------*/
Polygon Polygon::operator* (const double& mult) const {
	return *clone(*this) *= mult;
} //Polygon::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	mult: The amount to scale the polygon by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator*= (const double& mult) {
	for (auto hole = getHoleSize() + 1; hole--; ) {
		auto& poly = getShape(hole);
		for (auto vertex = poly.vertSize(); vertex--; )
			*(poly[vertex]) *= mult;
	}
	return *this;
} //Polygon::operator*=


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply the polygon by
	
	return: The result
  --------------------------------------------------------------------*/
Polygon Polygon::operator* (const Matrix3x3& mult) const {
	return *clone(*this) *= mult;
} //Polygon::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	mult: The matrix to multiply the polygon by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator*= (const Matrix3x3& mult) {
	for (auto vertex = vertSize(); vertex--; )
		*(*this)[vertex] *= mult;
	Point ref1(0, 1), ref2(1, 0);
	ref1 *= mult;
	ref2 *= mult;
	if (auto lin = LinEquation::create(Point(), ref1); lin && (lin->positionOf(ref2) != Point::right)) {
		for (auto vertex = vertSize(); vertex--; )
			(*this)[vertex]->sweep = ((*this)[vertex]->sweep * -1);
	}
	if (m_hole != nullptr) {
		for (auto hole = getHoleSize(); (hole-- != 0); )
			*(*m_hole)[hole] *= mult;
	}
	return *this;
} //Polygon::operator*=


/*--------------------------------------------------------------------
	Multiplication operator

	mult: The matrix to multiply the polygon by

	return: The result
  --------------------------------------------------------------------*/
Polygon Polygon::operator* (const Matrix4x4& mult) const {
	return *clone(*this) *= mult;
} //Polygon::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator

	mult: The matrix to multiply the polygon by

	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator*= (const Matrix4x4& mult) {
	for (auto vertex = vertSize(); vertex--; )
		*(*this)[vertex] *= mult;
	Point ref1(0.0, 1.0), ref2(1.0, 0.0);
	ref1 *= mult;
	ref2 *= mult;
	if (auto lin = LinEquation::create(Point(), ref1); lin && (lin->positionOf(ref2) != Point::right)) {
		for (auto vertex = vertSize(); vertex--; )
			(*this)[vertex]->sweep = (*this)[vertex]->sweep * -1;
	}
	if (m_hole != nullptr) {
		for (auto hole = getHoleSize(); (hole-- != 0); )
			*(*m_hole)[hole] *= mult;
	}
	return *this;
} //Polygon::operator*=


/*--------------------------------------------------------------------
	Division operator
	
	mult: The amount to scale the polygon by
	
	return: A scaled copy of this
  --------------------------------------------------------------------*/
Polygon Polygon::operator/ (const double& mult) const {
	return *clone(*this) /= mult;
} //Polygon::operator/


/*--------------------------------------------------------------------
	Division with assignment operator
	
	mult: The amount to scale the polygon by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Polygon& Polygon::operator/= (const double& mult) {
	for (auto hole = getHoleSize() + 1; hole--; ) {
		auto& poly = getShape(hole);
		for (auto vertex = poly.vertSize(); vertex--; )
			*(poly[vertex]) /= mult;
	}
	return *this;
} //Polygon::operator/=


/*--------------------------------------------------------------------
	Subscript operator
	
	index: An index into the polygon
	
	return: The indexed vertex
  --------------------------------------------------------------------*/
PolyPoint* Polygon::operator[] (vertex_index index) {
	return base::at(wrapIndex(index)).get();
} //Polygon::operator[]


/*--------------------------------------------------------------------
	Subscript operator
	
	index: An index into the polygon
	
	return: The indexed vertex
  --------------------------------------------------------------------*/
const PolyPoint* Polygon::operator[] (vertex_index index) const {
	return base::at(wrapIndex(index)).get();
} //Polygon::operator[]

//MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if the polygon is valid (must have 2 or more vertices)

	doIntersect: True if the polygon should be checked for intersecting edges (crossing each other)
	prec: The required precision
 
	return: True if the polygon is valid
  --------------------------------------------------------------------*/
bool Polygon::isValid(bool doIntersect, double prec) const {
	if (!isClosed)
		return (vertSize() > 1);	//Any polyline with more than one point is valid
	if (!((vertSize() > 2) || ((vertSize() == 2) && ((*this)[0]->isArc() || (*this)[1]->isArc()))))
		return false;	//A polygon should either have at least 3 vertices, or 2 vertices with at least 1 curved edge
	if (doIntersect) {
		XInfo spec1{Point::latter}, spec2{Point::within};
		for (auto vertex = vertSize(); vertex--; ) {
			XList inter{spec1, spec2};
			if ((*this)[vertex]->isArc())
				intersectionWith(Arc(*(*this)[vertex - 1], *(*this)[vertex]), inter, prec);
			else
				intersectionWith(Line(*(*this)[vertex - 1], *(*this)[vertex]), inter, prec);
			if (inter.size() > 1)
				return false;	//Any edge should only touch the next edge
		}
	}
	return true;
} //Polygon::isValid


/*--------------------------------------------------------------------
	Determine if two polygons are equal in 2D
	
	ref: The polygon to compare
	prec: Precision
	
	return: True if the polygons are identical
  --------------------------------------------------------------------*/
bool Polygon::isEqual2D(const Polygon& ref, double prec) const {
	if (getHoleSize() != ref.getHoleSize())
		return false;
	for (auto hole = getHoleSize() + 1; hole--; ) {
		const auto& poly = getShape(hole);
		const auto& refPoly = ref.getShape(hole);
		if (poly.vertSize() != refPoly.vertSize())
			return false;
		for (auto vertex = poly.vertSize(); vertex--; )
			if (!(poly[vertex])->isEqual2D(*(refPoly[vertex]), prec))
				return false;
	}
	return true;
} //Polygon::isEqual2D


/*--------------------------------------------------------------------
	Determine if two polygons are equal in 3D
	
	ref: The polygon to compare
	prec: Precision
	
	return: True if the polygons are identical
  --------------------------------------------------------------------*/
bool Polygon::isEqual3D(const Polygon& ref, double prec) const {
	if (getHoleSize() != ref.getHoleSize())
		return false;
	for (auto hole = getHoleSize() + 1; hole--; ) {
		const auto& poly = getShape(hole);
		const auto& refPoly = ref.getShape(hole);
		if (poly.vertSize() != refPoly.vertSize())
			return false;
		for (auto vertex = poly.vertSize(); vertex--; )
			if (!(poly[vertex])->isEqual3D(*(refPoly[vertex]), prec))
				return false;
	}
	return true;
} //Polygon::isEqual3D


/*--------------------------------------------------------------------
	Get the number of vertices in the polygon
 
	isOuter: True to count the outer polygon vertices only (exclude holes)
 
	return: The number of vertices
  --------------------------------------------------------------------*/
vertex_index Polygon::vertSize(bool isOuter) const {
	auto result = static_cast<vertex_index>(size());
	if (!isHole && !isOuter)
		for (auto hole = getHoleSize(); hole--; result += (*m_hole)[hole]->vertSize()) {}
	return result;
} //Polygon::vertSize


/*--------------------------------------------------------------------
	Get the number of holes in the polygon
	
	isOuter: True if only the outer polygon is counted (not holes)

	return: The number of holes in the polygon
  --------------------------------------------------------------------*/
part_index Polygon::getHoleSize() const {
	return !m_hole ? 0 : static_cast<part_index>(m_hole->size());
} //Polygon::getHoleSize


/*--------------------------------------------------------------------
	Get the requested hole
	
	which: The index of the polygon to get
	
	return: The requested polygon
  --------------------------------------------------------------------*/
Polygon* Polygon::getHole(part_index which) const {
	if (!m_hole || (which < 0) || (static_cast<PolyVector::size_type>(which) >= m_hole->size()))
		throw std::out_of_range("");
	return (*m_hole)[which].get();
} //Polygon::getHole


/*--------------------------------------------------------------------
	Find a vertex with the specified ID (optionally searching holes)
	
	ID The ID to search for
 
	return: The index of a vertex with the specified ID (npos on failure)
  --------------------------------------------------------------------*/
std::optional<Polygon::Index> Polygon::findVertexById(vertex_id ID) const {
	for (auto vertex = vertSize(); vertex--; )
		if ((*this)[vertex]->id == ID)
			return Index{0, vertex, (*this)[vertex]};
	for (auto hole = getHoleSize(); hole--; ) {
		if (auto index = (*m_hole)[hole]->findVertexById(ID); index)
			return Index{hole + 1, index->vertex, index->point};
	}
	return std::nullopt;
} //Polygon::findVertexById


/*--------------------------------------------------------------------
	Find a vertex at the specified point (optionally searching holes)
	
	pt: The point to match
	prec: The required precision
	
	return: The index of a vertex at the specified point (npos = failure)
  --------------------------------------------------------------------*/
std::optional<Polygon::Index>  Polygon::findVertexByLocation(const Point& pt, double prec) const {
	for (auto vertex = 0; vertex < vertSize(); ++vertex)
		if (pt.isEqual2D(*(*this)[vertex], prec))
			return Index{0, vertex, (*this)[vertex]};
	for (auto hole = getHoleSize(); hole--; ) {
		if (auto index = (*m_hole)[hole]->findVertexByLocation(pt, prec); index)
			return Index{hole + 1, index->vertex, index->point};
	}
	return std::nullopt;
} //Polygon::findVertexByLocation


/*--------------------------------------------------------------------
	Get one of the subpolygons of which this polygon consists

	index:  The index of the shape

	return If *index* == nullptr, the polygon itself, otherwise the hole with number *index* - 1
  --------------------------------------------------------------------*/
const Polygon& Polygon::getShape(part_index index) const {
	return (index == 0) ? *this : *getHole(index - 1);
} //Polygon::getShape


/*--------------------------------------------------------------------
	Get one of the subpolygons of which this polygon consists

	index:  The index of the shape

	return If *index* == nullptr, the polygon itself, otherwise the hole with number *index* - 1
  --------------------------------------------------------------------*/
Polygon& Polygon::getShape(part_index index) {
	return (index == 0) ? *this : *getHole(index - 1);
} //Polygon::getShape


/*--------------------------------------------------------------------
	Get an iterator pointing to the specified vertex
	
	index: The vertex index (will be wrapped)
	
	return: The requested iterator
  --------------------------------------------------------------------*/
Polygon::base::iterator Polygon::iteratorAt(vertex_index index) {
	if (vertSize() == 0)
		return end();
	return (begin() + wrapIndex(index));
} //Polygon::iteratorAt


/*--------------------------------------------------------------------
	Get an iterator pointing to the specified vertex
	
	index: The vertex index (will be wrapped)
	
	return: The requested iterator
  --------------------------------------------------------------------*/
Polygon::base::const_iterator Polygon::iteratorAt(vertex_index index) const {
	if (vertSize() == 0)
		return end();
	return (begin() + wrapIndex(index));
} //Polygon::iteratorAt


/*--------------------------------------------------------------------
	Get the polygon bounds
	
	return: The polygon bounds
  --------------------------------------------------------------------*/
Box::Option Polygon::bounds() const {
	if (vertSize() < 1)
		return std::nullopt;
	Box result{*(*this)[0], *(*this)[0]};
	for (auto vertex = vertSize(); vertex--; ) {
		if ((*this)[vertex]->isArc()) {
			Arc arc{*(*this)[vertex - 1], *(*this)[vertex]};
			auto bounds = arc.bounds();
			if (bounds.get() != nullptr)
				result.merge(*bounds);
		} else
			result.merge(*(*this)[vertex]);
	}
	result.sort();
	return Box::Option(result);
} //Polygon::bounds


/*--------------------------------------------------------------------
	Get the direction of the polygon
	
	return: The polygon direction
  --------------------------------------------------------------------*/
std::optional<Rotation> Polygon::getDirection() const {
	if (!Polygon::isValid())
		return std::nullopt;
	return getArea(false, true) > 0 ? Polygon::anticlockwise : Polygon::clockwise;
} //Polygon::getDirection


/*--------------------------------------------------------------------
	Calculate the polygon perimeter (non-closure handled)
	
	return: The total perimeter (plan only)
  --------------------------------------------------------------------*/
double Polygon::getPerimeter2D() const {
	double len = 0.0;
	for (auto vertex = edgeSize(); vertex--; )
		len += (*this)[vertex + 1]->lengthFrom2D(*(*this)[vertex]);
	return len;
} //Polygon::getPerimeter2D


/*--------------------------------------------------------------------
	Calculate the polygon perimeter (non-closure handled)
	
	return: The total perimeter (3D polyline)
  --------------------------------------------------------------------*/
double Polygon::getPerimeter3D() const {
	double len = 0.0;
	for (auto vertex = edgeSize(); vertex--; )
		len += (*this)[vertex + 1]->lengthFrom3D(*(*this)[vertex]);
	return len;
} //Polygon::getPerimeter3D


/*--------------------------------------------------------------------
	Trace the polygon perimeter for the specified length
	
	len: The length to trace for
	endPos: The point reached
	
	return: The index of the last vertex passed
  --------------------------------------------------------------------*/
vertex_index Polygon::tracePerimeter(double len, PolyPoint& endPos) const {
	if (empty()) {
		endPos = Point();
		return 0;
	}
	if (isLessOrEqualZero(len)) {
		endPos = *front();
		return 0;
	}
	if (isGreaterOrEqual(len, getPerimeter3D())) {
		endPos = *back();
		return vertSize() - 1;
	}
	vertex_index result;
	for (result = 0; (isGreaterZero(len)) && (result < vertSize()); ++result) {
		PolyEdge edge{*(*this)[result], *(*this)[result + 1]};
		len -= edge.length3D();
		if (isLessOrEqualZero(len)) {
			edge.extend(len);
			endPos = edge.end;
			break;
		}
	}
	return result;
} //Polygon::tracePerimeter


/*--------------------------------------------------------------------
	Calculate the polygon area (closure assumed)
	
	isNet: True if the net area is calculated (less holes)
	isResultSigned: True if the returned area should be negative for polygons with clockwise sense
	
	return: The total area
  --------------------------------------------------------------------*/
double Polygon::getArea(bool isNet, bool isResultSigned) const {
	if (!Polygon::isValid())
		return 0.0;
	double result = 0.0;
	for (auto vertex = vertSize(); vertex > 0; vertex--) {
		result += (*this)[vertex + 1]->x * ((*this)[vertex + 2]->y - (*this)[vertex]->y) / 2.0;
		if ((*this)[vertex]->isArc())
			result += PolyEdge{*(*this)[vertex - 1], *(*this)[vertex]}.getArea(true);
	}
	double areaSign = sgn(result);
	result = fabs(result);
		//Subtract the area of the holes
	if (isNet)
		for (auto hole = getHoleSize(); hole--; )
			result -= getHole(hole)->getArea(false, false);
	if (isLessOrEqualZero(result))
		result = 0.0;
	else if (isResultSigned)
		result *= areaSign;
	return result;
} //Polygon::getArea


/*--------------------------------------------------------------------
	Correct indexes beyond the poly-line's range
	
	index: The index to check
	
	return: The corrected result
  --------------------------------------------------------------------*/
vertex_index Polygon::wrapIndex(vertex_index index) const {
	vertex_index top = vertSize();
	index %= top;
	if (index < 0)
		index += top;
	return index;
} //Polygon::wrapIndex


/*-------------------------------------------------------------------
	Get the internal angle between the edges at the given index
 
	index: The vertex index
	
	return: The angle between the two edges
 -------------------------------------------------------------------*/
double Polygon::getInternalAngleAt(vertex_index index) const {
	PolyEdge edge1{*(*this)[index - 1], *(*this)[index]};
	PolyEdge edge2{*(*this)[index], *(*this)[index + 1]};
	double result = angleMod(pi - edge2.startTangent() + edge1.endTangent());
	if (auto direct = getDirection(); direct && (direct == Polygon::clockwise))
		result = 2.0 * pi - result;
	return result;
} //Polygon::getInternalAngleAt


/*--------------------------------------------------------------------
	Determine if the edges at a specified vertex are tangential
	
	vertex: The vertex index at which the edges meet
	anglePrec: The angle precision
	
	return: True if the edges are tangential at the vertex
  --------------------------------------------------------------------*/
bool Polygon::isTangentialAt(vertex_index vertex, double anglePrec) const {
	if (!Polygon::isValid())
		return false;
	return PolyEdge{*(*this)[vertex], *(*this)[vertex + 1]}.isTangentialTo2D(PolyEdge(*(*this)[vertex - 1], *(*this)[vertex]), eps, anglePrec);
}//Polygon::isTangentialAt


/*--------------------------------------------------------------------
	Determine if the polygon reflects off a specified line at a specified vertex
	
	index: The index of the vertex to test
	ref: The reference line
	prec: The required precision
	
	return: True if a reflection ocurrs at the specified index
  --------------------------------------------------------------------*/
bool Polygon::isReflection(vertex_index index, const LinEquation& ref, double prec) const {
	wrapIndex(index);
	auto orig = *(*this)[index];
	if (ref.positionOf(orig, prec) != Point::along)
		return false;
	vertex_index next, prev;
	for (prev = index + vertSize() - 1; prev > index; --prev) {
		if (!isZero((*this)[prev + 1]->sweep, prec) || (ref.positionOf(*(*this)[prev], prec) != Point::along))
			break;
	}
	if (prev == index)
		return true;	//The whole polygon is linear
	for (next = index - vertSize() + 1 ; next != index; ++next) {
		if (!isZero((*this)[next]->sweep, prec) || (ref.positionOf(*(*this)[next], prec) != Point::along))
			break;
	}
	if (wrapIndex(next) == prev)
		return true;	//The polygon forms a triangle
	auto offsets = getVertexOffsets(*this, next, prev, &ref, prec);
	return (ref.positionOf(offsets.second, prec) == ref.positionOf(offsets.first, prec));
} //Polygon::isReflection


/*--------------------------------------------------------------------
	Get a point inside the polygon

	return: A point inside the polygon (nullopt on failure)
  --------------------------------------------------------------------*/
Point::Option Polygon::getInternalPoint() const {
		//First get the polygon rectilinear bounds
	auto myBounds = bounds();
	if (!myBounds)
		return std::nullopt;
		//Then calculate the intersections between the polygon a horizontal line drawn through the centre of the polygon bounds
	XList intersects{XInfo{Point::former}, XInfo{Point::within}};
	PolyEdge centreLine{myBounds->getAnchor2D(Box::leftHalf), myBounds->getAnchor2D(Box::rightHalf)};
	if (intersectionWith(centreLine, intersects) < 2)
		return std::nullopt;
		//Then determine if the midpoint between any 2 adjacent intersections is within the polygon
	intersects.sort(AlongLengthOf{centreLine});
	auto iter = intersects.begin();
	auto previous = **(iter++);
	for (; iter != intersects.end(); ++iter) {
		auto midpoint = (previous + **iter) / 2.0;
			//If the midpoint is inside the polygon, we have an internal point
		if (positionOf(midpoint) == Point::inside)
			return midpoint;
	}
	return std::nullopt;
} //Polygon::getInternalPoint


/*--------------------------------------------------------------------
	Get the position of a point relative to the polygon (closure assumed)
	
	ref: The reference point
	prec: The required precision
	
	return: The relative position
  --------------------------------------------------------------------*/
Position Polygon::positionOf(const Point& ref, double prec) const {
		//First do a simple test on the rectilinear bounds
	auto myBounds = bounds();
	if ((myBounds) && (myBounds->positionOf2D(ref, prec) == Point::outside))
		return Point::outside;
		//Next test if the point is along an edge
	std::list<double> edgeAngles;
	for (auto hole = getHoleSize() + 1; hole--; ) {
		const auto& poly = getShape(hole);
		for (auto vertex = poly.vertSize(); vertex--; ) {
			PolyEdge edge{*poly[vertex - 1], *poly[vertex]};
			if ((edge.positionOf2D(ref, prec) & Point::within) != Point::undefined)
				return Point::along;
			if (!edge.isArc())
				edgeAngles.push_back(fmod(edge.azimuthAngle(), pi));
		}
	}
		//Finally test the number intersections travelling from the reference point
		//An even number of intersection means the point is outside
		//The line direction should be as far as possible from parallel from a polygon edge
	double refAngle = 0.0;
	if (edgeAngles.size() > 0) {
		edgeAngles.push_back(pi);
		edgeAngles.sort();
		double prevAngle = 0.0, maxAngle = 0.0;
		for (auto angle = edgeAngles.begin(); angle != edgeAngles.end(); ++angle) {
			if (isGreater(*angle - prevAngle, maxAngle)) {
				maxAngle = *angle - prevAngle;
				refAngle = prevAngle + (maxAngle / 2);
			}
			prevAngle = *angle;
		}
	}
	Point refEnd{ref};
	refEnd.movePolar(2 * myBounds->origin.lengthFrom2D(myBounds->end), refAngle);
	Line refLine{ref, refEnd};
	XInfo spec1{Point::within}, spec2{Point::within};
	XList inter{spec1, spec2};
	intersectionWith(refLine, inter, prec);
	XList::iterator xpoint;
		//Remove intersections on an edge end which continue along the line
	inter.removeDuplicates();
	if (inter.size() > 0) {
		vertex_index totalInt = 0;
		auto refEq = LinEquation::create(refLine);
		for (xpoint = inter.begin(); xpoint != inter.end(); ++xpoint) {
			if ((**xpoint).getPos(blade) == Point::origin)
				return Point::along;
			if (((**xpoint).getPos(target) & Point::vertex) != Point::undefined) {
				auto obj = (**xpoint).getPart(target).value_or(0);
				const auto& targ = getShape(obj);
				auto inc = ((**xpoint).getPos(target) == Point::origin) ? -1 : 0;
				if ((refEq) && targ.isReflection((**xpoint).getVertex(target).value_or(0) + inc, *refEq))
					continue;
			}
			++totalInt;
		}
		return ((totalInt % 2) == 0) ? Point::outside : Point::inside;
	}
	return Point::outside;
} //Polygon::positionOf


/*--------------------------------------------------------------------
	Get the closest point in the polygon to a given point in 2D

	ref: The reference point
	withHoles: True to compare holes
	prec: The required precision

	return: The closest point in the polygon to the reference point, coupled with the intercept position info
  --------------------------------------------------------------------*/
XPoint Polygon::closestPointAlong2D(const Point& ref, bool withHoles, double prec) const {
	XPoint result;
	if (vertSize() > 1) {
		result = *(*this)[0];
		result.setVertex(target, 1);
		result.setPos(target, Point::origin);
		double min = ref.lengthFrom2D(result);
		for (auto hole = ((withHoles) ? getHoleSize() + 1 : 1); hole--; ) {
			const auto& poly = getShape(hole);
			for (auto vertex = poly.edgeSize(); vertex--; ) {
				PolyEdge refEdge{*poly[vertex], *poly[vertex + 1]};
				auto test = refEdge.closestPointAlong2D(ref, prec);
				double len = ref.lengthFrom2D(test);
				if (len < min) {
					min = len;
					if (ref.isEqual2D(refEdge.origin, prec))
						result.setPos(target, Point::origin);
					else if (ref.isEqual2D(refEdge.end, prec))
						result.setPos(target, Point::end);
					else
						result.setPos(target, Point::along);
					result.setVertex(target, vertex);
					result.setPart(target, hole);
					result = test;
				}
			}
		}
	}
	return result;
} //Polygon::closestPointAlong2D


/*--------------------------------------------------------------------
	Get the closest point in the polygon to a given point in 3D

	ref: The reference point
	withHoles: True to compare holes
	prec: The required precision

	return: The closest point in the polygon to the reference point, coupled with the intercept position info
  --------------------------------------------------------------------*/
XPoint Polygon::closestPointAlong3D(const Point& ref, bool withHoles, double prec) const {
	XPoint result;
	if (vertSize() > 1) {
		result = *(*this)[0];
		result.setVertex(target, 1);
		result.setPos(target, isClosed ? Point::along : Point::origin);
		double min = ref.lengthFrom3D(result);
		for (auto hole = ((withHoles) ? getHoleSize() + 1 : 1); hole--; ) {
			const auto& poly = getShape(hole);
			for (auto vertex = poly.edgeSize(); --vertex; ) {
				auto test = PolyEdge{*poly[vertex], *poly[vertex + 1]}.closestPointAlong3D(ref, prec);
				double len = ref.lengthFrom3D(test);
				if (len < min) {
					min = len;
					if (!poly.isClosed)
						result.setPos(target, Point::along);
					else {
						if (vertex == 0)
							result.setPos(target, Point::origin);
						else if (vertex == (poly.vertSize() - 1))
							result.setPos(target, Point::end);
						else
							result.setPos(target, Point::along);
					}
					result.setVertex(target, vertex);
					result.setPart(target, hole);
					result = test;
				}
			}
		}
	}
	return result;
} //Polygon::closestPointAlong3D


/*--------------------------------------------------------------------
	Get the intersection(s) between this polygon and a line
	
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Polygon::intersectionWith(const Line& ref, XList& inter, double prec) const {
	if (vertSize() < 2)
		return 0;
	vertex_index totalInt = 0, baseObject = inter.getFilter(target).partIndex.value_or(0);
	for (auto vertex = edgeSize(); vertex--; ) {
		inter.setVertex(target, wrapIndex(vertex + 1));
		if ((*this)[vertex + 1]->isArc())
			totalInt += Arc{*(*this)[vertex], *(*this)[vertex + 1]}.intersectionWith2D(ref, inter, prec);
		else
			totalInt += Line{*(*this)[vertex], *(*this)[vertex + 1]}.intersectionWith2D(ref, inter, prec);
	}
	for (auto hole = getHoleSize(); hole--; ) {
		inter.setPart(target, baseObject + hole + 1);
		totalInt += (*m_hole)[hole]->intersectionWith(ref, inter, prec);
	}
	inter.setPart(target, baseObject);
	return totalInt;
} //Polygon::intersectionWith


/*--------------------------------------------------------------------
	Get the intersection(s) between this polygon and an arc
	
	ref: The reference arc
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Polygon::intersectionWith(const Arc& ref, XList& inter, double prec) const {
	if (vertSize() < 2)
		return 0;
	vertex_index totalInt = 0;
	part_index baseObject = inter.getFilter(target).partIndex.value_or(0);
	for (vertex_index vertex = edgeSize(); vertex--; ) {
		inter.setVertex(target, wrapIndex(vertex + 1));
		if ((*this)[vertex + 1]->isArc())
			totalInt += Arc{*(*this)[vertex], *(*this)[vertex + 1]}.intersectionWith2D(ref, inter, prec);
		else {
			inter.swapFilters();
			totalInt += ref.intersectionWith2D(Line{*(*this)[vertex], *(*this)[vertex + 1]}, inter, prec);
			inter.swapFilters();
		}
	}
	for (auto hole = getHoleSize(); hole--; ) {
		inter.setPart(target, baseObject + hole + 1);
		totalInt += (*m_hole)[hole]->intersectionWith(ref, inter, prec);
	}
	inter.setPart(target, baseObject);
	return totalInt;
} //Polygon::intersectionWith


/*--------------------------------------------------------------------
	Get the intersection(s) between this polygon and a poly-edge
 
	ref: The reference poly-edge
	inter: The intersection list to populate
	prec: The required precision
 
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Polygon::intersectionWith(const PolyEdge& ref, XList& inter, double prec) const {
	if (ref.isArc(prec))
		return intersectionWith(Arc(ref.origin, ref.end), inter, prec);
	return intersectionWith(Line(ref.origin, ref.end), inter, prec);
} //Polygon::intersectionWith


/*--------------------------------------------------------------------
	Get the intersection(s) between this polygon and another polygon
	
	ref: The reference polygon
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Polygon::intersectionWith(const Polygon& ref, XList& inter, double prec) const {
	if ((vertSize() < 2) || (ref.vertSize() < 2))
		return 0;
	vertex_index totalInt = 0, baseObject = inter.getFilter(target).partIndex.value_or(0);
	inter.swapFilters();
	for (auto vertex = edgeSize(); vertex--; ) {
		inter.setVertex(blade, wrapIndex(vertex + 1));
		if ((*this)[vertex + 1]->isArc())
			totalInt += ref.intersectionWith(Arc(*(*this)[vertex], *(*this)[vertex + 1]), inter, prec);
		else
			totalInt += ref.intersectionWith(Line(*(*this)[vertex], *(*this)[vertex + 1]), inter, prec);
	}
	inter.swapFilters();
	for (auto hole = getHoleSize(); hole--; ) {
		inter.setPart(target, baseObject + hole + 1);
		totalInt += (*m_hole)[hole]->intersectionWith(ref, inter, prec);
	}
	inter.setPart(target, baseObject);
	return totalInt;
} //Polygon::intersectionWith


/*--------------------------------------------------------------------
	Calculate the polygons created by splitting this polygon along a line
	
	ref: The cutting line
	polyRight: The split polygons to the right of the reference line (nullptr = discard)
	polyLeft: The split polygons to the left of the reference line (nullptr = discard)
	prec: The required precision
  --------------------------------------------------------------------*/
void Polygon::splitWith(const LinEquation& ref, PolyVector* polyRight, PolyVector* polyLeft, double prec) const {
	if ((polyLeft == nullptr) && (polyRight == nullptr))
		return;
	Polygon target(*this);
	double angle = ref.azimuthAngle();
	Point orig{ref.closestPointTo(Point())};
	Line blade{orig, orig + Point{cos(angle), sin(angle)}};
		//The outer polygons must be clockwise, holes anticlockwise
	target.setDirection(Polygon::clockwise);
	for (auto hole = 0; hole < target.getHoleSize(); ++hole)
		target.getHole(hole)->setDirection(Polygon::anticlockwise);
	target.renumber();
	XInfo spec1{Point::within}, spec2{Point::undefined};
	XList inter{spec1, spec2};
	target.intersectionWith(blade, inter, prec);
	XList::iterator xpoint;
		//Replace the vertex index with the vertex ID
	for (xpoint = inter.begin(); xpoint != inter.end(); ++xpoint) {
		auto obj = (*xpoint)->getPart(::target).value_or(0);
		const auto& dest = target.getShape(obj);
		(*xpoint)->setVertex(::target, dest[(*xpoint)->getVertex(::target).value_or(0)]->id);
	}
	inter.removeDuplicates();
		//Split the polygon at the intersections
	for (xpoint = inter.begin(); xpoint != inter.end(); ++xpoint) {
		auto obj = (*xpoint)->getPart(::target).value_or(0);
		auto& target1 = target.getShape(obj);
		auto vert = target1.addNodeAlong((*xpoint)->getVertex(::target).value_or(0), **xpoint, prec);
		if (vert != 0)
			(*xpoint)->setVertex(::target, vert);
	}
		//Remove reflected intersections
	for (xpoint = inter.begin(); xpoint != inter.end(); ) {
		if (isPolyReflection(target, ref, **xpoint, prec))
			xpoint = inter.erase(xpoint);
		else
			++xpoint;
	}
	bool result = (inter.size() > 0);
	if (result) {
		PolyVector myHoles;
			//Find holes in this polygon not intersecting the reference line
		for (part_index hole = getHoleSize(); hole--; ) {
			xpoint = std::find_if(inter.begin(), inter.end(), [&](const auto& val) { return val->getPart(::target) == partOption(hole + 1); });
			if (xpoint == inter.end())
				myHoles.emplace_back(clone(*getHole(hole)));
		}
		inter.sort(ComparePosition(prec));
		auto basePoly = clone(*this);
		basePoly->clear();
		while (basePoly->getHoleSize() > 0)
			basePoly->removeHole(0);
		double azim = ref.azimuthAngle();
		auto direct = (isGreaterZero(cos(azim)) || (isZero(cos(azim)) && isGreaterZero(sin(azim)))) ?
				Point::left : Point::right;
			//Keep going until we run out of unused intersections
		for (;;) {
				//Find an unused starting point for the new polygon
			XList::iterator nextPt;
			auto usage = Point::undefined;
			for (nextPt = inter.begin(); (nextPt != inter.end()); ++nextPt) {
				auto seg = (**nextPt).getVertex(::target);
				if (!seg)
					continue;
				auto obj = (**nextPt).getPart(::target);
				if (!obj)
					continue;
				const auto& src = target.getShape(*obj);
				auto index = src.findVertexById(*seg);
				if (index) {
						//Determine where the new polygon will be in relation to the blade
					usage = ref.positionOf(getEdgeMidpoint(src, index->vertex, 1), prec / 10);
						//If one edge is along the blade, the other is not
					if (usage == Point::along)
						usage = (ref.positionOf(getEdgeMidpoint(src, index->vertex, -1), prec / 10) == Point::left) ?
								Point::right : Point::left;
					break;
				}
			}
			if (nextPt == inter.end())
				break;
				//Create a new polygon
			auto poly = clone(*basePoly);
			poly->push_back(new PolyPoint(**nextPt));
				//Travel through the relevant polygon edges until closure
			for (;;) {
				(*nextPt)->setVertex(::target, std::nullopt);
				if (travelPolygon(target, ref, nextPt, 1, *poly, inter, prec))
					break;
				if (usage == direct) {
					if (nextPt == inter.begin())
						break;
					nextPt--;
				} else {
					nextPt++;
					if (nextPt == inter.end())
						break;
				}
				if ((*nextPt)->isEqual2D(*(*poly)[0], prec))
					break;
				if (!poly->insertUniqueVertex(PolyPoint(**nextPt)))
					break;
			}
			if (poly->Polygon::isValid() &&
					(((usage == Point::left) && (polyLeft != nullptr)) ||
					((usage != Point::left) && (polyRight != nullptr)))) {
				insertHoles(*poly, myHoles);
					//Add the new polygon to the relevant list
				if (usage == Point::left)
					polyLeft->emplace_back(poly);
				else
					polyRight->emplace_back(poly);
			}
		}
	} else {
		auto where = Point::undefined;
		for (auto vertex = 0; (vertex < vertSize()) && (where != Point::left) && (where != Point::right); ++vertex)
			where = ref.positionOf(*(*this)[vertex]);
			//Add this polygon to the relevant list
		if (where == Point::left) {
			if (polyLeft != nullptr)
				polyLeft->emplace_back(clone(*this));
		} else {
			if (polyRight != nullptr)
				polyRight->emplace_back(clone(*this));
		}
	}
} //Polygon::splitWith


/*--------------------------------------------------------------------
	Calculate the polygons created by splitting this polygon with another polygon
	
	ref: The cutting polygon
	polyIn: The split polygons inside the reference polygon (nullptr = discard)
	polyOut: The split polygons outside the reference polygon (nullptr = discard)
	prec: The required precision
  --------------------------------------------------------------------*/
void Polygon::splitWith(const Polygon& ref, PolyVector* polyIn, PolyVector* polyOut, double prec) const {
	if ((polyIn == nullptr) && (polyOut == nullptr))
		return;
	auto original = clone(*this);
	original->clear();
	original->setHoles();
	auto target = clone(*this);
	auto blade = clone(*original);
	blade->Polygon::operator= (ref);
	auto offcuts = std::make_unique<PolyVector>();
	splitPolyWithPoly(*target, *blade, offcuts.get(), polyOut, prec);
	for (auto hole = ref.getHoleSize(); hole--; ) {
		blade = clone(*original);
		blade->Polygon::operator= (*ref.getHole(hole));
		auto holeOffcuts = std::make_unique<PolyVector>();
		for (auto poly = offcuts->begin(); poly != offcuts->end(); ) {
			splitPolyWithPoly(**poly, *blade, polyOut, holeOffcuts.get(), prec);
			poly = offcuts->erase(poly);
		}
		offcuts.reset(holeOffcuts.release());
	}
	if (polyIn != nullptr) {
		for (auto poly = offcuts->begin(); poly != offcuts->end(); )
			polyIn->emplace_back(offcuts->release(poly));
	}
} //Polygon::splitWith


/*--------------------------------------------------------------------
	Resolve the polygon into a list of polygons with no self-intersection
	
	resolved: The list of resolved polygons
	prec: The required precision
	
	return: The number of resolved polygons created (0 if the polygon is already resolved)
  --------------------------------------------------------------------*/
part_index Polygon::resolveSelfIntersect(PolyVector& resolved, double prec) const {
	auto startSize = static_cast<part_index>(resolved.size());
	PolyVector cache1;
	if (!resolvePolyIntersect(*this, cache1, prec))
		return 0;
	PolyVector cache2;
	auto* unchecked = &cache1;
	auto* processed = &cache2;
		//This is the maximum number of attempts to repair the polygon
	auto testLimit = vertSize();
	PolyVector resolvedBounds;
	while ((unchecked->size() > 0) && (testLimit-- > 0)) {
		for (auto toCheck = unchecked->begin(); toCheck != unchecked->end(); ) {
			if (resolvePolyIntersect(**toCheck, *processed, prec))
				toCheck = unchecked->erase(toCheck);
			else
				resolvedBounds.emplace_back(unchecked->release(toCheck));
		}
		std::swap(unchecked, processed);
	}
	double totalArea = 0.0;
		//Remove polygons with no measureable area
	for (auto resolved = resolvedBounds.begin(); resolved != resolvedBounds.end(); ) {
		double thisArea = (**resolved).getArea();
		if (isZero(thisArea, 10 * prec))
			resolved = resolvedBounds.erase(resolved);
		else {
			++resolved;
			totalArea += thisArea;
		}
	}
		//Now resolve any self-intersection in the hole polygons
	auto* incoming = &resolvedBounds;
	PolyVector buffer;
	if (m_hole) {
		PolyVector resolvedHole;
		for (auto& hole : *m_hole)
			if ((*hole).resolveSelfIntersect(resolvedHole, prec) == 0)
				resolvedHole.emplace_back(clone(*hole));
			//Finally cut the resolved bounding polygons with the resolved holes
		auto* outgoing = &buffer;
		for (auto& hole : resolvedHole) {
			double holeArea = (*hole).getArea();
			if (!isZero(holeArea, 10 * prec) && isLessOrEqual(holeArea, totalArea)) {
				for (auto& outer : *incoming)
					(*outer).splitWith(*hole, nullptr, outgoing, prec);
				incoming->clear();
				std::swap(incoming, outgoing);
			}
		}
	}
	for (auto poly = incoming->begin(); poly != incoming->end(); )
		resolved.emplace_back(incoming->release(poly));
	return (static_cast<part_index>(resolved.size()) - startSize);
} //Polygon::resolveSelfIntersect


/*--------------------------------------------------------------------
	Determine if this crosses a specified line (not just touching)
	
	ref: The reference line
	prec: The required precision
	
	return: True if the polygon perimeter crosses the line
  --------------------------------------------------------------------*/
bool Polygon::crosses(const Line& ref, double prec) const {
	XInfo spec1{Point::within}, spec2{Point::within};
	XList inter{spec1, spec2};
	intersectionWith(ref, inter, prec);
	inter.insert(XPoint{ref.origin, Point::along, Point::origin});
	inter.insert(XPoint{ref.end, Point::along, Point::end});
	inter.sort(ComparePosition{prec});
	auto xpoint = inter.begin();
	Point startPt(**xpoint);
		//Check if the line crosses the polygon interior
	for (; ++xpoint != inter.end(); ) {
		if ((startPt != **xpoint) && (positionOf((startPt + **xpoint) / 2) == Point::inside))
			return true;
		startPt = **xpoint;
	}
	return false;
} //Polygon::crosses


/*--------------------------------------------------------------------
	Determine if the this encloses a specified point (closure assumed)
	
	ref: The reference point
	prec: The coordinate precision
	
	return: True if the point is enclosed by the polygon (in or along)
  --------------------------------------------------------------------*/
bool Polygon::encloses(const Point& ref, double prec) const {
	auto pos = positionOf(ref, prec);
	return ((pos == Point::inside) || (pos == Point::along));
} //Polygon::encloses


/*--------------------------------------------------------------------
	Determine if the this encloses a specified polygon (closure assumed)
	
	ref: The reference point
	prec: The coordinate precision
	
	return: True if the polygon is enclosed by this (in or along)
  --------------------------------------------------------------------*/
bool Polygon::encloses(const Polygon& ref, double prec) const {
		//First check if the bounds of this polygon enclose the reference polygon
	auto myBounds = bounds();
	if (!myBounds)
		return false;
	auto refBounds = ref.bounds();
	if (!refBounds)
		return false;
	if (!myBounds->encloses2D(*refBounds))
		return false;
		//A simple test for any reference vertices outside of this polygon
	for (auto hole = ref.getHoleSize() + 1; hole--; ) {
		const auto& poly = ref.getShape(hole);
		for (auto vertex = 0; vertex < poly.vertSize(); ++vertex)
			if (!encloses(*poly[vertex], prec))
				return false;
	}
		//Another simple test for any vertices of this polygon inside the reference polygon
	for (auto hole = getHoleSize() + 1; hole--; ) {
		const auto& poly = getShape(hole);
		for (auto vertex = 0; vertex < poly.vertSize(); ++vertex)
			if (ref.positionOf(*poly[vertex], prec) == Point::inside)
				return false;
	}
		//Now we need to check whether any edges are outside the perimeter
	auto target{clone(*this)}, blade{clone(ref)};
	XInfo spec1{Point::within}, spec2{Point::within};
	XList inter{spec1, spec2};
	bool isTouching = intersectPolyWithPoly(*target, *blade, inter, prec), isInside = false;
		//Test for any reference edges outside this polygon
	for (auto hole = blade->getHoleSize() + 1; hole--; ) {
		const auto& poly = blade->getShape(hole);
		for (auto vertex = 0; vertex < poly.vertSize(); ++vertex) {
			switch (positionOf(PolyEdge(*poly[vertex - 1], *poly[vertex]).midpoint(), prec)) {
				case Point::outside:
					return false;	//The edge is not enclosed
				case Point::inside:
					isInside = true;	//At least one edge is in the polygon body
					break;
				default:
					break;	//Not interested in other values
			}
		}
	}
	if (isInside)
		return true;
	if (!isTouching)
		return false;
		//Test for edges outside the reference polygon
	for (auto vertex = 0; vertex < target->vertSize(); ++vertex)
		if (ref.positionOf(PolyEdge{*(*target)[vertex - 1], *(*target)[vertex]}.midpoint(), prec) == Point::outside)
			return false;	//The edge is outside
	return true;
} //Polygon::encloses


/*--------------------------------------------------------------------
	Determine if the this overlaps a specified polygon (closure assumed)
	
	ref: The reference point
	prec: The coordinate precision
	
	return: True if the polygon is overlapped by this (partial or total)
  --------------------------------------------------------------------*/
bool Polygon::overlaps(const Polygon& ref, double prec) const {
		//First check if the bounds of this polygon overlap the reference polygon
	auto myBounds = bounds();
	if (!myBounds)
		return false;
	auto refBounds = ref.bounds();
	if (!refBounds)
		return false;
	if (!myBounds->overlaps2D(*refBounds))
		return false;
		//A simple test for any reference vertices inside this polygon
	for (auto hole = ref.getHoleSize() + 1; hole--; ) {
		const auto& poly = ref.getShape(hole);
		for (auto vertex = 0; vertex < poly.vertSize(); ++vertex)
			if (positionOf(*poly[vertex], prec) == Point::inside)
				return true;
	}
		//A simple test for any vertices inside the reference polygon
	for (auto hole = getHoleSize() + 1; hole--; ) {
		const auto& poly = getShape(hole);
		for (auto vertex = 0; vertex < poly.vertSize(); ++vertex)
			if (ref.positionOf(*poly[vertex], prec) == Point::inside)
				return true;
	}
		//Now we need to check if any edges intersect
	auto target{clone(*this)}, blade{clone(ref)};
	XInfo spec1{Point::within}, spec2{Point::within};
	XList inter{spec1, spec2};
	bool isTouching = intersectPolyWithPoly(*target, *blade, inter, prec);
	if (inter.size() > 0)
		return true;	//A non-reflecting intersection is proof of an overlap
		//Test for any edges inside either polygon
	bool isAlong = true;
	for (short order = 0; order < 2; ++order) {
		for (auto hole = blade->getHoleSize() + 1; hole--; ) {
			const auto& poly = blade->getShape(hole);
			for (auto vertex = 0; vertex < poly.vertSize(); ++vertex) {
				switch (target->positionOf(PolyEdge(*poly[vertex - 1], *poly[vertex]).midpoint(), prec)) {
					case Point::inside:
						return true;	//This is proof of an overlap
					case Point::outside:
						isAlong = false;	//The polygons do not perfectly overlap
						break;
					default:
						break;	//Not interested in other values
				}
			}
		}
		std::swap(target, blade);
	}
	return isAlong && isTouching;
} //Polygon::overlaps

//MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Clear the specified polygon content
 
	allVertices: True to clear all vertices
	allHoles: True to clear all holes
  --------------------------------------------------------------------*/
void Polygon::clear(bool allVertices, bool allHoles) {
	if (allVertices) {
		base::clear();
		m_topID = 0;
	}
	if (allHoles)
		m_hole = nullptr;
} //Polygon::clear


/*--------------------------------------------------------------------
	Insert a hole in the polygon
 
	hole: The hole to insert
 
	return: The inserted hole
  --------------------------------------------------------------------*/
Polygon* Polygon::insertHole(const Polygon& hole) {
	return emplaceHole(std::make_unique<Polygon>(hole));
} //Polygon::insertHole


/*--------------------------------------------------------------------
	Insert a hole in the polygon

	hole: The hole to insert (nullptr = create a new polygon)
 
	return: The inserted hole
  --------------------------------------------------------------------*/
Polygon* Polygon::insertHole(Polygon* hole) {
	return emplaceHole(Unique(hole));
} //Polygon::insertHole


/*--------------------------------------------------------------------
	Emplace a hole in the polygon

	hole: The hole to insert
 
	return: The emplaced hole
  --------------------------------------------------------------------*/
Polygon* Polygon::emplaceHole(Polygon::Unique&& hole) {
	hole->isHole = true;
		//Holes can't contain holes
	hole->setHoles();
	if (!m_hole)
		m_hole = std::make_unique<PolyVector>(1);
	auto* result = hole.get();
	m_hole->emplace_back(std::move(hole));
	return result;
} //Polygon::emplaceHole


/*--------------------------------------------------------------------
	Add a hole to the polygon
 
	hole: The hole to add
  --------------------------------------------------------------------*/
Polygon* Polygon::emplaceHole(Polygon&& hole) {
	return emplaceHole(std::make_unique<Polygon>(std::move(hole)));
} //Polygon::emplaceHole


/*--------------------------------------------------------------------
	Remove a hole from the polygon

	which: The index of the hole to remove
  --------------------------------------------------------------------*/
void Polygon::removeHole(part_index which) {
	if (!m_hole || (which < 0) || (which >= m_hole->size()))
		throw std::out_of_range("");
	m_hole->erase(m_hole->begin() + which);
	if (m_hole->empty())
		m_hole.reset();
} //Polygon::removeHole


/*--------------------------------------------------------------------
	Release a hole from the polygon

	which: The index of the hole to release
	
	return: The released hole polygon
  --------------------------------------------------------------------*/
Polygon::Unique Polygon::releaseHole(part_index which) {
	if (!m_hole || (which < 0) || (which >= m_hole->size()))
		throw std::out_of_range("");
	auto pos = m_hole->begin() + which;
	auto result = m_hole->release(pos);
	if (m_hole->empty())
		m_hole.reset();
	return result;
} //Polygon::releaseHole


/*--------------------------------------------------------------------
	Set the holes in the polygon
	
	holes: The list of holes to set in the polygon
  --------------------------------------------------------------------*/
void Polygon::setHoles(PolyVector::Unique&& holes) {
	m_hole = std::move(holes);
} //Polygon::setHoles


/*--------------------------------------------------------------------
	Remove the polygon holes and release them to the caller
	
	return: The holes in the polygon
  --------------------------------------------------------------------*/
PolyVector::Unique Polygon::releaseHoles() {
	return std::move(m_hole);
} //Polygon::releaseHoles


/*--------------------------------------------------------------------
	Introduce a new node along the specified polygon edge
	
	vert: The ID of the edge vertex to start searching from
	pos: The coordinates of the new node
	prec: The required precision
	
	return: The ID of the new node (0 on failure)
  --------------------------------------------------------------------*/
vertex_id Polygon::addNodeAlong(vertex_id ID, const Point& pos, double prec) {
	vertex_index vert = (ID == 0) ? 0 : (findVertexById(ID).value_or(Index())).vertex;
	auto where = Point::undefined, origPos = Point::undefined;
	vertex_index edge;
	vertOption orig;
	for (edge = vert + vertSize(); (edge != vert) && (where != Point::along); edge--) {
		if (!isClosed && (wrapIndex(edge) == 0))
			continue;
		if ((*this)[edge]->isArc())
			where = Arc{*(*this)[edge - 1], *(*this)[edge]}.positionOf2D(pos, prec);
		else
			where = Line{*(*this)[edge - 1], *(*this)[edge]}.positionOf2D(pos, prec);
			//An edge intersection is preferred, but this will be used if none is found
		if (!orig && ((where == Point::end) || (where == Point::origin))) {
			orig = edge;
			origPos = where;
		}
	}
	if (where != Point::along) {
		if (orig) {
			switch (origPos) {
				case Point::origin:
					return (*this)[*orig - 1]->id;
				case Point::end:
					return (*this)[*orig]->id;
				default:
					break;	//Not interested in other values
			}
		}
		return 0;
	}
	vert = wrapIndex(edge + 1);
	auto node{clone(*(*this)[vert])};
	node->id = allocateID();
	node->Point::operator=(pos);
	if (node->isArc()) {
		Arc arc{*(*this)[vert - 1], *(*this)[vert]};
		Arc newArc{arc.centre, arc.getOrigin(), pos, arc.sweep < 0};
		node->sweep = newArc.sweep;
		(*this)[vert]->sweep = arc.sweep - newArc.sweep;
	}
	emplace(begin() + vert, node);
	return node->id;
} //addNodeAlong


/*--------------------------------------------------------------------
	Insert a new node to a polygon, testing that the vertex is unique
	
	pt: The new node to insert
	where: The position to insert the new point (nullopt = at the end)
	
	return: True if the new vertex is unique (not inserted if false)
  --------------------------------------------------------------------*/
bool Polygon::insertUniqueVertex(PolyPoint&& pt, vertOption where) {
	if (findVertexByLocation(pt))
		return false;
	if (!where)
		emplace_back(std::move(pt));
	else
		emplace(begin() + wrapIndex(*where), std::move(pt));
	return true;
} //Polygon::insertUniqueVertex


/*--------------------------------------------------------------------
	Redirect the order of the polygon in the specified direction

	direct: The direction of the polygon
	withHoles: True to also set the direction of holes
	invertHoleDir: True to invert the direction of the holes
  --------------------------------------------------------------------*/
void Polygon::setDirection(Rotation direct, bool withHoles, bool invertHoleDir) {
	auto maxPoly = (withHoles) ? getHoleSize() : 0;
	for (auto shape = 0; shape <= maxPoly; ++shape) {
		auto& target = getShape(shape);
		auto thisDirect = ((shape == 0) || !invertHoleDir) ? direct : (direct == Polygon::clockwise ? anticlockwise : clockwise);
		if (auto direct = target.getDirection(); direct &&(target.getDirection() != thisDirect))
			target.reverse();
	}
} //Polygon::setDirection


/*--------------------------------------------------------------------
	Reverse the polygon direction (clockwise/anticlockwise)
  --------------------------------------------------------------------*/
void Polygon::reverse() {
	if (vertSize() < 2)
		return;
	auto bottom = 0, top = vertSize() - 1;
		//Reverse the vertices
	while (bottom < top)
		base::operator[](bottom++).swap(base::operator[](top--));
		//Reverse arc sweeps
	double swp = -(*this)[0]->sweep;
	for (auto vertex = vertSize(); vertex--; )
		(*this)[vertex + 1]->sweep = -(*this)[vertex]->sweep;
	(*this)[1]->sweep = swp;
} //Polygon::reverse


/*--------------------------------------------------------------------
	Remove duplicate adjoining vertices in 2D

	prec: The coordinate precision

	return: True if duplicates were removed
  --------------------------------------------------------------------*/
bool Polygon::removeDuplicates2D(double prec) {
	bool removed = false;
	auto maxPoly = getHoleSize();
	for (auto shape = 0; shape <= maxPoly; ++shape) {
		auto& poly = getShape(shape);
		auto prevVertex(*poly[poly.vertSize() - 1]);
		for (auto vertex = poly.begin(); vertex != poly.end(); ) {
			if ((**vertex).isEqual2D(prevVertex, prec)) {
				removed = true;
				vertex = poly.erase(vertex);
			} else {
				prevVertex = **vertex;
				++vertex;
			}
		}
	}
	return removed;
} //Polygon::removeDuplicates2D


/*--------------------------------------------------------------------
	Remove duplicate adjoining vertices in 3D
 
	prec: The coordinate precision
	
	return: True if duplicates were removed
  --------------------------------------------------------------------*/
bool Polygon::removeDuplicates3D(double prec) {
	bool removed = false;
	auto maxPoly = getHoleSize();
	for (auto shape = 0; shape <= maxPoly; ++shape) {
		auto& poly = getShape(shape);
		auto prevVertex(*poly[poly.vertSize() - 1]);
		for (auto vertex = poly.begin(); vertex != poly.end(); ) {
			if ((**vertex).isEqual3D(prevVertex, prec)) {
				removed = true;
				vertex = poly.erase(vertex);
			} else {
				prevVertex = **vertex;
				++vertex;
			}
		}
	}
	return removed;
} //Polygon::removeDuplicates3D


/*--------------------------------------------------------------------
	Optimise the polygon (eliminate duplicates, co-linear points)
	
	doColin: True if colinear points should be removed
	prec: The required precision
  --------------------------------------------------------------------*/
void Polygon::optimise(bool doColin, double prec) {
	Point previousPoint{*(*this)[-1]};
	for (auto vertex = 0; vertex <= vertSize(); ) {
		auto* thisPoint = (*this)[vertex];
		bool isErased = thisPoint->isEqual2D(previousPoint, prec);
		auto* nextPoint = (*this)[vertex + 1];
		if (!isErased && doColin && PolyEdge{previousPoint, *thisPoint}.isColinearTo2D(PolyEdge{*thisPoint, *nextPoint}, prec)) {
			isErased = true;
			nextPoint->sweep = nextPoint->sweep + thisPoint->sweep;
		}
		if (isErased)
			erase(begin() + vertex);
		else {
			previousPoint = *thisPoint;
			++vertex;
		}
	}
} //Polygon::optimise


/*--------------------------------------------------------------------
	Renumber the polygon vertices
  --------------------------------------------------------------------*/
void Polygon::renumber(bool restart) {
	m_topID = getTopVertexID(*this);
	std::map<vertex_id, bool> isUsed;
	isUsed[0] = true;
	auto maxPoly = getHoleSize();
	for (auto shape = 0; shape <= maxPoly; ++shape) {
		auto& poly = getShape(shape);
		for (auto vertex = poly.vertSize(); vertex--; ) {
			if (restart || isUsed[poly[vertex]->id])
				poly[vertex]->id = allocateID();
			else
				isUsed[poly[vertex]->id] = true;
		}
		poly.m_topID = m_topID;
	}
} //Polygon::renumber


/*--------------------------------------------------------------------
	Facet the curved edges on the polygon
  --------------------------------------------------------------------*/
void Polygon::facet() {
	for (auto vertex = edgeSize(); vertex--; ) {
		if (!(*this)[vertex + 1]->isArc())
			continue;
		Faceter faceter(*(*this)[vertex], *(*this)[vertex + 1], true, false);
		(*this)[vertex + 1]->sweep = 0.0;
		auto newPos = vertex + 1;
		do {
			++faceter;
			insert(begin() + newPos++, new PolyPoint(faceter.getVertex()));
		} while (!faceter.isAtEnd());
	}
} //Polygon::facet


/*--------------------------------------------------------------------
	Set the level (z coordinate) of all the polygon vertices

	z: The required level
  --------------------------------------------------------------------*/
void Polygon::setBaseLevel(double z) {
	auto maxPoly = getHoleSize();
	for (part_index shape = 0; shape <= maxPoly; ++shape) {
		auto& poly = getShape(shape);
		for (auto vertex = poly.begin(); vertex != poly.end(); ++vertex)
			(**vertex).z = z;
	}
} //Polygon::setBaseLevel


/*--------------------------------------------------------------------
	Align the z coordinates of the polygon vertices to a specified plane

	plane: The plane to align the polygon to
  --------------------------------------------------------------------*/
void Polygon::alignTo(const Plane& plane) {
	auto maxPoly = getHoleSize();
	for (auto shape = 0; shape <= maxPoly; ++shape) {
		auto& poly = getShape(shape);
		for (auto vertex = poly.begin(); vertex != poly.end(); ++vertex)
			(**vertex).z = plane.heightAt(**vertex);
	}
} //Polygon::alignTo


/*--------------------------------------------------------------------
	Find the largest polygon in the list
 
	return: An iterator at the largest polygon (end() on failure)
  --------------------------------------------------------------------*/
PolyVector::const_iterator PolyVector::findLargest() const {
		//NB: Const only discarded to call non-const function (doesn't mutate the object). The returned iterator is const, so const preserved
	return const_cast<PolyVector*>(this)->findLargest();
} //PolyVector::findLargest


/*--------------------------------------------------------------------
	Find the largest polygon in the list
 
	return: An iterator at the largest polygon (end() on failure)
  --------------------------------------------------------------------*/
PolyVector::iterator PolyVector::findLargest() {
		//NB: This function does not mutate the object, but is non-const to return a non-const iterator
	double largestArea = 0.0;
	auto result = end();
	for (auto polygon = begin(); polygon != end(); ++polygon) {
		double area = (**polygon).getArea();
		if (area > largestArea) {
			largestArea = area;
			result = polygon;
		}
	}
	return result;
} //PolyVector::findLargest
