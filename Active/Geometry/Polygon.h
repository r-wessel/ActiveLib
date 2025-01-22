/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_POLYGON
#define ACTIVE_GEOMETRY_POLYGON

#include "Active/Container/List.h"
#include "Active/Container/Vector.h"
#include "Active/Geometry/Box.h"
#include "Active/Geometry/PolyPoint.h"
#include "Active/Geometry/Rotation.h"
#include "Active/Geometry/XPoint.h"
#include "Active/Utility/Cloner.h"

namespace active::geometry {
	
	class Arc;
	class Line;
	class LinEquation;
	class Plane;
	class PolyEdge;
	class PolyVector;
	class PolyVector;
	class XList;

	/*!
		A class to represent a polygon (or open polyline)
	
		NB: Refer to PolyPoint for an description of the vertex/edge class. A polygon/polyline is simply an array of PolyPoint objects (or objects
		serived from PolyPoint). Note that using an array of polymorphic objects (allocated on the heap) is intentional. Although not as fast as a
		contiguous allocation for the entire array, supporting a polymorphic vertex type supports additional metadata describing the edge leading
		to the edge (or the vertex itself in some cases). The base PolyPoint class only defines a sweep angle for the edge, but polygonal shapes
		often add more information, e.g. graphic attributes (colour/linetype/etc) or 3D properties (edge profile/visibility/etc). Maintaining
		parallel arrays or vertices and edges compounds the complexity of all functions that mutate the polygon shape. Any function processing or
		mutating polygon vertices can treat any polygon in the same way irrespective of the associated metadata. Although there is a slight
		speed penalty to this approach, the ease of managing polygon content outweighs it (and alernative may not ultimately prove to be faster).
		
		A `Polygon` can be marked as open or closed - open simply means there is no edge connecting the first and last vertex, i.e. an open polyline.
		This modifies functions like edgeSize (counts the number of edges) which will be less than the number or vertices for open polylines. Closure
		should not be denoted by duplicating the first into the last (this is seen as a redundancy at best).
	 
		The `Polygon` array acts like a loop for indexing or subscripting into a specific vertex, e.g. the last vertex can be accessed like this:
			
	 		auto lastVertex = polygon[-1];
		
		Therefore it is impossible for the index to be out of range (unless the polygon is empty). This greatly simplifies algorithms
		that iterate through the edges - it is perfectly valid to write:
		
	 			double len = 0.0;
				for (auto vertex = edgeSize(); vertex--; )
					len += (*this)[vertex + 1]->lengthFrom2D(*(*this)[vertex]);
		
		…where `vertex + 1` will exceed the array bounds and loop back to the first vertex (0).
	*/
	class Polygon : public container::Vector<PolyPoint>, public utility::Cloner, public utility::Mover {
	public:

		//MARK: - Types

			///Base storage class for Polygon
		using base = container::Vector<PolyPoint>;
			///Unique pointer
		using Unique = std::unique_ptr<Polygon>;
			///Shared pointer
		using Shared = std::shared_ptr<Polygon>;
			///Optional
		using Option = std::optional<Polygon>;

		/// The default number of vertices in a new polygon
		static const vertex_index defSize = 5;

		struct Index {
			part_index part = 0;
			vertex_index vertex = 0;
			const PolyPoint* point = nullptr;
		};
		
		using enum Rotation;

		//MARK: - Constructors
		
		/*!
			Constructor
			@param size The number of vertices to reserve space for
			@param holeSize The number of holes to reserve space for
			@param isClosed Set to true if this polygon is closed
		*/
		Polygon(vertex_index size = defSize, part_index holeSize = 0, bool isClosed = true);
		/*!
			Constructor
			@param points The polygon vertices
			@param isClosed Set to true if this polygon is closed
		*/
		Polygon(const std::initializer_list<PolyPoint>& points, bool isClosed = true) : base(points) { this->isClosed = isClosed; }
		/*!
			Constructor
			@param source The box to base the construction of this polygon on
			@param angle The rotation angle of the box
		*/
		Polygon(const Box& source, double angle = 0.0);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Polygon(const Polygon& source);
		/*!
			Move constructor
			@param source The object to move
		*/
		Polygon(Polygon&& source) noexcept;
		/*!
			Destructor
		*/
		~Polygon() override = default;
		
