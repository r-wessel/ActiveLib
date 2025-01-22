/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_TEXT
#define ACTIVE_PRIMITIVE_TEXT

#include "Active/Attribute/TextStyle.h"
#include "Active/Primitives/2D/Point.h"

namespace active::primitive {
	
	/*!
	 Class to represent a 2D text primitive
	 */
	class Text : public Point {
	public:

		// MARK: Constructors

		/*!
		 Default constructor
		*/
		Text() {}
		/*!
		 Constructor
		 @param txt The text
		 @param origin Origin point of the text (text is rendered relative to this position using the style anchor and offset)
		 @param angl Rotation angle of the text (about the origin point)
		 @param styl The text style
		 @param offset An offset from the origin (in render units, added after anchor and offset have been applied)
		*/
		Text(const utility::String& txt, const geometry::Point& origin = geometry::Point{}, double angl = 0.0,
					  const attribute::TextStyle styl = attribute::TextStyle(), const geometry::Point& offset = geometry::Point{});
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		Text(const Text& source) = default;
		/*!
		 Move constructor
		 @param source The object to move
		 */
		Text(Text&& source) = default;
		/*!
		 Destructor
		 */
		virtual ~Text() = default;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Text* clonePtr() const override  { return new Text(*this); }

		// MARK: Public variables

			///The primitive text
		utility::String text;
			///Text angle
		double angle = 0.0;
			///Text style
		attribute::TextStyle style;
			///Offset from the origin (in render units, added after anchor and offset have been applied)
		geometry::Point renderOffset;

		// MARK: - Functions (const)
		
			///True if the primitive has visible content
		virtual bool isVisible() const override { return !text.empty() && Point::isVisible(); }
		/*!
			Get the rendering pen (describes weight, 0 = minimum renderable thickness)
			@return The rendering pen
		*/
		virtual attribute::Pen getPen() const override { return style.outlinePen; }
		/*!
			Get the rendering edge colour
			@return The rendering edge colour (nullopt = no edges)
		*/
		virtual attribute::Colour::Option getEdgeColour() const override { return style.outlineColour; }
		/*!
			Get the rendering fill colour
			@return The rendering fill colour (nullopt = no fill)
		*/
		virtual attribute::Colour::Option getFillColour() const override { return style.bodyColour; }
		/*!
			Get the anchor position of the origin of a render-sized primitive
			@return anchor The anchor position (nullopt = determined by primitive)
		*/
		virtual std::optional<geometry::Anchor2D> getRenderAnchor() const override { return style.anchor; }

		// MARK: - Functions (mutating)
		
		/*!
			Set the rendering pen (describes weight/thickness)
			@param pen The rendering pen
		*/
		virtual void setPen(attribute::Pen pen) override { style.outlinePen = pen; }
		/*!
			Set the rendering edge colour
			@param colour The rendering edge colour (nullopt = no edges)
		*/
		virtual void setEdgeColour(attribute::Colour::Option colour) override { style.outlineColour = colour; }
		/*!
			Set the rendering fill colour
			@param colour The rendering fill colour (nullopt = no fill)
		*/
		virtual void setFillColour(attribute::Colour::Option colour) override { style.bodyColour = colour; }
		/*!
			Set the anchor position of the origin of a render-sized primitive
			@param anchor The anchor position (nullopt = determined by primitive)
		*/
		virtual void setRenderAnchor(std::optional<geometry::Anchor2D> anchor) override { style.anchor = anchor.value_or(geometry::Anchor2D::centreHalf); }
	};
	
}

#endif //ACTIVE_PRIMITIVE_TEXT
