/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_ANCHOR_2D
#define ACTIVE_GEOMETRY_ANCHOR_2D

#include "Active/Utility/String.h"

namespace active::geometry {
	
		///Ordinals in the X axis
	typedef enum {
		leftX = 0,
		centreX,
		rightX
	} OrdinalX;
		
		///Ordinals in the Y axis
	typedef enum {
		frontY = 0,
		halfY,
		backY
	} OrdinalY;
	
		///Ordinals in the Z axis
	typedef enum {
		bottomZ = 0,
		middleZ,
		topZ
	} OrdinalZ;
	
		///Anchor positions in 2D
	enum class Anchor2D {
		leftFront = (frontY * 3) + leftX,
		centreFront = (frontY * 3) + centreX,
		rightFront = (frontY * 3) + rightX,
		leftHalf = (halfY * 3) + leftX,
		centreHalf = (halfY * 3) + centreX,
		rightHalf = (halfY * 3) + rightX,
		leftBack = (backY * 3) + leftX,
		centreBack = (backY * 3) + centreX,
		rightBack = (backY * 3) + rightX
	};

	/*!
		Get an Anchor2D enumerator from text
		@param text The incoming text
		@return The equivalent anchor
	*/
	std::optional<Anchor2D> toAnchor2D(const utility::String& text);
	/*!
		Get the text for a Anchor2D value
		@param anchor The incoming anchor
		@return The anchor name as text
	*/
	utility::String fromAnchor2D(Anchor2D anchor);

}

#endif	//ACTIVE_GEOMETRY_ANCHOR_2D
