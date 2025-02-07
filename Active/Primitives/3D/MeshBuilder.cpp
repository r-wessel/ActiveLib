/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/3D/MeshBuilder.h"

#include "Active/Attribute/Colour.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Plane.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Geometry/Vector4.h"
#include "Active/Primitives/3D/Mesh.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/SHA256.h"
#include "Active/Utility/String.h"

#include <cmath>
#include <map>

using namespace active;
using namespace active::attribute;
using namespace active::geometry;
using namespace active::math;
using namespace active::primitive;
using namespace active::utility;

namespace {
	
	const double alphaTolerance = 1e-3;
	const double coordTolerance = 1e-3;
	
		///Struct to pair matching colours
	struct ColourMatch : public Colour {
		bool operator< (const ColourMatch& ref) const {
			return ((r < ref.r) || ((r == ref.r) &&
					((g < ref.g) || ((g == ref.g) &&
					 ((b < ref.b) || ((b == ref.b) &&
						isLess(a, ref.a, alphaTolerance)))))));
		}
	};

		///Struct to pair matching vertices
	struct VertexMatch : public Vertex {
		bool operator< (const VertexMatch& ref) const {
			return (isLess(x, ref.x, coordTolerance) || (isEqual(x, ref.x, coordTolerance) &&
					(isLess(y, ref.y, coordTolerance) || (isEqual(y, ref.y, coordTolerance) &&
						isLess(z, ref.z, coordTolerance)))));
		}
	};

		///Struct to pair matching normals
	struct NormalMatch : public Vector3 {
		bool operator< (const NormalMatch& ref) const {
			return (isLess((*this)[0], ref[0], coordTolerance) || (isEqual((*this)[0], ref[0], coordTolerance) &&
					(isLess((*this)[1], ref[1], coordTolerance) || (isEqual((*this)[1], ref[0], coordTolerance) &&
						isLess((*this)[2], ref[2], coordTolerance)))));
		}
	};

		///Struct to pair matching edges
	struct EdgeMatch : public std::pair<Index, Index> {
		using base = std::pair<Index, Index>;
		EdgeMatch(const Edge& edgeIn) : base{edgeIn.origin, edgeIn.end}, edge{edgeIn} { if (first > second) std::swap(first, second); }
		operator bool() { return first != second; }
		Edge edge;
	};

		///Struct to pair matching finishes
	struct FinishMatch : public active::attribute::Finish {
		using base = active::attribute::Finish;
		FinishMatch(const active::attribute::Finish& finish) : base{finish}, colour{finish.colour} {}
		ColourMatch colour;
		bool operator< (const FinishMatch& ref) const {
			return (colour < ref.colour) || ((colour == ref.colour) && (id < ref.id));	//This may need to be more expansive in future
		}
	};

		///Struct to pair matching faces
	struct FaceMatch : public Face {
		using base = Face;
		FaceMatch(const Face& face);
		String hash;
		bool operator< (const FaceMatch& ref) const { return hash < ref.hash; }
		operator bool() { return (edges.size() > 2); }
	};

	using ColourTable = std::map<ColourMatch, Index>;
	using VertexTable = std::map<VertexMatch, Index>;
	using NormalTable = std::map<NormalMatch, Index>;
	using EdgeTable = std::map<EdgeMatch, Index>;
	using FinishTable = std::map<FinishMatch, Index>;
	using FaceTable = std::map<FaceMatch, Index>;

}

namespace active::primitive {
	
	class MeshBuilder::Cache {
	public:
		ColourTable colours;
		VertexTable vertices;
		VertexTable normals;
		EdgeTable edges;
		FinishTable finishes;
		FaceTable faces;
	};
	
}

FaceMatch::FaceMatch(const Face& face) : base{face} {
	std::vector<Index> edges;
	for (auto& edge : face.edges)
		edges.push_back(edge);
	std::sort(edges.begin(), edges.end());
	active::utility::SHA256 hasher;
	for (auto& edge : edges)
		hasher << edge;
	hash = hasher.base64Hash();
} //MeshBuilder::FaceMatch::FaceMatch


/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
MeshBuilder::MeshBuilder() { reset(); }


/*--------------------------------------------------------------------
	Destructor
  --------------------------------------------------------------------*/
MeshBuilder::~MeshBuilder() {}


/*--------------------------------------------------------------------
	Add a colour to the mesh (for an edge) ensuring the mesh table only contains unique colours
 
	colour: The edge colour
 
	return: The index of the colour in the mesh table
  --------------------------------------------------------------------*/
Index MeshBuilder::addColour(const Colour& colour) {
	ColourMatch match{colour};
	auto& colours{m_cache->colours};
	if (auto iter = colours.find(match); iter != colours.end())
		return iter->second;
	auto result = static_cast<Index>(colours.size());
	colours[match] = result;
	return result;
} //MeshBuilder::addColour


/*--------------------------------------------------------------------
	Add an edge to the mesh ensuring the mesh table only contains unique edges
 
	start: The edge start vertex
	end: The edge end vertex
	attribute The edge attributes (colour etc)
 
	return: The index of the normal in the mesh table
  --------------------------------------------------------------------*/
std::optional<Index> MeshBuilder::addEdge(const Vertex& start, const Vertex& end, Edge::Attribute attribute) {
	auto originIndex = addVertex(start),
			endIndex = addVertex(end);
	EdgeMatch match{Edge{originIndex, endIndex, attribute}};
	if (!match)
		return std::nullopt;
	auto& edges{m_cache->edges};
	if (auto iter = edges.find(match); iter != edges.end())
		return iter->second;
	auto result = static_cast<Index>(edges.size());
	edges[match] = result;
	return result;
} //MeshBuilder::addEdge


