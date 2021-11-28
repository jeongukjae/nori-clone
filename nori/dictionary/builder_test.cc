#include "nori/dictionary/builder.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace nori::dictionary::builder;

TEST(TestInteral, listDictionary) {
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

TEST(TestUnknownDictionary, TestPutCharacterCategory) {}
