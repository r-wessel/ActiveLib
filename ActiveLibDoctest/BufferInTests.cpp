#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/File/Directory.h"
#include "Active/File/File.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"

#include "Active/Serialise/Generic/HexTransport.h"

using namespace active::file;
using namespace active::math;
using namespace active::utility;

namespace {
	String testBufferPath{"BufferTesting"};
	String testBufferName{"BufferExample.txt"};
	String sampleBufferData{u8"<tag>ਖdਖdਖdਖda</tag><tag>bਖdefghabਖdefgh</tag>"};
} // namespace

TEST_SUITE(TESTQ(BufferInTests)) TEST_SUITE_OPEN

		///Tests for buffering text i/o
	TEST_CASE(TESTQ(testBufferInText)) {
		Directory testDirectory;
		File testFile;
		try {
			testDirectory = Directory{Directory::temporary(), testBufferPath, true};
			CHECK_MESSAGE(testDirectory.exists(), TEST_MESSAGE(Created directory does not exist));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Create directory from relative path failed));
		}
		try {
			testFile = File{testDirectory, testBufferName, File::readWrite, true};
			CHECK_MESSAGE(testFile.exists(), TEST_MESSAGE(Created file does not exist));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Create file in specified directory failed));
		}
		try {
			testFile.open();
			testFile.write(sampleBufferData);
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File write failed));
		}
		try {
			testFile.flush();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File flush failed));
		}
		try {
			CHECK_MESSAGE(testFile.size() == sampleBufferData.dataSize(), TEST_MESSAGE(File write contents size incorrect));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File size failed));
		}
		try {
			testFile.setPosition(0);
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Set file position failed));
		}
			//The following test finds the tags and data from sample XML
		std::vector<String> data;
		BufferIn bufferIn{testFile};
		while (bufferIn) {
			if (!bufferIn.find('<', nullptr, true))
				break;
			String openTag;
			if (!bufferIn.find('>', &openTag, true))
				break;
			CHECK_MESSAGE(openTag == "tag", TEST_MESSAGE(BufferIn.find failed to read opening tag correctly));
			String content;
			if (!bufferIn.find('<', &content, true))
				break;
			data.push_back(content);
			String closeTag;
			if (!bufferIn.find('>', &closeTag, true))
				break;
			CHECK_MESSAGE(closeTag == "/tag", TEST_MESSAGE(BufferIn.find failed to read closing tag correctly));
		}
		CHECK_MESSAGE(data.size() == 2, TEST_MESSAGE(BufferIn failed to read all content));
		if (data.size() >= 2) {
			CHECK_MESSAGE(data[0] == String{u8"ਖdਖdਖdਖda"}, TEST_MESSAGE(BufferIn failed to read first data item));
			CHECK_MESSAGE(data[1] == String{u8"bਖdefghabਖdefgh"}, TEST_MESSAGE(BufferIn failed to read second data item));
		}
		try {
			testFile.close();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File close failed));
		}
		try {
			testFile.remove();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File remove failed in buffer test));
		}
		try {
			testDirectory.remove();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Directory remove failed in buffer test));
		}
	}

		/// Tests for buffering numeric i/o
	TEST_CASE(TESTQ(testBufferIO)) {
		String startWord{"Something"};
		double startVal = 654321.12345678;
		int16_t start16 = -7654;
		int32_t start32 = -1073741824;
		uint32_t startu32 = 2147483648;
		int64_t start64 = -9007199254740992;
		uint64_t startu64 = 9223372036854775808u;
		String buffer;
		{
			BufferOut bufferOut{buffer};
			bufferOut << startWord << '\t' << startVal << '\t' << start16 << '\t' << start32 << '\t' << startu32 << '\t' << start64;
#ifndef WINDOWS
			bufferOut << '\t' << startu64;
#endif
		}
		String endWord;
		double endVal = 0.0;
		int16_t end16 = 0;
		int32_t end32 = 0;
		uint32_t endu32 = 0;
		int64_t end64 = 0;
		uint64_t endu64 = 0;
		{
			BufferIn bufferIn{buffer};
			bufferIn >> endWord >> endVal >> end16 >> end32 >> endu32 >> end64;
#ifndef WINDOWS
			bufferIn >> endu64;
#endif
		}
		CHECK_MESSAGE(startWord == endWord, TEST_MESSAGE(String (word) buffered i/o failed));
		CHECK_MESSAGE(isEqual(startVal, endVal), TEST_MESSAGE(Double buffered i/o failed));
		CHECK_MESSAGE(start16 == end16, TEST_MESSAGE(int16_t buffered i/o failed));
		CHECK_MESSAGE(start32 == end32, TEST_MESSAGE(int32_t buffered i/o failed));
		CHECK_MESSAGE(startu32 == endu32, TEST_MESSAGE(uint32_t buffered i/o failed));
		CHECK_MESSAGE(start64 == end64, TEST_MESSAGE(int64_t buffered i/o failed));
#ifndef WINDOWS
		CHECK_MESSAGE(startu64 == endu64, TEST_MESSAGE(uint64_t buffered i/o failed));
#endif
	}

TEST_SUITE_CLOSE
