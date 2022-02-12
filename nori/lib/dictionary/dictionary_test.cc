#include "nori/lib/dictionary/dictionary.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nori/lib/dictionary/builder.h"
#include "nori/lib/protos/dictionary.pb.h"

using namespace nori::dictionary;
using namespace nori::dictionary::builder;

TEST(TestBuilder, load) {
  DictionaryBuilder builder(true, "NFKC");
  auto status = builder.build("./testdata/dictionaryBuilder/");
  ASSERT_TRUE(status.ok()) << status.message();

  status = builder.save("dictionary.nori");
  ASSERT_TRUE(status.ok()) << status.message();

  Dictionary dic;
  status = dic.loadPrebuilt("dictionary.nori");
  ASSERT_TRUE(status.ok()) << status.message();
}

TEST(TestDictionary, loadPrebuilt) {
  Dictionary dic;
  auto status = dic.loadPrebuilt("./dictionary/latest-dictionary.nori");
  ASSERT_TRUE(status.ok()) << status.message();
  status = dic.loadUser("./dictionary/latest-userdict.txt");
  ASSERT_TRUE(status.ok()) << status.message();
}
