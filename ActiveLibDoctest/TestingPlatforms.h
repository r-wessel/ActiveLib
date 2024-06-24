/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

/**
	Macros that adapt test code to relevant platforms
	
	Currently targeting:
		- doctest
		- Visual Studio
		- Xcode

	Xcode and Visual Studio need to define an identifying macro (switch if a built-in replacement can be identified)
*/

#ifndef ACTIVE_TESTING_PLATFORMS
#define ACTIVE_TESTING_PLATFORMS

	//Xcode testing macros
#ifdef XCODE_TESTING

	#import <XCTest/XCTest.h>

	#define TEST_SUITE_OPEN

	#define TEST_SUITE(suiteName) \
	@interface suiteName : XCTestCase\
	@end\
	@implementation suiteName

	#define TEST_CASE(caseName) - (void)caseName

	#define TEST_SUITE_CLOSE @end

	#define CHECK_MESSAGE XCTAssert
	#define FAIL_CHECK XCTFail

	#define TESTQ(expr) expr

	#define TEST_MESSAGE(expr) #expr

	//Visual Studio testing macros
#elif VSCODE_TESTING

	#include "CppUnitTest.h"

	using namespace Microsoft::VisualStudio::CppUnitTestFramework;

	#define TEST_SUITE_OPEN {

	#define TEST_SUITE TEST_CLASS

	#define TEST_CASE TEST_METHOD

	#define TEST_SUITE_CLOSE };

	#define CHECK_MESSAGE Assert::IsTrue
	#define FAIL_CHECK Assert::Fail

	#define TESTQ(expr) expr

	#define TEST_MESSAGE(expr) L#expr

	//doctest macros
#else

	#include "doctest/doctest.h"

	#define TEST_SUITE_OPEN {

	#define TEST_SUITE_CLOSE }

	#define TESTQ(expr) #expr

	#define TEST_MESSAGE(expr) #expr

#endif

#endif	//ACTIVE_TESTING_PLATFORMS
