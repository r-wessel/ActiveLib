/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_POSITION
#define ACTIVE_GEOMETRY_POSITION

namespace active::geometry {
	
		///Relative spatial position
	enum class Position {
		undefined = 0x00,	///< Not calculated or invalid

		after = 0x01,		///< After the end of an edge
		origin = 0x02,		///< On the origin of an edge
		along = 0x04,		///< Along the span of an edge
		end = 0x08,			///< On the end of an edge
		before = 0x10,		///< Before the origin of an edge
		radial = 0x20,		///< On the extended circle of an arc, outside the arc itself

		outside = 0x01,		///< Outside a closed shape
		inside = 0x10,		///< Inside a closed shape

		left = 0x01,		///< Left of a straight line
		right = 0x10,		///< Right of a straight line

		front = 0x10,		///< In front of a plane
		back = 0x01,		///< At the back of a plane

		within = along | end | origin,  ///< Within the span of an edge, including ends
		vertex = end | origin,	///< On a vertex
		former = along | origin,	///< Within the edge span, excluding the end vertex
		latter = along | end,	///< Within the edge span, excluding the origin vertex
		all = before | origin | along | end | after | radial,	///< Any defined relative position
	};
	
	inline Position operator| (Position left, Position right) {
		return static_cast<Position>(static_cast<short>(left) | static_cast<short>(right));
	}
	
	inline Position operator& (Position left, Position right) {
		return static_cast<Position>(static_cast<short>(left) & static_cast<short>(right));
	}

}

#endif	//ACTIVE_GEOMETRY_POSITION
