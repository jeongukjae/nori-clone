#include "nori/lib/utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace nori::utils;

TEST(TestUtils, hasJongsungAtLast) {
  ASSERT_EQ(internal::hasJongsungAtLast("Hello World!"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("Hello 안녀ㅇWorld!"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("안녕"), true);
  ASSERT_EQ(internal::hasJongsungAtLast("안녀"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("힣"), true);
  ASSERT_EQ(internal::hasJongsungAtLast("가"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("12"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("안ㄱ"), false);
  ASSERT_EQ(internal::hasJongsungAtLast("뭐"), false);
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
  ASSERT_EQ(resolvePOSType("*"), nori::POSType::MORPHEME);
  ASSERT_EQ(resolvePOSType("Inflect"), nori::POSType::INFLECT);
  ASSERT_EQ(resolvePOSType("inflect"), nori::POSType::INFLECT);
  ASSERT_EQ(resolvePOSType("Preanalysis"), nori::POSType::PREANALYSIS);
  ASSERT_EQ(resolvePOSType("Compound"), nori::POSType::COMPOUND);
}

TEST(TestUtils, resolvePOSTag) {
  ASSERT_EQ(resolvePOSTag("SSC"), nori::POSTag::SSC);
  ASSERT_EQ(resolvePOSTag("SP"), nori::POSTag::SP);
  ASSERT_EQ(resolvePOSTag("JX"), nori::POSTag::J);
}

TEST(TestUtils, isDirectory) {
  ASSERT_TRUE(isDirectory("./testdata"));
  ASSERT_FALSE(isDirectory("./testdata12"));
  ASSERT_TRUE(isDirectory("./testdata/listDirectory"));
}