		/*!
			Clone method
			@return A duplicate of this object
		*/
		Polygon* clonePtr() const override { return new Polygon(*this); }
		/*!
			Object cloning with move
			@return A clone of this object with resources moved to the clone
		*/
		Polygon* movePtr() override { return new Polygon(std::move(*this)); }
		/*!
			Clone method, copying just the raw geometry (all vertices will be reduced to PolyPoints)
			@return A duplicate of this object's geometry
		*/
		virtual Polygon::Unique cloneGeometry() const;
		
		// MARK: - Public variables
		
			///True if the polygon is a hole
		bool isHole = false;
			///True if the polygon is closed, i.e. an edge is assumed from the last vertex to the first
		bool isClosed = true;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		virtual Polygon& operator= (const Polygon& source);
		/*!
			Assignment operator
			@param source The object to move
			@return A reference to this
		*/
		virtual Polygon& operator= (Polygon&& source) noexcept;
		/*!
			Subscript operator
			@param index An index into the polygon
			@return The indexed vertex
		*/
		PolyPoint* operator[] (vertex_index index);
		/*!
			Subscript operator
			@param index An index into the polygon
			@return The indexed vertex
		*/
		const PolyPoint* operator[] (vertex_index index) const;
		/*!
			Addition operator
			@param offset The amount to offset the polygon by
			@return An offset copy of this
		*/
		Polygon operator+ (const Point& offset) const;
		/*!
			Addition with assignment operator
			@param offset The amount to offset the polygon by
			@return A reference to this
		*/
		Polygon& operator+= (const Point& offset);
		/*!
			Subtraction operator
			@param offset The amount to offset the polygon by
			@return An offset copy of this
		*/
		Polygon operator- (const Point& offset) const;
		/*!
			Subtraction with assignment operator
			@param offset The amount to offset the polygon by
			@return A reference to this
		*/
		Polygon& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param mult The amount to multiply the polygon by
			@return A scaled copy of this
		*/
		Polygon operator* (double mult) const;
		/*!
			Multiplication with assignment operator
			@param mult The amount to multiply the polygon by
			@return A reference to this
		*/
		Polygon& operator*= (double mult);
		/*!
			Multiplication operator
			@param mult The amount to multiply the polygon by
			@return A scaled copy of this
		*/
		Polygon operator* (const Point& mult) const;
		/*!
			Multiplication with assignment operator
			@param mult The amount to multiply the polygon by
			@return A reference to this
		*/
		Polygon& operator*= (const Point& mult);
		/*!
			Multiplication operator
			@param mult The matrix to multiply the polygon by
			@return The result
		*/
		Polygon operator* (const Matrix3x3& mult) const;
		/*!
			Multiplication with assignment operator
			@param mult The matrix to multiply the polygon by
			@return A reference to this
		*/
		Polygon& operator*= (const Matrix3x3& mult);
		/*!
			Multiplication operator
			@param mult The matrix to multiply the polygon by
			@return The result
		*/
		Polygon operator* (const Matrix4x4& mult) const;
		/*!
			Multiplication with assignment operator
			@param mult The matrix to multiply the polygon by
			@return A reference to this
		*/
		Polygon& operator*= (const Matrix4x4& mult);
		/*!
			Division operator
			@param mult The amount to scale the polygon by
			@return A scaled copy of this
		*/
		Polygon operator/ (double mult) const;
		/*!
			Division with assignment operator
			@param mult The amount to scale the polygon by
			@return A reference to this
		*/
		Polygon& operator/= (double mult);

		//MARK: - Functions (const)
		
