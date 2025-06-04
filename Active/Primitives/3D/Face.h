/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_FACE
#define ACTIVE_PRIMITIVE_FACE

#include "Active/Primitives/3D/Index.h"

#include <algorithm>
#include <vector>

namespace active::primitive {
	
	/*!
	 A class to represent a face in a 3D mesh
	 
	 A face indexes into the edge table to identify bounding edges and carries indices for materials, normal etc
	*/
	class Face {
	public:

		// MARK: - Types
		
			///A list of edge indices
		using EdgeList = std::vector<Index>;

		// MARK: - Constructors
			
		/*!
		 Default constructor
		 */
		Face() {}
		/*!
		 Constructor
		 @param normIndex Normal index
		 @param finishIndex Finish index
		 */
		Face(Index normIndex, Index finishIndex = npos) {
			finish = finishIndex;
			normal = normIndex;
		}
		Face(const Face&) = default;

			///Indices of the bounding edges
		std::vector<Index> edges;
			///Indices of the vertex normal at each edge end (empty = use face normal)
		std::vector<std::optional<Index>> edgeNormals;
			///Face finish index (npos = trasnparent)
		Index finish = npos;
			///Face plane normal index
		Index normal = npos;
			///True if the face is visible
		bool isVisible = true;

		// MARK: Functions (const)
		
		/*!
		 Find an edge in the face
		 @param toFind the edge index to find
		 @return The index of the edge in the face edge list (npos = not found)
		 */
		Index findEdge(Index toFind) const {
			if (auto iter = std::find(edges.begin(), edges.end(), toFind); iter != edges.end())
				return *iter;
			return npos;
		}
	};
}

#endif //ACTIVE_PRIMITIVE_FACE
