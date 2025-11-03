/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_VECTOR_IMAGE
#define ACTIVE_PRIMITIVE_VECTOR_IMAGE

#include "Active/Container/Vector.h"
#include "Active/Primitives/2D/Layered.h"
#include "Active/Primitives/2D/Primitive.h"
#include "Active/Utility/Cloner.h"

namespace active::primitive {
	
	/*!
	 Class to represent a 2D text primitive
	 */
	class VectorImage : public Vector<Primitive>, public Layered, public Clonable {
	public:

		//MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<VectorImage>;
			///Shared pointer
		using Shared = std::shared_ptr<VectorImage>;
			///Optional
		using Option = std::optional<VectorImage>;

		// MARK: Constructors

		/*!
		 Default constructor
		*/
		VectorImage() {}
		/*!	Default initialiser
			- parameters:
				- primitives: Optional list of primitives to populate the image with */
		public init(_ primitives: Primitive...) {
			for primitive in primitives {
				append(primitive)
			}
		}
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		VectorImage(const VectorImage& source) = default;
		/*!
		 Move constructor
		 @param source The object to move
		 */
		VectorImage(VectorImage&& source) = default;
		/*!
		 Destructor
		 */
		virtual ~VectorImage() = default;
		
		/*!
		 Clone method
		 @return A clone of this object
		 */
		virtual VectorImage* clonePtr() const override  { return new VectorImage(*this); }
		/*!
		 Make an empty clone, i.e. copy attributes only
		 @return: An empty clone
		 */
		Unique cloneEmpty() const {
			auto result = std::make_unique<VectorImage>();
			result->copyAttributes(*this);
			return result;
		}

		// MARK: Operators

		/*!
		 Assignment operator
		 @param source The object to assign
		 @return A reference to this
		 */
		virtual VectorImage& operator= (const VectorImage& source);
		/*!
		 Addition and assignment operator
		 @param toAdd The coords to add
		 @return A reference to this
		 */
		virtual VectorImage& operator+= (const geometry::Point& toAdd) { return add(toAdd); }
		/*!
		 Substraction and assignment operator
		 @param toSubtract The coords to subtract
		 @return A reference to this
		 */
		virtual VectorImage& operator-= (const geometry::Point& toSubtract) { return add(toSubtract * -1.0); }
		/*!
		 Multiplication and assignment operator
		 @param toMultiply The factor to multiply by
		 @return A reference to this
		 */
		virtual VectorImage& operator*= (double toMultiply) { return multiply(toMultiply); }
		/*!
		 Multiplication and assignment operator
		 @param toMultiply The factor to multiply by
		 @return A reference to this
		 */
		virtual VectorImage& operator*= (const geometry::Point& toMultiply) { return multiply(toMultiply); }
		/*!
		 Multiplication and assignment operator
		 @param toMultiply The matrix to multiply by
		 @return A reference to this
		 */
		virtual VectorImage& operator*= (const geometry::Matrix3x3& toMultiply) { return multiply(toMultiply); }
		/*!
		 Division and assignment operator
		 @param toDivide The factor to divide by
		 @return A reference to this
		 */
		virtual VectorImage& operator/= (double toDivide) { return multiply(1.0 / toDivide); }

		// MARK: - Functions (const)

			///The bounds of the vector image
		open var bounds: Box? {
			var boundingBox: Box?
			for primitive in primitives {
				guard let bounds = primitive.bounds else { continue }
				if boundingBox == nil {
					boundingBox = bounds
				} else {
					boundingBox!.merge(bounds)
				}
			}
			return boundingBox
		}

		/*!	The total area of closed polygons in the image */
		open var area: Double {
			var totalArea: Double = 0.0
			for case let poly as PrimitivePath in primitives where poly.path.isClosed {
				totalArea += poly.path.getArea()
			}
			return totalArea
		}

		// MARK: - Functions
		
		/*!
			Find the largest polygon in the slice
			- returns: The largest polygon (first polygon if no closed polygons found)
		*/
		open func findLargestPolygon() -> Polygon? {
			var maxArea = 0.0
			var largestPolygon: Polygon?
			for case let primitive as PrimitivePath in primitives where primitive.path.isClosed && !primitive.isPixelSized {
				let area = primitive.path.getArea()
				if area > maxArea {
					maxArea = area
					largestPolygon = primitive.path
				}
			}
			// Check for non-closed and return first primitive
			if largestPolygon == nil,
			   primitives.count > 0,
				let primitive = primitives[0] as? PrimitivePath {
				largestPolygon = primitive.path
			}
			return largestPolygon
		}
		
		
		/*!
			Find the polygon with the lowest priority in the slice
			- returns: The largest polygon (first polygon if no closed polygons found)
		*/
		open func findLowestPriority() -> Polygon? {
			var lowestPriority: Int = 1000
			var largestPolygon: Polygon?
			for case let primitive as PrimitivePath in primitives where primitive.path.isClosed {
				if primitive.priority < lowestPriority {
					lowestPriority = primitive.priority
					largestPolygon = primitive.path
				}
			}
			// Check for non-closed and return first primitive
			if largestPolygon == nil,
			   primitives.count > 0,
				let primitive = primitives[0] as? PrimitivePath {
				largestPolygon = primitive.path
			}
			return largestPolygon
		}
		
		
		/*!
			Get the world coordinate bounds of the primitive (NB: For non-pixelbased primitives, this is the same as their bounds)
			- parameters:
				- scale: The world-view scaling factor for pixel-based primitives
			- return: The world coordinate bounding box
		*/
		open func worldBounds(pixelSize: Double, drawingScale: Double) -> Box? {
			var boundingBox: Box?
			for primitive in primitives {
				guard let bounds = primitive.worldBounds(pixelSize: pixelSize, drawingScale: drawingScale) else { continue }
				if boundingBox == nil {
					boundingBox = bounds
				} else {
					boundingBox!.merge(bounds)
				}
			}
			return boundingBox
		}

