/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_EDGE
#define ACTIVE_PRIMITIVE_EDGE

#include "Active/Primitives/3D/Index.h"

namespace active::primitive {
	
	/*!
	 A class to represent an edge in a 3D mesh
	 
	 An edge indexes into a vertex table to identity the start and end vertices. Edges carry attributes for colour, weight, visibility and smoothness
	*/
	class Edge {
	public:

		// MARK: - Types
		
			//Edge attribute container
		struct Attribute {
				///The edge colour index (npos = transparent)
			Index colour = npos;
				///The edge thickness (0.0 = hairline)
			double thickness = 0.0;
				///True if the adjacent faces render as a smooth (continuous) surface
			bool isSmooth = false;
				///True if the edge is hidden
			bool isHidden = false;
		};
		

		// MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		Edge() {}
		/*!
		 Constructor
		 @param origIndex Index of the origin vertex (npos = undefined)
		 @param endIndex Index of the end vertex (npos = undefined)
		 */
		Edge(Index origIndex, Index endIndex) {
			origin = origIndex;
			end = endIndex;
		}
		/*!
		 Constructor
		 @param origIndex Index of the origin vertex (npos = undefined)
		 @param endIndex Index of the end vertex (npos = undefined)
		 @param attr The edge attributes
		 */
		Edge(Index origIndex, Index endIndex, const Attribute& attr) : attribute{attr} {
			origin = origIndex;
			end = endIndex;
		}
		Edge(const Edge&) = default;

		// MARK: - Variables
		
			///The index of the edge origin vertex (npos = undefined)
		Index origin = npos;
			///The index of the edge end vertex (npos = undefined)
		Index end = npos;
			///The index of the first face linked to this edge (npos = unlinked)
		Index face1 = npos;
			///The index of the second face linked to this edge (npos = unlinked)
		Index face2 = npos;
			///The edge attribute
		Attribute attribute;

		// MARK: - Functions (mutating)
		
		/*!
		 Link a face to this edge by index
		 @param index The face index (ignored if the edge is already linked to two faces)
		 */
		void addFace(Index index) {
			if (face1 == npos)
				face1 = index;
			else if (face2 == npos)
				face2 = index;
		}
		
		// MARK: - Functions (const)
		
			///True if the edge is visible
		bool isVisible() const { return !attribute.isHidden && !attribute.isSmooth && (attribute.colour != npos); }
			///True if the edge contains the specified vertex index
		bool contains(Index index) const { return (origin == index) || (end == index); }
			///True if the edge has two adjacent faces
		bool hasAdjacentFaces() const { return ((face1 != npos) && (face2 != npos)); }
		/*!
		 Determine if the edge is connected to a specified vertex
		 @param index The vertex index
		 @return True if the edge has the specified vertex index
		*/
		bool hasVertex(Index index) const { return ((origin == index) || (end == index)); }
	};

}

#endif //ACTIVE_PRIMITIVE_EDGE
