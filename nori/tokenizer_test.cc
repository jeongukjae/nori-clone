#include "nori/tokenizer.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nori/dictionary/dictionary.h"

using namespace nori;

class TokenizerFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    auto status = dictionary.load("./dictionary");
    CHECK(status.ok()) << status;
  }

  void TearDown() override {}

  nori::dictionary::Dictionary dictionary;
};

TEST_F(TokenizerFixture, loadTokenizer) {
  NoriTokenizer tokenizer(&dictionary);
}

// TODO(jeongukjae): rewrite tokenizer tests in lucene
