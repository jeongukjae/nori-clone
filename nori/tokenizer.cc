#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>

#include <map>
#include <queue>
#include <vector>

#include "nori/protos/dictionary.pb.h"

typedef Darts::DoubleArray::result_pair_type DartsResults;

namespace nori {

namespace internal {

struct PathValue {
  size_t cost = 0;
  size_t lastPositionIndex = 0;
  size_t length = 0;

  PathValue(size_t cost, size_t lastPositionIndex, size_t length)
      : cost(cost), lastPositionIndex(lastPositionIndex), length(length) {}
};

inline int getSpacePenalty(nori::POSTag tag) {
  switch (tag) {
    case nori::POSTag::E:
    case nori::POSTag::J:
    case nori::POSTag::VCP:
    case nori::POSTag::XSA:
    case nori::POSTag::XSN:
    case nori::POSTag::XSV:
      return 3000;
    default:
      return 0;
  }
}

}  // namespace internal

// NoriTokenizer class

absl::Status NoriTokenizer::tokenize(const std::string& text,
                                     Lattice* lattice) const {
  size_t length = text.size();
  const char* begin = text.data();
  const char* current = begin;
  const char* end = text.data() + length;
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  std::map<int, std::vector<internal::PathValue>> bestPaths;
  // add bos
  bestPaths[0].push_back(internal::PathValue(0, 0, 0));

  // while (current < end) {
  const int numNodes = dictionary->getTrie()->commonPrefixSearch(
      current, trieResults.data(), maxTrieResults,
      static_cast<int>(current - begin));

  if (numNodes > maxTrieResults)
    return absl::InternalError("Cannot search trie");
  LOG(INFO) << dictionary->getCost(0, 1794);
  LOG(INFO) << dictionary->getCost(3555, 1794);

  if (numNodes == 0) {
    auto category = dictionary->getCharClass(current);
    LOG(INFO) << category << ", "
              << dictionary->getUnkDictionary()
                     ->morphememap()
                     .at(category)
                     .wordcost();
  }

  for (int k = 0; k < numNodes; ++k) {
    LOG(INFO) << std::string(current, current + trieResults[k].length);
    auto morpheme = dictionary->getTokenDictionary()->morphemelistmap().at(
        trieResults[k].value);
    for (int j = 0; j < morpheme.morphemes_size(); j++) {
      // LOG(INFO) << morpheme.morphemes(j).leftid();
      LOG(INFO) << morpheme.morphemes(j).wordcost();
      // LOG(INFO) << morpheme.morphemes(j).rightid();
    }
  }
  // }

  return absl::OkStatus();
}

}  // namespace nori
