/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_MESH_BUILDER
#define ACTIVE_PRIMITIVE_MESH_BUILDER

#include "Active/Geometry/Vector3.h"
#include "Active/Primitives/3D/Edge.h"
#include "Active/Primitives/3D/Index.h"
#include "Active/Primitives/3D/Vertex.h"

#include <memory>
#include <optional>

namespace active::attribute {
	struct Colour;
	struct Finish;
}

namespace active::geometry {
	class Vector3;
}

namespace active::primitive {
	
	class Mesh;
	
	/*!
	 Class for building meshes from vertices, edges, faces etc
	*/
	class MeshBuilder {
	public:
		
		// MARK: - Types
		
			///Description of an edge during mesh-building
		struct RawEdge {
				///The vertex at the edge end
			Vertex vertex;
				///The edge attributes (colour/visibility etc)
			Edge::Attribute attribute;
				///The surface normal at the end vertex (nullopt = no vertex normal, i.e. generate a normal from the face plane)
			std::optional<active::geometry::Vector3> normal;
		};
		
		/*!
		 Constructor
		 */
		MeshBuilder();
		/*!
		 Destructor
		 */
		~MeshBuilder();
		
		// MARK: - Functions (mutating)
		
		/*!
		 Add a colour to the mesh (for an edge) ensuring the mesh table only contains unique colours
		 @param colour The edge colour
		 @return The index of the colour in the mesh table
		 */
		Index addColour(const attribute::Colour& colour);
		/*!
		 Add an edge to the mesh ensuring the mesh table only contains unique edges
		 @param start The edge start vertex
		 @param end The edge end vertex
		 @param attribute The edge attributes (colour etc)
		 @return The index of the normal in the mesh table
		 */
		std::optional<Index> addEdge(const Vertex& start, const Vertex& end, Edge::Attribute attribute);
		/*!
		 Add a face to the mesh ensuring the mesh table only contains unique faces
		 @param edges The face edges (each specifying end vertex and attributes). Assumed that the first edge origin is the last edge end vertex
		 @param finish The face surface finish
		 @return The index of the face in the mesh table
		 */
		std::optional<Index> addFace(const std::vector<RawEdge>& edges, const attribute::Finish& finish);
		/*!
		 Reset the builder data, erasing all contributed vertices, edges etc
		 */
		void reset();
		
		// MARK: - Functions (const)
		
		/*!
		 Get the mesh product from the builder
		 @param findSoftEdges True to set the visibility/softness attributes of edges based on the normals of adjacent faces
		 @return The mesh created by the builder from the contributed faces etc (nullptr if the mesh is empty)
		 */
		std::unique_ptr<Mesh> product(bool findSoftEdges = false) const;
		
	protected:
		/*!
		 Add a vertex to the mesh ensuring the mesh table only contains unique colours
		 @param vertex The vertex to add
		 @return The index of the vertex in the mesh table
		 */
		Index addVertex(const Vertex& vertex);
		/*!
		 Add a surface normal to the mesh ensuring the mesh table only contains unique normals
		 @param normal The normal to add
		 @return The index of the normal in the mesh table
		 */
		Index addNormal(const geometry::Vector3& normal);
		/*!
		 Add a surface finish to the mesh (for a face) ensuring the mesh table only contains unique finishes
		 @param finish The surface finish
		 @return The index of the finish in the mesh table
		 */
		Index addFinish(const attribute::Finish& finish);

	private:
		class Cache;
		
			///Internal data for building meshes
		std::unique_ptr<Cache> m_cache;
	};
	
}

#endif	//ACTIVE_PRIMITIVE_MESH_BUILDER