		/*!
			Determine if the polygon is valid (must have 2 or more vertices)
			@param doIntersect True if the polygon should be checked for intersecting edges (crossing each other)
			@param prec The required precision
			@return True if this is a valid polygon
		*/
		virtual bool isValid(bool doIntersect = false, double prec = math::eps) const;
		/*!
			Determine if two polygons are equal in 2D
			@param ref The polygon to compare
			@param prec The required precision
			@return True if the polygons are identical
		*/
		bool isEqual2D(const Polygon& ref, double prec = math::eps) const;
		/*!
			Determine if two polygons are equal in 3D
			@param ref The polygon to compare
			@param prec The required precision
			@return True if the polygons are identical
		*/
		bool isEqual3D(const Polygon& ref, double prec = math::eps) const;
		/*!
			Get the number of vertices in the polygon
			@param isOuter True to count the outer polygon vertices only (exclude holes)
			@return The number of vertices
		*/
		vertex_index vertSize(bool isOuter = true) const;
		/*!
			Get the number of edges in the polygon
			@return The number of edges (dependent on whether a closed path is defined)
		*/
		vertex_index edgeSize() const { return isClosed ? vertSize() : vertSize() - 1; }
		/*!
			Get the number of arc edges in the polygon
			@param isOuter True to count the outer polygon edges only (exclude holes)
			@return The number of arc edges
		*/
		vertex_index arcSize(bool isOuter = true) const;
		/*!
			Get the number of holes in the polygon
			@return The number of holes in the polygon
		*/
		part_index getHoleSize() const;
		/*!
			Get the highest vertex ID
			@return The highest vertex ID
		*/
		vertex_id getTopID() const { return m_topID; }
		/*!
			Get the requested hole
			@param which The index of the polygon to get
			@return The requested polygon
		*/
		Polygon* getHole(part_index which) const;
		/*!
			Get the list of holes in this polygon
			@return The list of holes in this polygon
		*/
		PolyVector* getHoles() const { return m_hole.get(); }
		/*!
			Get the direction of the polygon
			@return The polygon direction (nullopt if the polygon is invalid)
		*/
		std::optional<Rotation> getDirection() const;
		/*!
			Get the polygon bounds
			@return The polygon bounds
		*/
		Box::Option bounds() const;
		/*!
			Calculate the polygon perimeter (non-closure handled)
			@return The total perimeter (plan only)
		*/
		double getPerimeter2D() const;
		/*!
			Calculate the polygon perimeter (non-closure handled)
			@return The total perimeter (3D polyline)
		*/
		double getPerimeter3D() const;
		/*!
			Trace the polygon perimeter for the specified length
		 	@param len The length to length for
		 	@param endPos The point reached
			@return The index of the last vertex passed
		*/
		vertex_index tracePerimeter(double len, PolyPoint& endPos) const;
		/*!
			Calculate the polygon area (closure assumed)
			@param isNet True if the net area is calculated (less holes)
		 	@param isResultSigned True if the result should be signed (reflecting the polygon direction)
			@return The total area
		*/
		double getArea(bool isNet = true, bool isResultSigned = false) const;
		/*!
			Get the internal angle between the edges at the given index
			index The vertex index
			return The angle between the two edges
		*/
		double getInternalAngleAt(vertex_index index) const;
		/*!
			Determine if the edges at a specified vertex are tangential
			@param vertex The index of the vertex to check
			@param anglePrec The angle precision
			@return true if the incident and emerging edges are tangential
		*/
		bool isTangentialAt(vertex_index vertex, double anglePrec = math::epsAngle) const;
		/*!
			Determine if the polygon reflects off a specified line at a specified vertex
			@param index The index of the vertex to test
			@param ref The reference line
			@param prec The required precision
			@return True if a reflection ocurrs at the specified index
		*/
		bool isReflection(vertex_index index, const LinEquation& ref, double prec = math::eps) const;
		/*!
			Get a point inside the polygon
			@return A point inside the polygon (nullopt on failure)
		*/
		Point::Option getInternalPoint() const;
		/*!
			Find a vertex with the specified ID (optionally searching holes)
			@param ID The ID to search for
			@return The index of a vertex with the specified ID (nullopt on failure)
		*/
		std::optional<Index> findVertexById(vertex_id ID) const;
		/*!
			Find a vertex at the specified point
			@param pt The point to match
			@param prec The required precision
			@return The index of a vertex at the specified point (npos = failure)
		*/
		std::optional<Index> findVertexByLocation(const Point& pt, double prec = math::eps) const;
		/*!
			Get one of the subpolygons of which this polygon consists
			@param index The index of the shape
			@return If *index* == 0, the polygon itself, otherwise the hole with number *index* - 1
		*/
		const Polygon& getShape(part_index index) const;
		/*!
			Get one of the subpolygons of which this polygon consists
			@param index The index of the shape
			@return If *index* == 0, the polygon itself, otherwise the hole with number *index* - 1
		*/
		Polygon& getShape(part_index index);
		/*!
			Get an iterator pointing to the specified vertex
			@param index The vertex index (will be wrapped)
			@return The requested iterator
		*/
		base::iterator iteratorAt(vertex_index index);
		/*!
			Get an iterator pointing to the specified vertex
			@param index The vertex index (will be wrapped)
			@return The requested iterator
		*/
		base::const_iterator iteratorAt(vertex_index index) const;
		/*!
			Wrap an index into the polygon range
			@param index The index
			@return The wrapped index
		*/
		vertex_index wrapIndex(vertex_index index) const;
		/*!
			Get the closest point in the polygon to a given point in 3D
			@param ref The reference point
			@param prec The required precision
			@return The closest point in the polygon to the reference point, coupled with the intercept position info
		*/
		XPoint closestPointAlong3D(const Point& ref, bool withHoles, double prec = math::eps) const;
		/*!
			Get the closest point in the polygon to a given point in 2D
			@param ref The reference point
			@param prec The required precision
			@return The closest point in the polygon to the reference point, coupled with the intercept position info
		*/
		XPoint closestPointAlong2D(const Point& ref, bool withHoles, double prec = math::eps) const;
		/*!
			Get the position of a point relative to the polygon (closure assumed)
			@param ref The reference point
			@param prec The required precision
			@return The relative position
		*/
		Position positionOf(const Point& ref, double prec = math::eps) const;
		/*!
			Get the intersection(s) between this polygon and a line
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection(s) between this polygon and an arc
			@param ref The reference arc
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith(const Arc& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection(s) between this polygon and a poly-edge
			@param ref The reference poly-edge
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith(const PolyEdge& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection(s) between this polygon and another polygon
			@param ref The reference polygon
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith(const Polygon& ref, XList& inter, double prec = math::eps) const;
		/*!
			Calculate the polygons created by splitting this polygon along a line
			@param ref The cutting line
			@param polyRight The split polygons to the right of the reference line (nullptr = discard)
			@param polyLeft The split polygons to the left of the reference line (nullptr = discard)
			@param prec The required precision
		*/
		void splitWith(const LinEquation& ref, PolyVector* polyRight = nullptr, PolyVector* polyLeft = nullptr, double prec = math::eps) const;
		/*!
			Calculate the polygons created by splitting this polygon with another polygon
			@param ref The cutting polygon
			@param polyIn The split polygons inside the reference polygon (nullptr = discard)
			@param polyOut The split polygons outside the reference polygon (nullptr = discard)
			@param prec The required precision
		*/
		void splitWith(const Polygon& ref, PolyVector* polyIn = nullptr, PolyVector* polyOut = nullptr, double prec = math::eps) const;
		/*!
			Resolve the polygon into a list of polygons with no self-intersection
			@param resolved The list of resolved polygons
			@param prec The required precision
			@return The number of resolved polygons created (0 if the polygon is already resolved)
		*/
		part_index resolveSelfIntersect(PolyVector& resolved, double prec = math::eps) const;
		/*!
			Determine if this crosses a specified line (not just touching)
			@param ref The reference line
			@param prec The required precision
			@return True if the polygon perimeter crosses the line
		*/
		bool crosses(const Line& ref, double prec = math::eps) const;
		/*!
			Determine if the this encloses a specified point (closure assumed)
			@param ref The reference point
			@param prec The required precision
			@return True if the point is enclosed by the polygon (in or along)
		*/
		bool encloses(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if the this encloses a specified polygon (closure assumed)
			@param ref The reference point
			@param prec The required precision
			@return True if the polygon is enclosed by this (in or along)
		*/
		bool encloses(const Polygon& ref, double prec = math::eps) const;
		/*!
			Determine if the this overlaps a specified polygon (closure assumed)
			@param ref The reference point
			@param prec The required precision
			@return True if the polygon is overlapped by this (partial or total)
		*/
		bool overlaps(const Polygon& ref, double prec = math::eps) const;

		//MARK: - Functions (mutating)
		
		/*!
			Allocate a new vertex ID
			@return A new vertex ID
		*/
		vertex_id allocateID() { return ++m_topID; }
		/*!
			Set the highest vertex ID
			@param topId The highest vertex ID
		*/
		void setTopID(vertex_id topId) { m_topID = topId; }
		/*!
			Clear the specified polygon content
			@param allVertices True to clear all vertices
			@param allHoles True to clear all holes
		*/
		void clear(bool allVertices = true, bool allHoles = true);
		/*!
			Insert a hole in the polygon
			@param hole The hole to insert
			@return The inserted hole
		*/
		Polygon* insertHole(const Polygon& hole);
		/*!
			Insert a hole in the polygon
			@param hole The hole to insert (nullptr = create a new polygon)
			@return The inserted hole
		*/
		Polygon* insertHole(Polygon* hole = nullptr);
		/*!
			Emplace a hole in the polygon
			@param hole The hole to emplace
			@return The emplaced hole
		*/
		Polygon* emplaceHole(Polygon::Unique&& hole);
		/*!
			Emplace a hole in the polygon
			@param hole The hole to emplace
			@return The emplaced hole
		*/
		Polygon* emplaceHole(Polygon&& hole);
		/*!
			Set the holes in the polygon
			@param holes The list of holes to set in the polygon (nullptr to erase all)
		*/
		void setHoles(std::unique_ptr<PolyVector>&& holes = nullptr);
		/*!
			Set the holes in the polygon
			@param hole The list of holes to set in the polygon
		*/
		void setHoles(PolyVector&& hole);
		/*!
			Remove a hole from the polygon
			@param which The index of the hole to remove
		*/
		void removeHole(part_index which);
		/*!
			Release a hole from the polygon
			@param which The index of the hole to release
			@return The released hole polygon
		*/
		Polygon::Unique releaseHole(part_index which);
		/*!
			Introduce a new node along the specified polygon edge
			@param ID The ID of the edge vertex to start searching from
			@param pos The coordinates of the new node
			@param prec The required precision
			@return The ID of the new node (0 on failure)
		*/
		vertex_id addNodeAlong(vertex_id ID, const Point& pos, double prec = math::eps);
		/*!
			Insert a new node to a polygon, testing that the vertex is unique
			@param pt The new node to insert
			@param where The position to insert the new point (nullopt = at the end)
			@return True if the new vertex is unique (not inserted if false)
		*/
		virtual bool insertUniqueVertex(PolyPoint&& pt, vertOption where = std::nullopt);
		/*!
			Remove the polygon holes and release them to the caller
			@return The holes in the polygon
		*/
		std::unique_ptr<PolyVector> releaseHoles();
		/*!
			Redirect the order of the polygon in the specified direction
			@param direct The direction of the polygon
		 	@param withHoles True to also set the direction of holes 
			@param invertHoleDir True to set the opposite sense for the holes
		*/
		virtual void setDirection(Rotation direct = clockwise, bool withHoles = true, bool invertHoleDir = false);
		/*!
			Reverse the polygon direction (clockwise/anticlockwise)
		*/
		virtual void reverse();
		/*!
			Remove duplicate adjoining vertices in 2D
			@param prec The required precision
			@return True if duplicates were removed
		*/
		bool removeDuplicates2D(double prec = math::eps);
		/*!
			Remove duplicate adjoining vertices in 3D
			@param prec The required precision
			@return True if duplicates were removed
		*/
		bool removeDuplicates3D(double prec = math::eps);
		/*!
			Optimise the polygon (eliminate duplicates, co-linear points)
			param doColin True if colinear points should be removed
			@param prec The required precision
		*/
		void optimise(bool doColin = false, double prec = math::eps);
		/*!
			Renumber the polygon vertices
		*/
		void renumber(bool restart = false);
		/*!
			Facet the curved edges on the polygon
		*/
		void facet();
		/*!
			Set the level (z coordinate) of all the polygon vertices
			@param z The required level
		*/
		void setBaseLevel(double z = 0);
		/*!
			Align the z coordinates of the polygon vertices to a specified plane
			@param plane The plane to align the polygon to
		*/
		void alignTo(const Plane& plane);
	
	private:
			///Holes in the polygon
		std::unique_ptr<PolyVector> m_hole;
			///The highest ID allocated to a vertex in this polygon (a simple counter)
		vertex_id m_topID = 0;
	};
	
	
		///A vector of polygons
	class PolyVector : public container::Vector<Polygon> {
	public:

		//MARK: - Types
		
			///Base storage class for Polygon
		using base = container::Vector<Polygon>;
			///Size type for indexing etc
		using size_type = typename base::size_type;
			///Unique pointer
		using Unique = std::unique_ptr<PolyVector>;
			///Shared pointer
		using Shared = std::shared_ptr<PolyVector>;
			///Optional
		using Option = std::optional<PolyVector>;
 
		//MARK: - Constructors
		
		/*!
			Constructor
			@param toReserve The number of polygons to reserve space for
		*/
		PolyVector(size_type toReserve = 0) : base(toReserve) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		PolyVector(const PolyVector& source) : base(source) {}
		/*!
			Destructor
		*/
		~PolyVector() override = default;

		//MARK: - Functions (const)
		
		/*!
			Find the largest polygon in the list
			@return An iterator at the largest polygon (end() on failure)
		*/
		const_iterator findLargest() const;

		//MARK: - Functions (mutating)
		
		/*!
			Find the largest polygon in the list
			@return An iterator at the largest polygon (end() on failure)
		*/
		iterator findLargest();
	};
	
}

#endif	//ACTIVE_GEOMETRY_POLYGON
