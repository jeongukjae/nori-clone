#include "nori/lib/tokenizer.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nori/lib/dictionary/dictionary.h"

nori::dictionary::Dictionary dictionary;

TEST(NoriTokenizer, loadTokenizer) {
  nori::NoriTokenizer tokenizer(&dictionary);
}

TEST(NoriTokenizer, testDefaultSentence) {
  std::vector<std::string> testCases = {
      "화학 이외의 것",
      "화학               이외의 것",
      "화학 이외의              것",
      "화학             이외의              것",
      "화학 이외의 것 ",
  };
  std::vector<std::string> expectedTokens = {"화학", "이외", "의", "것"};
  std::vector<std::vector<nori::POSTag>> expectedPOS = {
      {nori::POSTag::NNG},
      {nori::POSTag::NNG},
      {nori::POSTag::J},
      {nori::POSTag::NNB},
  };

  for (const auto& tt : testCases) {
    nori::NoriTokenizer tokenizer(&dictionary);
    nori::Lattice lattice;
    auto status = lattice.setSentence(tt);
    ASSERT_TRUE(status.ok());
    status = tokenizer.tokenize(lattice);
    ASSERT_TRUE(status.ok());

    for (int i = 0; i < expectedTokens.size(); i++) {
      // +1: BOS token
      ASSERT_EQ(lattice.getTokens()->at(i + 1).surface, expectedTokens[i]);
      ASSERT_EQ(lattice.getTokens()->at(i + 1).morpheme->postag_size(),
                expectedPOS[i].size());
      for (int j = 0; j < expectedPOS[i].size(); j++) {
        ASSERT_EQ(lattice.getTokens()->at(i + 1).morpheme->postag(j),
                  expectedPOS[i][j]);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  auto status = dictionary.loadPrebuilt("./dictionary");
  CHECK(status.ok()) << status.message();

  return RUN_ALL_TESTS();
}
