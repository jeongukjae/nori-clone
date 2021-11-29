#include "nori/utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace nori::utils;

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

TEST(TestUtils, parseCSVLine) {
  std::string inputString = "ALPHA,1793,3533,795,SL,*,*,*,*,*,*,*";
  std::vector<std::string> rows;
  internal::parsCSVLine(inputString, rows);

  ASSERT_THAT(rows, testing::ElementsAre("ALPHA", "1793", "3533", "795", "SL",
                                         "*", "*", "*", "*", "*", "*", "*"));

  inputString = "\"ALPHA\",1793,3533,795,SL,*,*,*,*,*,*,*";
  rows.clear();
  internal::parsCSVLine(inputString, rows);

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

TEST(TestUtils, resolveType) {
  ASSERT_EQ(resolveType("*"), nori::POSType::MORPHEME);
  ASSERT_EQ(resolveType("Inflect"), nori::POSType::INFLECT);
  ASSERT_EQ(resolveType("Preanalysis"), nori::POSType::PREANALYSIS);
  ASSERT_EQ(resolveType("Compound"), nori::POSType::COMPOUND);
}
