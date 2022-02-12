#include "nori/c/c_api.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(CApiTest, tokenize) {
  Dictionary* dictionary;
  Tokenizer* tokenizer;

  // load tokenizer
  int status = initializeTokenizer("./dictionary/latest-dictionary.nori",
                                   "./dictionary/latest-userdict.txt",
                                   &dictionary, &tokenizer);
  ASSERT_EQ(status, 0);

  Lattice* lattice;
  const char* input = "화학 이외의 것";
  status = tokenize(tokenizer, input, &lattice);
  ASSERT_EQ(status, 0);

  // BOS/화학/이외/의/것/EOS
  ASSERT_EQ(lattice->tokenLength, 6);

  std::vector<std::string> expectedTerms = {"화학", "이외", "의", "것"};

  for (int i = 0; i < expectedTerms.size(); i++) {
    int start = lattice->tokens[i + 1].offset;
    int end = lattice->tokens[i + 1].offset + lattice->tokens[i + 1].length;

    ASSERT_EQ(std::string(input + start, input + end), expectedTerms[i]);
  }

  freeLattice(lattice);
  freeTokenizer(dictionary, tokenizer);
}
