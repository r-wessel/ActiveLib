/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_POINT
#define ACTIVE_PRIMITIVE_POINT

#include "Active/Geometry/Point.h"
#include "Active/Primitives/2D/Primitive.h"

namespace active::primitive {
	
	/*!
	 Base class for 2D geometric primitives
	 */
	class Point : public Primitive, public geometry::Point {
	public:

		// MARK: Constructors

		using geometry::Point::Point;
		
		/*!
		 Constructor
		 @param point The primitive point coordinates
		 @param pn The primitive linework pen
		 @param edgeCol The edge colour (nullopt = no edge)
		*/
		Point(const geometry::Point& point, attribute::Pen pn = attribute::Pen{},
					   attribute::Colour::Option edgeCol = attribute::colour::black) :
				Primitive{pn, edgeCol}, geometry::Point{point} {}
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		Point(const Point& source) = default;
		/*!
		 Move constructor
		 @param source The object to move
		 */
		Point(Point&& source) = default;
		/*!
		 Destructor
		 */
		virtual ~Point() = default;

		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Point* clonePtr() const override { return new Point(*this); }
		
		// MARK: Functions (const)

		/*!
		 Get the primitive rectilinear bounds
		 @return The primitive bounds
		 */
		virtual geometry::Box::Option bounds() const override { return geometry::Box{*this}; }

		// MARK: Functions (mutating)

		/*!
		 Add point coordinates
		 @param toAdd The point to add
		 @return A reference to this
		 */
		virtual Primitive& add(const geometry::Point& toAdd) override;
		/*!
		 Multiply by a specified factor
		 @param toMultiply The factor to multiply the primitive by
		 @param isRenderOffset True to also scale the render offset (where relevant)
		 @return A reference to this
		 */
		virtual Primitive& multiply(double toMultiply, bool isRenderOffset = false) override;
		/*!
		 Multiply by the coordinates of a point
		 @param toMultiply The point coords to multiply by
		 @param isRenderOffset True to also scale the render offset (where relevant)
		 @return A reference to this
		 */
		virtual Primitive& multiply(const geometry::Point& toMultiply, bool isRenderOffset = false) override;
		/*!
		 Multiply by a 3x3 matrix
		 @param toMultiply A 3x3 matrix
		 @param includeRenderSized Include render-sized geometry in the transformation
		 @param isRenderTranslation True if the matrix translation applies to render-sized geometry
		 @return A reference to this
		 */
		virtual Primitive& multiply(const geometry::Matrix3x3& toMultiply, bool includeRenderSized = false, bool isRenderTranslation = false) override;
	};

}

#endif //ACTIVE_PRIMITIVE_POINT