/*--------------------------------------------------------------------
	Add a face to the mesh ensuring the mesh table only contains unique faces
 
	edges: The face edges (each specifying end vertex and attributes). Assumed that the first edge origin is the last edge end vertex
	finish: The face surface finish
 
	return: The index of the face in the mesh table
  --------------------------------------------------------------------*/
std::optional<Index> MeshBuilder::addFace(const std::vector<RawEdge>& vertices, const active::attribute::Finish& finish) {
	if (vertices.size() < 3)
		return std::nullopt;
	Face face;
	face.finish = addFinish(finish);
	std::vector<Vertex> planeSetout;
	bool isVertexNormal = false;
	auto previous = vertices.back();
	for (auto& current : vertices) {
		auto edge = addEdge(previous.vertex, current.vertex, current.attribute);
		if (!edge)
			continue;
		if (planeSetout.size() < 3)
			planeSetout.push_back(current.vertex);
		face.edges.push_back(*edge);
		std::optional<Index> normalIndex;
		if (current.normal) {
			isVertexNormal = true;
			normalIndex = addNormal(*current.normal);
		}
		face.edgeNormals.push_back(normalIndex);
		previous = current;
	}
	if (planeSetout.size() < 3)
		return std::nullopt;
	auto facePlane = Plane::create(planeSetout[2], planeSetout[1], planeSetout[0]);
	if (!facePlane)
		return std::nullopt;
	face.normal = addNormal(facePlane->getNormal());
	if (isVertexNormal) {
		isVertexNormal = false;
		std::for_each(face.edgeNormals.begin(), face.edgeNormals.end(), [&face, &isVertexNormal](auto& index){
			if (index == std::nullopt)
				return;
			if (index == face.normal)
				index = std::nullopt;
			else
				isVertexNormal = true;
		});
	}
	if (!isVertexNormal)
		face.edgeNormals.clear();
	FaceMatch match{face};
	auto& faces{m_cache->faces};
	if (auto iter = faces.find(match); iter != faces.end())
		return iter->second;
	auto result = static_cast<Index>(faces.size());
	faces[match] = result;
	return result;
} //MeshBuilder::addFace


/*--------------------------------------------------------------------
	Reset the builder data, erasing all contributed vertices, edges etc
  --------------------------------------------------------------------*/
void MeshBuilder::reset() {
	m_cache = std::make_unique<Cache>();
} //MeshBuilder::reset


/*--------------------------------------------------------------------
	Get the mesh product from the builder
 
	findSoftEdges: True to set the visibility/softness attributes of edges based on the normals of adjacent faces
 
	return: The mesh created by the builder from the contributed faces etc
  --------------------------------------------------------------------*/
Mesh MeshBuilder::product(bool findSoftEdges) const {
	Mesh result;
	auto& colours{m_cache->colours};
	result.colours.resize(colours.size());
	for (auto& colour : colours)
		result.colours[colour.second] = colour.first;
	auto& vertices{m_cache->vertices};
	result.vertices.resize(vertices.size());
	for (auto& vertex : vertices)
		result.vertices[vertex.second] = vertex.first;
	auto& normals{m_cache->normals};
	result.normals.resize(normals.size());
	for (auto& normal : normals)
		result.normals[normal.second] = normal.first;
	auto& edges{m_cache->edges};
	result.edges.resize(edges.size());
	for (auto& edge : edges)
		result.edges[edge.second] = edge.first.edge;
	auto& finishes{m_cache->finishes};
	result.finishes.resize(finishes.size());
	for (auto& finish : finishes)
		result.finishes[finish.second] = finish.first;
	auto& faces{m_cache->faces};
	result.faces.resize(faces.size());
	for (auto& face : faces) {
		result.faces[face.second] = face.first;
			//Mark the face adjacencies in the mesh edges
		for (auto& edge : face.first.edges)
			result.edges[edge].addFace(face.second);
	}
	return result;
} //MeshBuilder::product


/*--------------------------------------------------------------------
	Add a vertex to the mesh ensuring the mesh table only contains unique colours
 
	vertex: The vertex to add
 
	return: The index of the vertex in the mesh table
  --------------------------------------------------------------------*/
Index MeshBuilder::addVertex(const Vertex& vertex) {
	VertexMatch match{vertex};
	auto& vertices{m_cache->vertices};
	if (auto iter = vertices.find(match); iter != vertices.end())
		return iter->second;
	auto result = static_cast<Index>(vertices.size());
	vertices[match] = result;
	return result;
} //MeshBuilder::addVertex


/*--------------------------------------------------------------------
	Add a surface normal to the mesh ensuring the mesh table only contains unique normals
 
	normal: The normal to add
 
	return: The index of the normal in the mesh table
  --------------------------------------------------------------------*/
Index MeshBuilder::addNormal(const Vector3& normal) {
	VertexMatch match{normal};
	auto& normals{m_cache->normals};
	if (auto iter = normals.find(match); iter != normals.end())
		return iter->second;
	auto result = static_cast<Index>(normals.size());
	normals[match] = result;
	return result;
} //MeshBuilder::addNormal


/*--------------------------------------------------------------------
	Add a surface finish to the mesh (for a face) ensuring the mesh table only contains unique finishes
 
	finish: The surface finish
 
	return: The index of the finish in the mesh table
  --------------------------------------------------------------------*/
Index MeshBuilder::addFinish(const active::attribute::Finish& finish) {
	FinishMatch match{finish};
	auto& finishes{m_cache->finishes};
	if (auto iter = finishes.find(match); iter != finishes.end())
		return iter->second;
	auto result = static_cast<Index>(finishes.size());
	finishes[match] = result;
	return result;
} //MeshBuilder::addFinish
