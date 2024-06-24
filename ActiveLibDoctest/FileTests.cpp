#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/File/Directory.h"
#include "Active/File/File.h"

using namespace active;
using namespace active::file;
using namespace active::utility;

namespace {
	String testDirectoryPath{"Testing"};
	String testFileName{"Example.txt"};
	String sampleData{u8"\u0A16d\u0A16d\u0A16d\u0A16dab\u0A16defghab\u0A16defgh"};
} // namespace

TEST_SUITE(TESTQ(FileTests)) TEST_SUITE_OPEN

		//Test general file/directory creation and writing tests
	TEST_CASE(TESTQ(testFileContent)) {
		Path testingPath{testDirectoryPath, true};
		Directory testDirectory;
		File testFile;
		try {
			testDirectory = Directory{testingPath, true};
			CHECK_MESSAGE(testDirectory.exists(), TEST_MESSAGE(Created directory does not exist));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Create directory from relative path failed));
		}
		try {
			testFile = File{testDirectory, testFileName, File::readWrite, true};
			CHECK_MESSAGE(testFile.exists(), TEST_MESSAGE(Created file does not exist));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Create file in specified directory failed));
		}
		try {
			testFile.open();
			testFile.write(sampleData);
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File write failed));
		}
		try {
			testFile.flush();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File flush failed));
		}
		try {
			CHECK_MESSAGE(testFile.size() == sampleData.dataSize(), TEST_MESSAGE(File write contents size incorrect));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File size failed));
		}
		try {
			testFile.close();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File close failed));
		}
		try {
			testFile.open();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File reopen failed));
		}
		try {
			testFile.setPosition(0);
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Set file position failed));
		}
		String textIn;
		try {
			auto readSize = testFile.size();
			CHECK_MESSAGE(readSize == sampleData.dataSize(), TEST_MESSAGE(File size reported incorrectly));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File size failed));
		}
		try {
			auto readSize = testFile.read(textIn);
			CHECK_MESSAGE(readSize == sampleData.dataSize(), TEST_MESSAGE(Data size read from file incorrect));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File read failed));
		}
		CHECK_MESSAGE(textIn == sampleData, TEST_MESSAGE(Data read from file incorrect));
		try {
			testFile.remove();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(File remove failed));
		}
		try {
			CHECK_MESSAGE(!testFile.exists(), TEST_MESSAGE(File exists after removal));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Checking file exists failed));
		}
		try {
			testDirectory.remove();
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Directory remove failed));
		}
		try {
			CHECK_MESSAGE(!testDirectory.exists(), TEST_MESSAGE(Directory exists after removal));
		} catch(std::system_error& error) {
			FAIL_CHECK(TEST_MESSAGE(Checking directory exists failed));
		}
	}

TEST_SUITE_CLOSE
