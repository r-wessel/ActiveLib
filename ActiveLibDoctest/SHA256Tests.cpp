#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/SHA256.h"
#include "Active/Utility/BufferIn.h"

using namespace active::math;
using namespace active::utility;

namespace {
	
		///Short test message
	const String shortMessage{"hello world"};
		///Chunk-size test message
	const String chunkMessage{"It was the best of times, it was the worst of times, it was the "};
		///Chunk-size + 1 test message
	const String chunkPlusMessage{"It was the best of times, it was the worst of times, it was the a"};
		///Longer test message
	const String longMessage{"It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to Heaven, we were all going direct the other way – in short, the period was so far like the present period, that some of its noisiest authorities insisted on its being received, for good or for evil, in the superlative degree of comparison only."};

	struct TestHash {
	public:
		double v1 = 1.234;
		int v2 = 1234;
		String v3{"4321"};
		
		String hash() const { return (SHA256() << v1 << v2 << v3).hexHash();}
	};
	
}


TEST_SUITE(TESTQ(SHA256Tests)) TEST_SUITE_OPEN

		///Tests for hashing using SHA256
	TEST_CASE(TESTQ(testSHA256)) {
			//Hash a short message into hex/base64
		auto testActiveLibHex = (SHA256() << shortMessage).hexHash();
		CHECK_MESSAGE(testActiveLibHex == "B94D27B9934D3E08A52E52D7DA7DABFAC484EFE37A5380EE9088F7ACE2EFCDE9", TEST_MESSAGE(SHA256 hex encoding failed));
		auto testActiveLibBase64 = (SHA256() << shortMessage).base64Hash();
		CHECK_MESSAGE(testActiveLibBase64 == "uU0nuZNNPgilLlLX2n2r+sSE7+N6U4DukIj3rOLvzek=", TEST_MESSAGE(SHA256 base64 encoding failed));
		  //Hash a message equal to the chunk size into hex/base64
		testActiveLibHex = (SHA256() << chunkMessage).hexHash();
		CHECK_MESSAGE(testActiveLibHex == "B32728CBC6923132F6A5D5A6BAF9C8D45D4C3C00F9C8C05573459955DBFB803A", TEST_MESSAGE(SHA256 chunk-size hex encoding failed));
		testActiveLibBase64 = (SHA256() << chunkMessage).base64Hash();
		CHECK_MESSAGE(testActiveLibBase64 == "sycoy8aSMTL2pdWmuvnI1F1MPAD5yMBVc0WZVdv7gDo=", TEST_MESSAGE(SHA256 chunk-size base64 encoding failed));
		  //Hash a message equal to the chunk size + 1 byte into hex/base64
		testActiveLibHex = (SHA256() << chunkPlusMessage).hexHash();
		CHECK_MESSAGE(testActiveLibHex == "9A8FFB307AB0AE81C35618E816E9B1F7BBE167D9BE80AA4264121394E1773102", TEST_MESSAGE(SHA256 chunk-size + 1 hex encoding failed));
		testActiveLibBase64 = (SHA256() << chunkPlusMessage).base64Hash();
		CHECK_MESSAGE(testActiveLibBase64 == "mo/7MHqwroHDVhjoFumx97vhZ9m+gKpCZBITlOF3MQI=", TEST_MESSAGE(SHA256 chunk-size + 1 base64 encoding failed));
		  //Hash a longer message into hex/base64
		testActiveLibHex = (SHA256() << longMessage).hexHash();
		CHECK_MESSAGE(testActiveLibHex == "267A0C5692258B522C3E416D0BE85C3D8BF181C72DDD4D1548330E42D806801D", TEST_MESSAGE(SHA256 larger hex encoding failed));
		testActiveLibBase64 = (SHA256() << longMessage).base64Hash();
		CHECK_MESSAGE(testActiveLibBase64 == "JnoMVpIli1IsPkFtC+hcPYvxgcct3U0VSDMOQtgGgB0=", TEST_MESSAGE(SHA256 larger base64 encoding failed));
		auto test = TestHash{}.hash();
		CHECK_MESSAGE(test == "0CB43D2D948242706CFC50A337861141BE5A4FD9ED2EAEB3DB0CBD9A771260C8", TEST_MESSAGE(Object hashing failed));
  }

TEST_SUITE_CLOSE

/*
 It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to Heaven, we were all going direct the other way – in short, the period was so far like the present period, that some of its noisiest authorities insisted on its being received, for good or for evil, in the superlative degree of comparison only.
 */
