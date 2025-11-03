//  Copyright © 2020 CeruleanLabs. All rights reserved.

import Container
import Geometry
import Maths
import Serialization
import Settings

	/// A vector image primitives container. If transformations are required for a specific view rendering, we might add it at this level
open class VectorImage: Clonable, Storable, Layered, Collection {

	// MARK: - Initialisers

	/*!	Default initialiser
		- parameters:
			- primitives: Optional list of primitives to populate the image with */
	public init(_ primitives: Primitive...) {
		for primitive in primitives {
			append(primitive)
		}
	}
	
	
	/*!	Clone Initialiser
		- parameters:
			- original: The original object to clone */
	required public init(_ original: VectorImage) {
		copyProperties(from: original, to: self)
		primitives = original.primitives.clone()
	}
	
	
	/*!
		Make an empty clone of the vector image, i.e. containing no primitives.
		- returns: An empty clone
	*/
	open func cloneEmpty() -> VectorImage {
		let result = VectorImage()
		copyProperties(from: self, to: result)
		return result
	}


	// MARK: - Variables

		///List of constituent primitives
	private var primitives = [Primitive]()
		///An index for the image layer to support layered rendering: ordered low (bottom) -> high (top)
	open var layerIndex: Int = 0
		///Priority for display order within the same layer index: low (below) -> high (above)
	open var priority: Int = 0
		///Optional id for the originating source
	open var id: Guid? = nil
		///Optional link back to the originating source
	open var link: Any? = nil

	// MARK: - Calculated variables

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

	/*!	The number of primitives in the image */
	open var count: VertexIndex {
		return primitives.count
	}
	
	
	open var startIndex: Int {
		return 0
	}
	
	
	open var endIndex: Int {
		return count
	}
	
		///True if any primitive path in the image has a hole
	open var hasHoles: Bool {
		for primitive in primitives {
			if let poly = primitive as? PrimitivePath,
			   poly.path.holeCount > 0 {
				return true
			}
		}
		return false
	}
		///Unique storage ID - rarely applicable to this class
	open var packageID: Guid? { return nil }

	// MARK: - Subscript

	/*!	Checked subscript method - note: index 'wraps' around the polygon, so -1 = last point
		- parameters:
			- index: The index of the required point in the polygon
		- returns: The requested polygon point */
	open subscript(index: Int) -> Primitive {
		get {
			return primitives[index]
		}
		set {
			primitives[index] = newValue
		}
	}

	// MARK: - Container functions

	/*!	Get the index following a specified index (required by Collection protocol)
		- parameters:
			- after: The index
		- returns: The index following the speciffied index */
	open func index(after i: Int) -> Int {
		return i + 1
	}
	
	
	/*!	Append a primitive to the image
		- parameters:
			- primitive: The primitive to append */
	open func append(_ primitive: Primitive) {
		primitives.append(primitive)
	}

		
	/*!	Append another VectorImage to the primitives
		- parameters:
			- vectorImage: The other VectorImage to append */
	open func append(_ vectorImage: VectorImage) {
		for primitive in vectorImage.primitives {
			primitives.append(primitive)
		}
	}


	/*!	Append another VectorImage to the primitives
		- parameters:
			- vectorImage: The other VectorImage to append */
	open func append(_ polys: PolygonArray, applying attributes: PrimitivePath) {
		for poly in polys {
			let primitive = PrimitivePath(poly, applying: attributes)
			primitives.append(primitive)
		}
	}


	/*!
		Extract all primitives from the image
		- returns: All the primitives in the image
	*/
	open func extractAll() -> [Primitive] {
		return primitives
	}


	/*!	Remove all primitives from the image */
	open func removeAll() {
		primitives.removeAll()
	}


	// MARK: - Functions

	
	/*!
		Set the image layer index
		- parameters:
			- index: The layer index to assign
		- returns: A reference to self
	*/
	func onLayer(_ index: Int) -> VectorImage {
		layerIndex = index
		return self
	}
	
	
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



	/*!
		Strip line endings from the image
	*/
	open func stripLineEndings() {
		for primitive in primitives {
			if let polyline = primitive as? PrimitivePath {
				polyline.startSymbolID = nil
				polyline.endSymbolID = nil
			}
		}
	}
	
	
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

	// MARK: Data persistence

		///Variable data tags
	enum Field: String {
		case tag = "vectorImage"
		case primitive
		case priority
	}


	/*!
		Initialiser
		- parameters:
			- from: The incoming package to initialise the object from
	*/
	required public init?(from incoming: Package) {
		if let source = incoming.packages {
			for package in source {
				guard let primitive = package.object() as? Primitive else { continue }
				primitives.append(primitive)
			}
		}
		priority = incoming.value(Field.priority.rawValue)?.int() ?? 0
	}


	/*!
		Pack the object into storage
		- parameters:
			- scope: The scope of the object data to export (nil = default for object)
		- returns: A storable package
	*/
	open func pack(scope: SettingList?) -> Package {
		let package = Package(self, tag: Field.tag.rawValue)
		for primitive in primitives {
			package.append(primitive, tag: Field.primitive.rawValue, scope: scope)
		}
		if priority != 0 {
			package.append(Unit(priority, tag: Field.priority.rawValue))
		}
		return package
	}

}

// MARK: Internal implementation

/*!
	Copy the properties from a previous vector image to a newly created vector image (excluding primitives)
	- parameters:
		- oldVectorImage: The old vector image
		- newVectorImage: The newly created vector image
*/
func copyProperties(from oldVectorImage: VectorImage, to newVectorImage: VectorImage) {
	newVectorImage.layerIndex = oldVectorImage.layerIndex
	newVectorImage.priority = oldVectorImage.priority
	newVectorImage.id = oldVectorImage.id
	newVectorImage.link = oldVectorImage.link
}

extension VectorImage {

	/*!
		Addition and assignment operator
		- parameters:
			- lhs: A vector image - modified by addition
			- rhs: A point, to be added to the vector image
	*/
	public static func += (_ lhs: VectorImage, _ rhs: Point) {
		lhs.offset(by: rhs)
	}


	/*!
		Subtraction and assignment operator
		- parameters:
			- lhs: A vector image - modified by subtraction
			- rhs: A point, to be added to the first
	*/
	public static func -= (_ lhs: VectorImage, _ rhs: Point)  {
		lhs.offset(by: (rhs * -1.0))
	}


	/*!
		Multiplication and assignment operator
		- parameters:
			- lhs: A vector image - modified by multiplication
			- rhs: The scaling factor to multiply the vector image by
	*/
	public static func *= (_ lhs: VectorImage, _ rhs: Double) {
		lhs.scale(by: rhs)
	}


	/*!
		Multiplication and assignment operator
		- parameters:
			- lhs: A vector image - modified by multiplication
			- rhs: The scaling point to multiply the vector image by
	*/
	public static func *= (_ lhs: VectorImage, _ rhs: Point) {
		lhs.scale(by: rhs)
	}


	/*!
		Multiplication and assignment operator
		- parameters:
			- lhs: A vector image - modified by multiplication
			- rhs: A 3x3 matrix
	*/
	public static func *= (_ lhs: VectorImage, _ rhs: Matrix3x3) {
		lhs.multiply(by: rhs)
	}

}
