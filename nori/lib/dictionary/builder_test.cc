#include "nori/lib/dictionary/builder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace nori::dictionary::builder;

TEST(TestInternal, convertMeCabCSVEntry) {
  std::vector<std::string> entry = {
      "은전한닢",    "0",   "0",   "0",
      "NNG+NR+NNG",  "*",   "T",   "은전한닢",
      "Preanalysis", "NNG", "NNG", "은전/NNG/*+한/NR/*+닢/NNG/*"};
  nori::protos::Morpheme morpheme;

  internal::convertMeCabCSVEntry(entry, &morpheme).IgnoreError();

  ASSERT_EQ(morpheme.pos_type(), nori::protos::POSType::PREANALYSIS);
  ASSERT_EQ(morpheme.expression_size(), 3);
  ASSERT_EQ(morpheme.expression(0).surface(), "은전");
  ASSERT_EQ(morpheme.expression(0).pos_tag(), nori::protos::POSTag::NNG);
  ASSERT_EQ(morpheme.expression(1).surface(), "한");
  ASSERT_EQ(morpheme.expression(1).pos_tag(), nori::protos::POSTag::NR);
  ASSERT_EQ(morpheme.expression(2).surface(), "닢");
  ASSERT_EQ(morpheme.expression(2).pos_tag(), nori::protos::POSTag::NNG);

  entry = {",", "1792", "3558", "788", "SC", "*", "*", "*", "*", "*", "*", "*"};
  nori::protos::Morpheme morpheme2;

  internal::convertMeCabCSVEntry(entry, &morpheme2).IgnoreError();

  ASSERT_EQ(morpheme2.pos_type(), nori::protos::POSType::MORPHEME);
  ASSERT_EQ(morpheme2.expression_size(), 0);
}

TEST(TestBuilder, DictionaryBuilder) {
  DictionaryBuilder builder(true, "NFKC");
  auto status = builder.build("./testdata/dictionaryBuilder/");
  ASSERT_TRUE(status.ok()) << status.message();

  status = builder.save("dictionary.nori");
  ASSERT_TRUE(status.ok()) << status.message();
}
