#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/SHA256.h"
#include "Active/Utility/BufferIn.h"

using namespace active::math;
using namespace active::utility;

TEST_SUITE(TESTQ(SHA256Tests)) TEST_SUITE_OPEN

	  ///Tests for hashing using SHA256
  TEST_CASE(TESTQ(testSHA256)) {
	  auto testActiveLibHex = (SHA256() << String{"hello world"}).hexHash();
	  CHECK_MESSAGE(testActiveLibHex == "B94D27B9934D3E08A52E52D7DA7DABFAC484EFE37A5380EE9088F7ACE2EFCDE9", TEST_MESSAGE(SHA256 hex encoding failed));
	  auto testActiveLibBase64 = (SHA256() << String{"hello world"}).base64Hash();
	  CHECK_MESSAGE(testActiveLibBase64 == "uU0nuZNNPgilLlLX2n2r+sSE7+N6U4DukIj3rOLvzek=", TEST_MESSAGE(SHA256 base64 encoding failed));
	  
	  testActiveLibHex = (SHA256() << String{"It was the best of times, it was the worst of times, it was the "}).hexHash();
	  CHECK_MESSAGE(testActiveLibHex == "B32728CBC6923132F6A5D5A6BAF9C8D45D4C3C00F9C8C05573459955DBFB803A", TEST_MESSAGE(SHA256 larger hex encoding failed));
	 
  }

TEST_SUITE_CLOSE

/*
 It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to Heaven, we were all going direct the other way – in short, the period was so far like the present period, that some of its noisiest authorities insisted on its being received, for good or for evil, in the superlative degree of comparison only.
 */
