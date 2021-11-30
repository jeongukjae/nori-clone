#include "nori/dictionary/builder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace nori::dictionary::builder;

TEST(TestInternal, convertMeCabCSVEntry) {
  std::vector<std::string> entry = {
      "은전한닢",    "0",   "0",   "0",
      "NNG+NR+NNG",  "*",   "T",   "은전한닢",
      "Preanalysis", "NNG", "NNG", "은전/NNG/*+한/NR/*+닢/NNG/*"};
  nori::Morpheme morpheme;

  internal::convertMeCabCSVEntry(entry, &morpheme).IgnoreError();

  ASSERT_EQ(morpheme.postype(), nori::POSType::PREANALYSIS);
  ASSERT_EQ(morpheme.expression_size(), 3);
  ASSERT_EQ(morpheme.expression(0).surface(), "은전");
  ASSERT_EQ(morpheme.expression(0).postag(), nori::POSTag::NNG);
  ASSERT_EQ(morpheme.expression(1).surface(), "한");
  ASSERT_EQ(morpheme.expression(1).postag(), nori::POSTag::NR);
  ASSERT_EQ(morpheme.expression(2).surface(), "닢");
  ASSERT_EQ(morpheme.expression(2).postag(), nori::POSTag::NNG);

  entry = {",", "1792", "3558", "788", "SC", "*", "*", "*", "*", "*", "*", "*"};
  nori::Morpheme morpheme2;

  internal::convertMeCabCSVEntry(entry, &morpheme2).IgnoreError();

  ASSERT_EQ(morpheme2.postype(), nori::POSType::MORPHEME);
  ASSERT_EQ(morpheme2.expression_size(), 0);
}

TEST(TestBuilder, TokenInfoDictionaryBuilder) {
  TokenInfoDictionaryBuilder builder(true, "NFKC");
  auto status = builder.parse("./testdata/tokenInfoDictionary/");
  ASSERT_TRUE(status.ok()) << status.message();
  status = builder.save(".");
  ASSERT_TRUE(status.ok()) << status.message();
}
