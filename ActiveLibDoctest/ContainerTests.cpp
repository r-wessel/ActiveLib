#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Container/HashMap.h"
#include "Active/Container/List.h"
#include "Active/Container/Map.h"
#include "Active/Container/MultiMap.h"
#include "Active/Container/Vector.h"
#include "Active/Geometry/Polygon.h"
#include "Active/Utility/String.h"

using namespace active;
using namespace active::container;
using namespace active::geometry;
using namespace active::utility;

TEST_SUITE(TESTQ(ContainerTests)) TEST_SUITE_OPEN

  		///Tests for Vector
	TEST_CASE(TESTQ(testVector)) {
		geometry::Polygon original{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		auto other = original;
		Vector<geometry::Polygon> first;
		first.emplace_back(std::move(original));
		CHECK_MESSAGE(first.size() == 1, TEST_MESSAGE(Vector emplace with moved rvalue failed));
		CHECK_MESSAGE(original.empty(), TEST_MESSAGE(Vector emplace with move made a copy rather than moving));
		auto second = first;
		CHECK_MESSAGE(second.size() == 1, TEST_MESSAGE(Vector copy failed));
		CHECK_MESSAGE(first[0]->isEqual2D(*second[0]), TEST_MESSAGE(Vector copy content wrong));
		auto third = std::move(second);
		CHECK_MESSAGE(third.size() == 1, TEST_MESSAGE(Vector move assignment failed));
		CHECK_MESSAGE(second.empty(), TEST_MESSAGE(Vector move assignment made a copy rather than moving));
		CHECK_MESSAGE(first[0]->isEqual2D(*third[0]), TEST_MESSAGE(Vector move assignment content wrong));
	}

  		///Tests for List
	TEST_CASE(TESTQ(testList)) {
		geometry::Polygon original{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		auto other = original;
		List<geometry::Polygon> first;
		first.emplace_back(std::move(original));
		CHECK_MESSAGE(first.size() == 1, TEST_MESSAGE(List emplace with moved rvalue failed));
		CHECK_MESSAGE(original.empty(), TEST_MESSAGE(List emplace with move made a copy rather than moving));
		auto second = first;
		CHECK_MESSAGE(second.size() == 1, TEST_MESSAGE(List copy failed));
		CHECK_MESSAGE(first.front()->isEqual2D(*second.front()), TEST_MESSAGE(List copy content wrong));
		auto third = std::move(second);
		CHECK_MESSAGE(third.size() == 1, TEST_MESSAGE(List move assignment failed));
		CHECK_MESSAGE(second.empty(), TEST_MESSAGE(List move assignment made a copy rather than moving));
		CHECK_MESSAGE(first.front()->isEqual2D(*third.front()), TEST_MESSAGE(List move assignment content wrong));
	}

  		///Tests for Map
	TEST_CASE(TESTQ(testMap)) {
		const char* testKey = "Shape";
		const char* testKey2 = "Shape2";
		geometry::Polygon original{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		auto other = original;
		Map<utility::String, geometry::Polygon> first;
		first.emplace(testKey, std::move(original));
		CHECK_MESSAGE(first.size() == 1, TEST_MESSAGE(Map insert with moved rvalue failed));
		CHECK_MESSAGE(original.empty(), TEST_MESSAGE(Map insert with move made a copy rather than moving));
		auto second = first;
		CHECK_MESSAGE(second.size() == 1, TEST_MESSAGE(Map copy failed));
		CHECK_MESSAGE(first[testKey]->isEqual2D(*second[testKey]), TEST_MESSAGE(Map copy content wrong));
		auto third = std::move(second);
		CHECK_MESSAGE(third.size() == 1, TEST_MESSAGE(Map move assignment failed));
		CHECK_MESSAGE(second.empty(), TEST_MESSAGE(Map move assignment made a copy rather than moving));
		CHECK_MESSAGE(first[testKey]->isEqual2D(*third[testKey]), TEST_MESSAGE(Map move assignment content wrong));
		Map<utility::String, geometry::Polygon> fourth;
		fourth.insert(std::make_pair(String{testKey2}, other));
		CHECK_MESSAGE(fourth.size() == 1, TEST_MESSAGE(Map clone insertion failed));
		fourth.insert(*third.begin());
		CHECK_MESSAGE(fourth.size() == 2, TEST_MESSAGE(Map clone insertion failed));
		CHECK_MESSAGE((third.size() == 1 && (third.begin()->second)), TEST_MESSAGE(Map clone insertion performed move));
		CHECK_MESSAGE(third[testKey]->isEqual2D(*fourth[testKey2]), TEST_MESSAGE(Map clone assignment content wrong));
		Map<utility::String, geometry::Polygon> fifth;
		fifth.insert(third.extract(third.begin()));
		CHECK_MESSAGE(third.empty(), TEST_MESSAGE(Map extract/insert failed to move item));
		CHECK_MESSAGE((fifth.size() == 1 && (fifth.begin()->second)), TEST_MESSAGE(Map node handle insert failed));
	}

  		///Tests for HashMap
	TEST_CASE(TESTQ(testHashMap)) {
		const char* testKey = "Shape";
		const char* testKey2 = "Shape2";
		geometry::Polygon original{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		auto other = original;
		HashMap<utility::String, geometry::Polygon> first;
		first.emplace(testKey, std::move(original));
		CHECK_MESSAGE(first.size() == 1, TEST_MESSAGE(HashMap insert with moved rvalue failed));
		CHECK_MESSAGE(original.empty(), TEST_MESSAGE(HashMap insert with move made a copy rather than moving));
		auto second = first;
		CHECK_MESSAGE(second.size() == 1, TEST_MESSAGE(HashMap copy failed));
		CHECK_MESSAGE(first[testKey]->isEqual2D(*second[testKey]), TEST_MESSAGE(HashMap copy content wrong));
		auto third = std::move(second);
		CHECK_MESSAGE(third.size() == 1, TEST_MESSAGE(HashMap move assignment failed));
		CHECK_MESSAGE(second.empty(), TEST_MESSAGE(HashMap move assignment made a copy rather than moving));
		CHECK_MESSAGE(first[testKey]->isEqual2D(*third[testKey]), TEST_MESSAGE(HashMap move assignment content wrong));
		HashMap<utility::String, geometry::Polygon> fourth;
		fourth.insert(std::make_pair(String{testKey2}, other));
		CHECK_MESSAGE(fourth.size() == 1, TEST_MESSAGE(HashMap clone insertion failed));
		fourth.insert(*third.begin());
		CHECK_MESSAGE(fourth.size() == 2, TEST_MESSAGE(HashMap clone insertion failed));
		CHECK_MESSAGE((third.size() == 1 && (third.begin()->second)), TEST_MESSAGE(HashMap clone insertion performed move));
		CHECK_MESSAGE(third[testKey]->isEqual2D(*fourth[testKey2]), TEST_MESSAGE(HashMap clone assignment content wrong));
		HashMap<utility::String, geometry::Polygon> fifth;
		fifth.insert(third.extract(third.begin()));
		CHECK_MESSAGE(third.empty(), TEST_MESSAGE(HashMap extract/insert failed to move item));
		CHECK_MESSAGE((fifth.size() == 1 && (fifth.begin()->second)), TEST_MESSAGE(HashMap node handle insert failed));
	}

  	///Tests for MultiMap
	TEST_CASE(TESTQ(testMultiMap)) {
		const char* testKey = "Shape";
		const char* testKey2 = "Shape2";
		geometry::Polygon original{PolyPoint{1.0, 2.0}, PolyPoint{3.0, 2.0}, PolyPoint{1.0, 4.0}};
		auto other = original;
		MultiMap<utility::String, geometry::Polygon> first;
		first.emplace(testKey, std::move(original));
		CHECK_MESSAGE(first.size() == 1, TEST_MESSAGE(MultiMap insert with moved rvalue failed));
		CHECK_MESSAGE(original.empty(), TEST_MESSAGE(MultiMap insert with move made a copy rather than moving));
		auto second = first;
		CHECK_MESSAGE(second.size() == 1, TEST_MESSAGE(MultiMap copy failed));
		CHECK_MESSAGE(first.begin()->second->isEqual2D(*second.begin()->second), TEST_MESSAGE(MultiMap copy content wrong));
		auto third = std::move(second);
		CHECK_MESSAGE(third.size() == 1, TEST_MESSAGE(MultiMap move assignment failed));
		CHECK_MESSAGE(second.empty(), TEST_MESSAGE(MultiMap move assignment made a copy rather than moving));
		CHECK_MESSAGE(first.begin()->second->isEqual2D(*third.begin()->second), TEST_MESSAGE(MultiMap move assignment content wrong));
		MultiMap<utility::String, geometry::Polygon> fourth;
		fourth.insert(std::make_pair(String{testKey2}, other));
		CHECK_MESSAGE(fourth.size() == 1, TEST_MESSAGE(MultiMap clone insertion failed));
		fourth.insert(*third.begin());
		CHECK_MESSAGE(fourth.size() == 2, TEST_MESSAGE(MultiMap clone insertion failed));
		CHECK_MESSAGE((third.size() == 1 && (third.begin()->second)), TEST_MESSAGE(MultiMap clone insertion performed move));
		CHECK_MESSAGE(third.find(testKey)->second->isEqual2D(*fourth.find(testKey2)->second), TEST_MESSAGE(MultiMap clone assignment content wrong));
		MultiMap<utility::String, geometry::Polygon> fifth;
		fifth.insert(third.extract(third.begin()));
		CHECK_MESSAGE(third.empty(), TEST_MESSAGE(MultiMap extract/insert failed to move item));
		CHECK_MESSAGE((fifth.size() == 1 && (fifth.begin()->second)), TEST_MESSAGE(MultiMap node handle insert failed));
	}

TEST_SUITE_CLOSE
