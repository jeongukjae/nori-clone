#include "nori/lib/tokenizer.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nori/lib/dictionary/dictionary.h"

nori::dictionary::Dictionary legacyDictionary, dictionary;

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
  std::vector<std::vector<std::string>> expectedTokens = {
      {"화학", "이외", "의", "것"}, {"화학", "이외", "의", "것"},
      {"화학", "이외", "의", "것"}, {"화학", "이외", "의", "것"},
      {"화학", "이외", "의", "것"},
  };
  std::vector<std::vector<std::vector<nori::protos::POSTag>>> expectedPOS = {
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNB},
      },
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNB},
      },
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNB},
      },
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNB},
      },
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNB},
      },
  };

  for (int i = 0; i < testCases.size(); i++) {
    nori::NoriTokenizer tokenizer(&dictionary);
    nori::Lattice lattice;
    auto status = lattice.setSentence(testCases[i], dictionary.getNormalizer());
    ASSERT_TRUE(status.ok());
    status = tokenizer.tokenize(lattice);
    ASSERT_TRUE(status.ok());

    for (int j = 0; j < expectedTokens[i].size(); j++) {
      // +1: BOS token
      ASSERT_EQ(lattice.getTokens()->at(j + 1).surface, expectedTokens[i][j]);
      ASSERT_EQ(lattice.getTokens()->at(j + 1).morpheme->pos_tags_size(),
                expectedPOS[i][j].size());

      for (int j = 0; j < expectedPOS[i][j].size(); j++) {
        ASSERT_EQ(lattice.getTokens()->at(j + 1).morpheme->pos_tags(j),
                  expectedPOS[i][j][j]);
      }
    }
  }
}

TEST(NoriTokenizer, testLegacyDictionary) {
  std::vector<std::string> testCases = {
      "가락지나물은 한국, 중국, 일본",
      "10.1 인치 모니터",
  };
  std::vector<std::vector<std::string>> expectedTokens = {
      {"가락지나물", "은", "한국", ",", "중국", ",", "일본"},
      {"10", ".", "1", "인치", "모니터"},
  };
  std::vector<std::vector<std::vector<nori::protos::POSTag>>> expectedPOS = {
      {
          {nori::protos::POSTag::NNG},
          {nori::protos::POSTag::J},
          {nori::protos::POSTag::NNP},
          {nori::protos::POSTag::SC},
          {nori::protos::POSTag::NNP},
          {nori::protos::POSTag::SC},
          {nori::protos::POSTag::NNP},
      },
      {
          {nori::protos::POSTag::SN},
          {nori::protos::POSTag::SY},
          {nori::protos::POSTag::SN},
          {nori::protos::POSTag::NNBC},
          {nori::protos::POSTag::NNG},
      },
  };

  for (int i = 0; i < testCases.size(); i++) {
    nori::NoriTokenizer tokenizer(&legacyDictionary);
    nori::Lattice lattice;
    auto status =
        lattice.setSentence(testCases[i], legacyDictionary.getNormalizer());
    ASSERT_TRUE(status.ok());
    status = tokenizer.tokenize(lattice);
    ASSERT_TRUE(status.ok());

    for (int j = 0; j < expectedTokens[i].size(); j++) {
      // +1: BOS token
      ASSERT_EQ(lattice.getTokens()->at(j + 1).surface, expectedTokens[i][j]);
      ASSERT_EQ(lattice.getTokens()->at(j + 1).morpheme->pos_tags_size(),
                expectedPOS[i][j].size());

      for (int j = 0; j < expectedPOS[i][j].size(); j++) {
        ASSERT_EQ(lattice.getTokens()->at(j + 1).morpheme->pos_tags(j),
                  expectedPOS[i][j][j]);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  auto status = dictionary.loadPrebuilt("./dictionary/latest-dictionary.nori");
  CHECK(status.ok()) << status.message();

  status = legacyDictionary.loadPrebuilt("./dictionary/legacy-dictionary.nori");
  CHECK(status.ok()) << status.message();

  return RUN_ALL_TESTS();
}
