
# Geometry

## Contents
1. [Purpose](#purpose)
2. [Overview](#over)

## Purpose <a name="purpose"></a>

The `Geometry` module is a maths toolkit for creating and manipulating geometric entities. Although there are representations of many graphics primitives, e.g. `Polygon`, `Arc`, `Line` etc, these are primarily intended as mathematical tools rather than data containers (although they can be harnessed toward that end). The core focus is on pure geometric data and functions to manage/manipulate them, avoiding any metadata typically linked with rendering, e.g. colour, line-weight, layer etc.

Refer to the `Primitive` and `Attribute` module for a representation of graphics primitives targeting rendering.

## Overview <a name="over"></a>

The module functionality is largely straightforward and is well covered in the source documentation. Note that geometric entities like arc, line and polygon that could be used 2D or 3D combine both capabilities rather than providing specialised 2D and 3D versions, allowing calculations and model data to be interchangeable between 2D and 3D enviroments. The z coordinate can simply be ignored in 2D contexts. Note that some functions specifically denote 2D and 3D variants, e.g. `isEqual2D` vs `isEqual3D`, wherever this differentiate can make a critical difference.

The `Position` enumerator is an important concept for testing the relationship between different components, e.g. the position of a point with respect to a polygon:
```Cpp
if (polygon.positionOf2D(point) == inside)
```
Note that precision also plays an important role in this calculation and most others in the Geometry module. This defaults to 1e-5m (0.01mm), but can be specified as required, e.g. to test if a point is along a polygon edge to the nearest millimetre:
```Cpp
if (polygon.positionOf2D(point, 1e-3) == along)
```
Many functions that calculate intersections are dealing with geometry that might intersect in multiple places. Therefore they populate intersections into a passed XList, e.g.:
```Cpp
/*!
	Get the intersection(s) between this polygon and a line
	@param ref The reference line
	@param inter The intersection list to populate
	@param prec The required precision
	@return The number of intersections calculated
*/
vertex_index intersectionWith(const Line& ref, XList& inter, double prec = math::eps) const;
```
Intersection are accumulated in the XList, i.e. it isn't cleared by these functions, so intersections can be calculated against multiple items before processing.

Intersections can also be within the extents of the geometry or projected along an extension, e.g. 2 lines might not overlap but could be projected to intersect beyond their bounds. This can be filtered by XList, determining what relationship the intersection point should have to the blade geometry or the target geometry. For example, cutting a polygon with a line might calculate intersections between them such that:
- Any intersection along the projected line is accepted (as if the line was infinite)
- Intersections with a polygon edge must either be along the edge or at the end-point (not the origin point to avoid duplicates)
```Cpp
XList intersects{{Point::latter}, {Point::undefined}};
polygon.intersectionWith(line, intersects);
```


The following list provides a simple overview for awareness of the module scope:

- `Arc`: Arc based on centrepoint, radius and start/sweep angles. Can be located in a 3D plane
- `Box`: 3D bounding box
- `Faceter`: Tool class to reduce polygon arc edges to linear facets
- `Leveller`: Tool class to reorientate 3D objects such that a reference plane aligns to the x-y plane
- `Line`: 3D line (origin/end points)
- `LinEquation`: 2D linear equation
- `Matrix3x3`: 3x3 matrix - typically used for 2D transformations
- `Matrix4x4`: 4x4 matrix - typically used for 3D transformations
- `Plane`: 3D plane
- `Point`: 2D point
- `PolyEdge`: Polygon edge based on origin and (PolyPoint) end. Can be located in a 3D plane
- `Polygon`: Polygon including curved edges and holes. Can be located in a 3D plane
- `PolyPoint`: Vectex in a Polygon, including unique ID and edge sweep angle
- `Rotater`: Tool class providing optimised rotations in x, y or z axis
- `Vector3`: Vector with 3 components
- `Vector4`: Vector with 4 components
- `XList`: List of intersection points
- `XPoint`: An intersection point
=