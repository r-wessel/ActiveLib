#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Geometry/Polygon.h"

using namespace active::geometry;
using namespace active::math;

TEST_SUITE(TESTQ(PolygonTests)) TEST_SUITE_OPEN
	
		///Tests for creating/modifying polygons
	TEST_CASE(TESTQ(testPolygonContent)) {
		active::geometry::Polygon testPoly1{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		CHECK_MESSAGE(testPoly1.size() == 3, TEST_MESSAGE(Polygon vertex count wrong));
		CHECK_MESSAGE(isEqual(testPoly1.getArea(), 2.0), TEST_MESSAGE(Polygon area wrong));
		testPoly1.emplace(testPoly1.begin() + 2, PolyPoint{3.0, 4.0});
		CHECK_MESSAGE(testPoly1.size() == 4, TEST_MESSAGE(Polygon vertex count wrong));
		CHECK_MESSAGE(isEqual(testPoly1.getArea(), 4.0), TEST_MESSAGE(Polygon area wrong));
		CHECK_MESSAGE(testPoly1.encloses(active::geometry::Point{3.0, 3.0}), TEST_MESSAGE(Polygon enclosing point failed));
		CHECK_MESSAGE(!testPoly1.encloses(active::geometry::Point{5.0, 6.0}), TEST_MESSAGE(Polygon not enclosing point failed));
		auto testPoly2 = testPoly1;
		CHECK_MESSAGE(testPoly1.isEqual2D(testPoly2), TEST_MESSAGE(Polygon equality failed));
		testPoly2 += active::geometry::Point{1.0, 1.0};
		CHECK_MESSAGE(!testPoly1.isEqual2D(testPoly2), TEST_MESSAGE(Polygon inequality failed));
		CHECK_MESSAGE(testPoly1.overlaps(testPoly2), TEST_MESSAGE(Polygon overlap failed));
	}

TEST_SUITE_CLOSE
