/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_MESH
#define ACTIVE_PRIMITIVE_MESH

#include "Active/Attribute/Colour.h"
#include "Active/Attribute/Finish.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Primitives/3D/Edge.h"
#include "Active/Primitives/3D/Face.h"
#include "Active/Primitives/3D/Vertex.h"

#include <vector>

namespace active::geometry {
	class Box;
	class Matrix3x3;
	class Matrix4x4;
}

namespace active::primitive {
	
	/*!
	 A class to represent a point in 3-dimensional space, i.e. with x, y, z coordinates
	 
	 Use this class in preference to Point in cases where memory overheads are important. Point has virtual functions which means that
	 every allocated instance uses an extra 8 bytes.
	 */
	class Mesh {
	public:

		// MARK: - Types

			///A list of vertices
		using VertexList = std::vector<Vertex>;
			///A list of normals
		using NormalList = std::vector<geometry::Vector3>;
			///A list of colours
		using ColourList = std::vector<attribute::Colour>;
			///A list of edges
		using EdgeList = std::vector<Edge>;
			///A list of materials
		using FinishList = std::vector<attribute::Finish>;
			///A list of faces
		using FaceList = std::vector<Face>;

		// MARK: - Constructors

		/*!
		 Default constructor
		 */
		Mesh() {}
		/*!
		 Constructor
		 @param finish A mesh surface finish
		 */
		Mesh(const attribute::Finish& finish) { finishes.push_back(finish); }
		
		// MARK: - Variables

			///Mesh identifier (optional - application-specific)
		utility::String id;
			///Mesh vertices
		VertexList vertices;
			///Mesh normals
		NormalList normals;
			///Mesh edge colours
		ColourList colours;
			///Mesh edges
		EdgeList edges;
			///Mesh face finishes (surface render attributes)
		FinishList finishes;
			///Mesh faces
		FaceList faces;

		// MARK: - Operators

		/*!
		 Addition operator
		 @param offset The offset to add to the mesh vertices
		 @return A mesh offset from this
		 */
		Mesh operator+ (const geometry::Point& offset) const {
			Mesh result{*this};
			result += offset;
			return result;
		}
		/*!
		 Subtraction operator
		 @param offset The offset to add to the mesh vertices
		 @return A mesh offset from this
		 */
		Mesh operator- (const geometry::Point& offset) const { return *this + (offset * -1.0); }
		/*!
		 Multiplication operator
		 @param matrix A matrix to multiply the mesh vertices by
		 @return A mesh multiplied by the matrix
		 */
		Mesh operator*(const geometry::Matrix4x4& matrix) const {
			Mesh result{*this};
			result *= matrix;
			return result;
		}
		/*!
		 Addition assignment operator
		 @param offset The offset to add to the mesh vertices
		 @return A reference to this
		 */
		Mesh& operator+= (const geometry::Point& offset);
		/*!
		 Subtraction assignment operator
		 @param offset The offset to add to the mesh vertices
		 @return A reference to this
		 */
		Mesh& operator-= (const geometry::Point& offset) { return *this += (offset * -1.0); }
		/*!
		 Multiplication assignment operator
		 @param matrix A matrix to multiply the mesh vertices by
		 @return A reference to this
		 */
		Mesh& operator*= (const geometry::Matrix4x4& matrix);

		// MARK: - Functions (const)

			///True if the mesh has no content
		bool empty() const { return vertices.empty() || edges.empty(); }
		/*!
		 Get a sequential list of vertices in a face
		 @param faceIndex The index of the target face
		 @return A list of the vertex indices for a face in sequential order
		 */
		std::vector<Index> faceVertices(Index faceIndex) const;
		/*!
		 Get the mesh bounding box
		 @return The bounding box (mullopt if no bounds can be calculated)
		 */
		std::optional<geometry::Box> bounds() const;
	};

		///A body comprised of any number of meshes
	struct Body : std::vector<Mesh> {};
	
}

#endif //ACTIVE_PRIMITIVE_MESH