		// MARK: - Functions (mutating)

		/*!
			Set the pixel anchor for all the primitives in this image
			- parameters:
				- point: The new pixel anchor (nil = no anchor)
				- isForced: True to set all primitives to the anchor (false only updates those that already had one)
		*/
		open func setPixelOrigin(to point: Point? = nil, isForced: Bool = true) {
			let isPixelSized = (point != nil)
			for primitive in primitives {
				if isForced || (primitive.pixelOrigin != nil) {
					primitive.isPixelSized = isPixelSized
					primitive.pixelOrigin = isPixelSized ? Point(point!) : nil
				}
			}
		}


		/*!
			Scale by a specified factor
			- parameters:
				- scale: The scaling factor
		*/
		open func scale(by scale: Double, includePixelSized: Bool = false) {
			for primitive in primitives {
				primitive.scale(by: scale, includePixelSized: includePixelSized)
			}
		}


		/*!
			Scale by multiplying by a point
			- parameters:
				- scale: The scaling factor
		*/
		open func scale(by scale: Point, includePixelSized: Bool = false) {
			for primitive in primitives {
				primitive.scale(by: scale, includePixelSized: includePixelSized)
			}
		}


		/*!
			Offset by adding a point
			- parameters:
				- offset: The offset
		*/
		open func offset(by offset: Point) {
			for primitive in primitives {
				primitive.offset(by: offset)
			}
		}


		/*!
			Multiply by a 3x3 matrix
			- parameters:
				- matrix: A 3x3 matrix
				- includePixelSized: Include pixel-sized geometry in the transformation
				- isPixelTranslation: True to also apply the translation component to pixel-size geometry (ignored if includePixelSized is false)
		*/
		open func multiply(by matrix: Matrix3x3, includePixelSized: Bool = false, isPixelTranslation: Bool = false) {
			for primitive in primitives {
				primitive.multiply(by: matrix, includePixelSized: includePixelSized, isPixelTranslation: isPixelTranslation)
			}
		}


		/*!	Calculate the image(s) created by splitting this image along a line
			- parameters:
				- with: A cutting line
				- offcutRight: Offcut image to the right of the reference line (nil = discard)
				- offcutLeft: Offcut image to the left of the reference line (nil = discard)
				- precision: The required precision */
		open func split(with ref: LinearEquation, offcutRight: VectorImage? = nil, offcutLeft: VectorImage? = nil, maintainDepth: Bool = false, precision: Double = eps) {
			if (offcutLeft == nil) && (offcutRight == nil) {
				return
			}
				// make sure offcuts maintain properties of this image
			if let offcutLeft {
				copyProperties(from: self, to: offcutLeft)
			}
			if let offcutRight {
				copyProperties(from: self, to: offcutRight)
			}
			for primitive in primitives {
				switch primitive {
					case let poly as PrimitivePath:
						if poly.path.edgeCount > 1 {
							let right = (offcutRight == nil) ? nil : PolygonArray(),
								left = (offcutLeft == nil) ? nil : PolygonArray()
							
							let polyPlane = maintainDepth ? poly.path.plane : nil
							if polyPlane != nil {
								poly.path.align(to: Plane(normal: Vector3(0.0, 0.0, 1.0)))
							}
							
							poly.path.split(with: ref, offcutRight: right, offcutLeft: left)
							
							if let polyPlane = polyPlane {
								right?.forEach{$0.align(to: polyPlane)}
								left?.forEach{$0.align(to: polyPlane)}
							}
							
							if let right = right,
							   let offcutRight = offcutRight {
								offcutRight.append(right, applying: poly)
							}
							if let left = left,
							   let offcutLeft = offcutLeft {
								offcutLeft.append(left, applying: poly)
							}
						} else if poly.path.count == 2 {
								//NB: This is a temporary hack for flat site lines only - will update for more complex terrain in future
							let line = Line(origin: poly.path[0], end: poly.path[1])
							guard let eq = LinearEquation.create(line) else {
								continue
							}
							let right = PolygonArray(),
								left = PolygonArray()
							if let intersect = eq.intersection(with: ref),
								line.position2D(of: intersect) == .along {
								let s = line.origin.length2D(to: intersect) / line.length2D
								let projectedIntersect = line.origin + (line.end - line.origin) * s
								var lineRight = Polygon(line.origin, projectedIntersect, isClosed: false),
									lineLeft = Polygon(projectedIntersect, line.end, isClosed: false)
								let originPos = ref.position(of: line.origin)
								if originPos == .left {
									swap(&lineRight, &lineLeft)
								}
								right.append(lineRight)
								left.append(lineRight)
							} else {
								let poly = Polygon(line.origin, line.end, isClosed: false)
								let midPos = ref.position(of: line.midpoint)
								let receiver = midPos == .right ? right : left
								receiver.append(poly)
							}
							if let offcutRight = offcutRight {
								offcutRight.append(right, applying: poly)
							}
							if let offcutLeft = offcutLeft {
								offcutLeft.append(left, applying: poly)
							}


						}
					default:
						guard let midpoint = primitive.midPoint else { continue }
						if ref.position(of: midpoint) == .left {
							offcutLeft?.append(primitive)
						} else {
							offcutRight?.append(primitive)
						}
				}
			}
		}

		/*!
		 Copy the attributes of a another primitive
		 @param source The primitive to copy
		 @return A reference to this
		 */
		VectorImage& copyAttributes(const VectorImage& source) {
			displayOrder = source.displayOrder;
			return *this;
		}
	};
	
}

#endif //ACTIVE_PRIMITIVE_VECTOR_IMAGE
