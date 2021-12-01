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

TEST(TestDictionary, getCharacterCategory) {
  Dictionary dic;
  auto status = dic.load("./dictionary");
  ASSERT_TRUE(status.ok()) << status.message();

  ASSERT_EQ(dic.getCharacterCategory(0x3B), nori::CharacterClass::SYMBOL);
  ASSERT_EQ(dic.getCharacterCategory(0x4E00),
            nori::CharacterClass::HANJANUMERIC);
}
