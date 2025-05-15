/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/3D/Mesh.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Box.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"

#include <cmath>

using namespace active;
using namespace active::geometry;
using namespace active::math;
using namespace active::primitive;

/*--------------------------------------------------------------------
	Addition assignment operator
 
	offset: The offset to add to the mesh vertices
 
	return: A reference to this
  --------------------------------------------------------------------*/
Mesh& Mesh::operator+= (const Point& offset) {
	for (auto& vertex : vertices)
		vertex += offset;
	return *this;
} //Mesh::operator+=


/*--------------------------------------------------------------------
	Multiplication assignment operator
 
	scale: The value to multiply the mesh vertices by
 
	return: A reference to this
  --------------------------------------------------------------------*/
Mesh& Mesh::operator*= (double scale) {
	for (auto& vertex : vertices)
		vertex *= scale;
	return *this;
} //Mesh::operator*=


/*--------------------------------------------------------------------
	Multiplication assignment operator
 
	matrix: A matrix to multiply the mesh vertices by
 
	return: A reference to this
  --------------------------------------------------------------------*/
Mesh& Mesh::operator*= (const Matrix4x4& matrix) {
	for (auto& vertex : vertices)
		vertex *= matrix;
	for (auto& normal : normals)
		normal *= matrix;
	return *this;
} //Mesh::operator*=


/*--------------------------------------------------------------------
	Get a sequential list of vertices in a face
 
	faceIndex: The index of the target face
 
	return: A list of the vertex indices for a face in sequential order
  --------------------------------------------------------------------*/
std::vector<Index> Mesh::faceVertices(Index faceIndex) const {
	std::vector<Index> result;
	const auto& face{faces.at(faceIndex)};
	auto lastEdge = edges[face.edges.back()];
	Index previousVertex = edges[face.edges.front()].contains(lastEdge.origin) ? lastEdge.origin : lastEdge.end;
	for (auto edgeIndex : face.edges) {
		auto edge = edges[edgeIndex];
		result.push_back((edge.origin == previousVertex) ? edge.end : edge.origin);
		previousVertex = result.back();
	}
	return result;
} //Mesh::faceVertices


/*--------------------------------------------------------------------
	Get the mesh bounding box
 
	return: The bounding box (mullopt if no bounds can be calculated)
  --------------------------------------------------------------------*/
std::optional<Box> Mesh::bounds() const {
	if (empty())
		return std::nullopt;
	Box bounds{vertices[0]};
	for (size_t i = 0; i < vertices.size(); ++i)
		bounds.merge(vertices[i]);
	return bounds;
} //Mesh::bounds
