#include "nori/lib/utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace nori::utils;

TEST(TestUtils, detectLastCharacterType) {
  ASSERT_EQ(internal::detectLastCharacterType("Hello World!"),
            internal::LastCharType::NNG);
  ASSERT_EQ(internal::detectLastCharacterType("Hello 안녀ㅇWorld!"),
            internal::LastCharType::NNG);
  ASSERT_EQ(internal::detectLastCharacterType("안녕"),
            internal::LastCharType::NNG_T);
  ASSERT_EQ(internal::detectLastCharacterType("안녀"),
            internal::LastCharType::NNG_F);
  ASSERT_EQ(internal::detectLastCharacterType("힣"),
            internal::LastCharType::NNG_T);
  ASSERT_EQ(internal::detectLastCharacterType("가"),
            internal::LastCharType::NNG_F);
  ASSERT_EQ(internal::detectLastCharacterType("12"),
            internal::LastCharType::NNG);
  ASSERT_EQ(internal::detectLastCharacterType("안ㄱ"),
            internal::LastCharType::NNG);
  ASSERT_EQ(internal::detectLastCharacterType("뭐"),
            internal::LastCharType::NNG_F);
}

TEST(TestUtils, lowercaseUTF8) {
  ASSERT_EQ(lowercaseUTF8("Hello World!"), "hello world!");
  ASSERT_EQ(lowercaseUTF8("Hello 안녀ㅇWorld!"), "hello 안녀ㅇworld!");
}

TEST(TestUtils, listDictionary) {
  std::vector<std::string> paths;

  // without slash
  internal::listDirectory("./testdata/listDirectory", paths,
                          [](absl::string_view path) {
                            return !(path == "./testdata/listDirectory/." ||
                                     path == "./testdata/listDirectory/..");
                          });
  ASSERT_THAT(paths, testing::ElementsAre("./testdata/listDirectory/1",
                                          "./testdata/listDirectory/2",
                                          "./testdata/listDirectory/3"));

  paths.clear();
  // with slash
  internal::listDirectory("./testdata/listDirectory/", paths,
                          [](absl::string_view path) {
                            return !(path == "./testdata/listDirectory/." ||
                                     path == "./testdata/listDirectory/..");
                          });
  ASSERT_THAT(paths, testing::ElementsAre("./testdata/listDirectory/1",
                                          "./testdata/listDirectory/2",
                                          "./testdata/listDirectory/3"));
}

TEST(TestUtils, joinPath) {
  ASSERT_THAT(internal::joinPath("testdata", "1"), "testdata/1");
  ASSERT_THAT(internal::joinPath("testdata/", "1"), "testdata/1");
}

TEST(TestUtils, parseCSVLine) {
  std::string inputString = "ALPHA,1793,3533,795,SL,*,*,*,*,*,*,*";
  std::vector<std::string> rows;
  internal::parseCSVLine(inputString, rows);

  ASSERT_THAT(rows, testing::ElementsAre("ALPHA", "1793", "3533", "795", "SL",
                                         "*", "*", "*", "*", "*", "*", "*"));

  inputString = "\"ALPHA\",1793,3533,795,SL,*,*,*,*,*,*,*";
  rows.clear();
  internal::parseCSVLine(inputString, rows);

  ASSERT_THAT(rows, testing::ElementsAre("ALPHA", "1793", "3533", "795", "SL",
                                         "*", "*", "*", "*", "*", "*", "*"));

  inputString = "\"ALPHA\",1793,3533,795,SL,*,*,*,*,*,*,*";
  rows.clear();
  rows = internal::parseCSVLine(inputString);

  ASSERT_THAT(rows, testing::ElementsAre("ALPHA", "1793", "3533", "795", "SL",
                                         "*", "*", "*", "*", "*", "*", "*"));
}

TEST(TestUtils, trimWhitespaces) {
  std::vector<std::tuple<std::string, std::string>> testCases = {
      {" a", "a"},
      {"\ta", "a"},
      {"\ta ", "a"},
      {"\t안녕 ", "안녕"},
  };

  for (auto testCase : testCases) {
    std::string inputText = std::get<0>(testCase);
    internal::trimWhitespaces(inputText);
    ASSERT_EQ(inputText, std::get<1>(testCase));
  }
}

TEST(TestUtils, simpleAtoi) {
  ASSERT_EQ(internal::simpleAtoi("12"), 12);
  ASSERT_EQ(internal::simpleAtoi("3456"), 3456);
}

TEST(TestUtils, simpleHexAtoi) {
  ASSERT_EQ(internal::simpleHexAtoi("0x12"), 18);
  ASSERT_EQ(internal::simpleHexAtoi("0x3456"), 13398);
}

TEST(TestUtils, resolvePOSType) {
  ASSERT_EQ(resolvePOSType("*"), nori::protos::POSType::MORPHEME);
  ASSERT_EQ(resolvePOSType("Inflect"), nori::protos::POSType::INFLECT);
  ASSERT_EQ(resolvePOSType("inflect"), nori::protos::POSType::INFLECT);
  ASSERT_EQ(resolvePOSType("Preanalysis"), nori::protos::POSType::PREANALYSIS);
  ASSERT_EQ(resolvePOSType("Compound"), nori::protos::POSType::COMPOUND);
}

TEST(TestUtils, resolvePOSTag) {
  ASSERT_EQ(resolvePOSTag("SSC"), nori::protos::POSTag::SSC);
  ASSERT_EQ(resolvePOSTag("SP"), nori::protos::POSTag::SP);
  ASSERT_EQ(resolvePOSTag("JX"), nori::protos::POSTag::J);
}

TEST(TestUtils, isDirectory) {
  ASSERT_TRUE(isDirectory("./testdata"));
  ASSERT_FALSE(isDirectory("./testdata12"));
  ASSERT_TRUE(isDirectory("./testdata/listDirectory"));
}
