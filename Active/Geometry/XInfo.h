/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_X_INFO
#define ACTIVE_GEOMETRY_X_INFO

#include "Active/Geometry/Point.h"

namespace active::geometry {
	
	/*!
		Information about an intersection point, e.g. the intersecting component and its geometric relationship to it
	 
		NB: The word "intersection" is generally abbreviated to "X" in class names, e.g. `XPoint`, `XList` etc.
		To understand this class, consider the calculation of the intersection between a polygon (with holes) and a line. There could be any number
		of intersections and, depending on the criteria, might include both actual and projected intersections (where an intersection would occur
		if the line was extended in either direction). In the first instance, a tool evaluating these intersections might want to stipulate that
		it is only interested in specific intersections, e.g. only:
		- Actual intersections (no projected intersections);
		- 'Along' or 'at the origin of' a polygon edge.
		This is the purpose of the `pos` member variable - it describes the relation between the intersecting component (in this case a polygon edge)
		and the intersection point, e.g. `along`, `origin` etc. There will also be equivalent information for the intersecting line, i.e. what is the
		relation of the intersection point with the line.
		Once these intersection have been calculated, the tool might need to perform calculations using the intersectig polygon edges (and possibly
		the intersecting line). This is the purpose of the other member variables, which for this example would be:
		- `vertexIndex`: The vertex index at the intersecting edge end
		- `partIndex`: The index of the intersecting polygon (0 = the outer polygon boundary, otherwise the index of the hole polygon boundary)
		Therefore, any intersection calculation will generate 2 `Xinfo` objects, one for each intersecting component. The list collecting the
		intersections (see `XList`) will also include 2 `Xinfo` objects specifying the criteria for retaining an intersection point.
	*/
	class XInfo {
	public:
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param pos The relationship of the point to the component (optional)
			@param vertIndex The ID of the intersecting component vertex (optional)
			@param partIndex The ID of the component part (optional)
		*/
		XInfo(Position pos = Point::undefined, vertOption vertIndex = std::nullopt, partOption partIndex = std::nullopt) {
			this->pos = pos;
			this->vertexIndex = vertIndex;
			this->partIndex = partIndex;
		}
		
		// MARK: Variables
		
			///The relative position of the point to the component
		Position pos;
			///The ID of the intersecting component vertex (optional)
		vertOption vertexIndex;
			///The ID of the component part (optional)
		partOption partIndex;
	};
		
}

#endif	//ACTIVE_GEOMETRY_X_INFO
