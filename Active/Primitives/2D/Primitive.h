/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_PRIMITIVE
#define ACTIVE_PRIMITIVE_PRIMITIVE

#include "Active/Attribute/Colour.h"
#include "Active/Attribute/Pen.h"
#include "Active/Geometry/Anchor2D.h"
#include "Active/Geometry/Box.h"
#include "Active/Geometry/Point.h"
#include "Active/Primitives/2D/Layered.h"
#include "Active/Utility/Cloner.h"

namespace active::geometry {
	class Polygon;
}

namespace active::primitive {
	
	/*!
	 Base class for 2D geometric primitives
	 */
	class Primitive: public Layered, public utility::Cloner {
	public:

		//MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Primitive>;
			///Shared pointer
		using Shared = std::shared_ptr<Primitive>;
			///Optional
		using Option = std::optional<Primitive>;

		// MARK: Constructors

		/*!
			Default constructor
		*/
		Primitive() = default;
		/*!
			Constructor
			@param pn The primitive linework pen
			@param edgeCol The edge colour (nullopt = no edge)
			@param fillCol The file colour (nullopt = no fill)
		*/
		Primitive(attribute::Pen pn, attribute::Colour::Option edgeCol = std::nullopt, attribute::Colour::Option fillCol = std::nullopt) :
				m_pen{pn}, m_edgeColour{edgeCol}, m_fillColour{fillCol} {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Primitive(const Primitive& source);
		/*!
			Move constructor
			@param source The object to move
		*/
		Primitive(Primitive&& source) = default;
		/*!
			Destructor
		*/
		virtual ~Primitive() = default;
		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Primitive* clonePtr() const = 0;

		// MARK: Operators

		/*!
			Assignment operator
			@param source The object to assign
			@return A reference to this
		*/
		virtual Primitive& operator= (const Primitive& source);
		/*!
			Addition and assignment operator
			@param toAdd The coords to add
			@return A reference to this
		*/
		virtual Primitive& operator+= (const geometry::Point& toAdd) { return add(toAdd); }
		/*!
			Substraction and assignment operator
			@param toSubtract The coords to subtract
			@return A reference to this
		*/
		virtual Primitive& operator-= (const geometry::Point& toSubtract) { return add(toSubtract * -1.0); }
		/*!
			Multiplication and assignment operator
			@param toMultiply The factor to multiply by
			@return A reference to this
		*/
		virtual Primitive& operator*= (double toMultiply) { return multiply(toMultiply); }
		/*!
			Multiplication and assignment operator
			@param toMultiply The factor to multiply by
			@return A reference to this
		*/
		virtual Primitive& operator*= (const geometry::Point& toMultiply) { return multiply(toMultiply); }
		/*!
			Multiplication and assignment operator
			@param toMultiply The matrix to multiply by
			@return A reference to this
		*/
		virtual Primitive& operator*= (const geometry::Matrix3x3& toMultiply) { return multiply(toMultiply); }
		/*!
			Division and assignment operator
			@param toDivide The factor to divide by
			@return A reference to this
		*/
		virtual Primitive& operator/= (double toDivide) { return multiply(1.0 / toDivide); }

		// MARK: Functions (const)

			///True if the primitive has visible content
		virtual bool isVisible() const { return hasFill() || hasEdge(); }
			///True if the primitive has a visible fill
		bool hasFill() const { return getFillColour().has_value(); }
			///True if the primitive has a visible edge
		bool hasEdge() const { return getEdgeColour().has_value(); }
		/*!
			Determine if the primitive size/bounds is for literal rendered output (pixels), i.e. not sized in scalable world units
			@return True if the primitive size/bounds is for literal rendered output
		*/
		virtual bool isRenderSized() const { return m_isRenderSized; }
		/*!
			Get the rendering pen (describes weight, 0 = minimum renderable thickness)
			@return The rendering pen
		*/
		virtual attribute::Pen getPen() const { return m_pen; }
		/*!
			Get the rendering edge colour
			@return The rendering edge colour (nullopt = no edges)
		*/
		virtual attribute::Colour::Option getEdgeColour() const { return m_edgeColour; }
		/*!
			Get the rendering fill colour
			@return The rendering fill colour (nullopt = no fill)
		*/
		virtual attribute::Colour::Option getFillColour() const { return m_fillColour; }
		/*!
			Get the anchor position of the origin of a render-sized primitive
			@return The anchor position (nullopt = determined by primitive)
		*/
		virtual std::optional<geometry::Anchor2D> getRenderAnchor() const { return m_renderAnchor; }
		/*!
			Get the midpoint of the primitive
			@return The primitive midpoint
		*/
		virtual geometry::Point::Option midpoint() const;
		/*!
			Get the primitive rectilinear bounds
			@return The primitive bounds
		*/
		virtual geometry::Box::Option bounds() const { return std::nullopt; }
		/*!
			Get the primitive rectilinear bounds in world coordinates
			@param pixelSize The renderer pixel size for the scene
			@param scale The primitive rendering scale
			@return The primitive bounds in world coordinates
		*/
		virtual geometry::Box::Option worldBounds(double pixelSize, double scale);
		/*!
			Get the primitive path
			@param pixelSize The renderer pixel size
			@param scale The primitive rendering scale
			@return A polyline describing the path or perimeter of the primitive
		*/
		virtual std::unique_ptr<geometry::Polygon> worldPath(double pixelSize, double scale) { return nullptr; }
		/*!
			Determine if the primitive encloses a specified point
			@param point The point
			@param prec The required precision
			@return: True if the primitive bounds encloses the point
		*/
		virtual bool encloses(const geometry::Point& point, double prec = math::eps) { return false; }
		/*!
			Determine if the primitive world bounds encloses a point
			@param point The point
			@param pixelSize The renderer pixel size
			@param scale The primitive rendering scale
			@param prec The required precision
			@return True if the primitive bounds encloses the point
		*/
		virtual bool enclosesWorld(const geometry::Point& point, double pixelSize, double scale, double prec = math::eps);
		
		// MARK: Functions (mutating)

		/*!
			Set whether the primitive size/bounds is for literal rendered output (pixels), i.e. not sized in scalable world units
			@param state True if the primitive size/bounds is for literal rendered output
		*/
		virtual void setRenderSized(bool state) { m_isRenderSized = state; }
		/*!
			Set the rendering pen (describes weight/thickness)
			@param pen The rendering pen
		*/
		virtual void setPen(attribute::Pen pen) { m_pen = pen; }
		/*!
			Set the rendering edge colour
			@param colour The rendering edge colour (nullopt = no edges)
		*/
		virtual void setEdgeColour(attribute::Colour::Option colour) { m_edgeColour = colour; }
		/*!
			Set the rendering fill colour
			@param colour The rendering fill colour (nullopt = no fill)
		*/
		virtual void setFillColour(attribute::Colour::Option colour) { m_fillColour = colour; }
		/*!
			Set the anchor position of the origin of a render-sized primitive
			@param anchor The anchor position (nullopt = determined by primitive)
		*/
		virtual void setRenderAnchor(std::optional<geometry::Anchor2D> anchor) { m_renderAnchor = anchor; }
		/*!
			Add point coordinates
			@param toAdd The point to add
			@return A reference to this
		*/
		virtual Primitive& add(const geometry::Point& toAdd);
		/*!
			Multiply by a specified factor
			@param toMultiply The factor to multiply the primitive by
			@param isRenderOffset True to also scale the render offset (where relevant)
			@return A reference to this
		*/
		virtual Primitive& multiply(double toMultiply, bool isRenderOffset = false);
		/*!
			Multiply by the coordinates of a point
			@param toMultiply The point coords to multiply by
			@param isRenderOffset True to also scale the render offset (where relevant)
			@return A reference to this
		*/
		virtual Primitive& multiply(const geometry::Point& toMultiply, bool isRenderOffset = false);
		/*!
			Multiply by a 3x3 matrix
			@param toMultiply A 3x3 matrix
			@param includeRenderSized Include render-sized geometry in the transformation
			@param isRenderTranslation True if the matrix translation applies to render-sized geometry
			@return A reference to this
		*/
		virtual Primitive& multiply(const geometry::Matrix3x3& toMultiply, bool includeRenderSized = false, bool isRenderTranslation = false);
		/*!
			Copy the attributes of a another primitive
			@param source The primitive to copy
			@param isOriginalFill True to retain the original fill
			@return A reference to this
		*/
		virtual Primitive& copyAttributes(const Primitive& source, bool isOriginalFill = false);
		
	private:
			///Pen for linework
		attribute::Pen m_pen;
			///Edge colour (nullopt = no edges)
		attribute::Colour::Option m_edgeColour = attribute::colour::black;
			///Fill colour (nullopt = no fill)
		attribute::Colour::Option m_fillColour;
			///True if the primitive size/bounds is for literal rendered output (pixels), i.e. not sized in scalable world units
		bool m_isRenderSized = false;
			///The origin for a render-sized primitive
		geometry::Point::Unique m_renderOrigin;
			///The offset of a render-sized primitive from its origin
		geometry::Point::Unique m_renderOffset;
			///The anchor position of the origin of a render-sized primitive (on the rectilinear bounds)
		std::optional<geometry::Anchor2D> m_renderAnchor;
	};

}

#endif //ACTIVE_PRIMITIVE_PRIMITIVE
