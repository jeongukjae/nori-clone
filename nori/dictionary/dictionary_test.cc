#include "nori/dictionary/dictionary.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nori/protos/dictionary.pb.h"

using namespace nori::dictionary;

TEST(TestDictionary, load) {
  Dictionary dic;
  auto status = dic.load("./dictionary");
  ASSERT_TRUE(status.ok()) << status.message();
}
