#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/Generic/Base64Transport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/Time.h"

using namespace active;
using namespace active::serialise;
using namespace active::utility;

	///Tests for base64 encoding
TEST_SUITE(TESTQ(base64Test)) TEST_SUITE_OPEN
	
		///Tests for sending and receiving data via base64
	TEST_CASE(TESTQ(testBase64Transport)) {
			//Positive tests (good data)
		
			//Initialise sample test data
		constexpr Memory::size_type bufferLen = 0x1000;
		Memory dataOut, dataIn;
		dataOut.resize(bufferLen);
		srand(Time{}.microsecond());
		for (auto i = 0; i < bufferLen; ++i)
			dataOut[i] = rand() % 256;
			//Allocate a string to hold the outgoing base64
		String collector;
		Base64Transport transporter;
		for (auto i = 0; i < 5; ++i) {
				//Send the test data as base64 (into the collection string)
			CHECK_MESSAGE((transporter.send(dataOut, collector) && !collector.empty()), TEST_MESSAGE(base64 send failed));
				//Receive the test data from base64
			CHECK_MESSAGE(transporter.receive(dataIn, collector), TEST_MESSAGE(base64 receive failed));
				//Confirm that the incoming object matches the original outgoing object
			CHECK_MESSAGE(dataOut == dataIn, TEST_MESSAGE(Object transported via base64 does not match original));
				//Truncate the outgoing data by one char
			dataOut.resize(dataOut.size() - 1);
			dataIn.clear();
		}

			//Negative tests (bad data)
		
			//Invalid base64 numeral
		CHECK_MESSAGE(!transporter.receive(dataIn, String{"ABC~"}), TEST_MESSAGE(Bad base64 numerals accepted by Base64Transport));
			//Short data
		CHECK_MESSAGE(!transporter.receive(dataIn, String{"A"}), TEST_MESSAGE(Insufficient base64 data accepted by Base64Transport));
	}

TEST_SUITE_CLOSE
