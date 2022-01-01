#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>

#include <queue>
#include <unordered_map>
#include <vector>

#include "nori/protos/dictionary.pb.h"

namespace nori {

namespace internal {

struct TrieNode {
  size_t cost;
  size_t lastPositionIndex;
  size_t length;

  nori::Morpheme* morpheme;
  std::shared_ptr<TrieNode> parent;

  TrieNode(size_t cost, size_t lastPositionIndex, size_t length,
           nori::Morpheme* morpheme, std::shared_ptr<TrieNode> parent = nullptr)
      : cost(cost),
        lastPositionIndex(lastPositionIndex),
        length(length),
        morpheme(morpheme),
        parent(parent) {}
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
typedef Darts::DoubleArray::result_pair_type DartsResults;
typedef std::shared_ptr<internal::TrieNode> SharedTrieNode;

absl::Status NoriTokenizer::tokenize(const std::string& text,
                                     Lattice* lattice) const {
  size_t length = text.size();
  const char* begin = text.data();
  const char* current = begin;
  const char* end = text.data() + length;
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  std::shared_ptr<internal::TrieNode> nodesByPos;

  auto cmp = [](SharedTrieNode left, SharedTrieNode right) {
    return (left->lastPositionIndex > right->lastPositionIndex) ||
           (left->cost < right->cost);
  };
  std::priority_queue<SharedTrieNode, std::vector<SharedTrieNode>,
                      decltype(cmp)>
      nodes(cmp);
  nodes.emplace(new internal::TrieNode(0, 0, 0, nullptr));  // bos node;

  size_t minimumCost = 1e+10;
  while (nodes.size() > 0) {
    SharedTrieNode nodeToSearch = nodes.top();
    nodes.pop();

    current = begin + nodeToSearch->lastPositionIndex;
    if (current == end) {
      // EOS
      minimumCost = std::min(minimumCost, nodeToSearch->cost);
      continue;
    }

    LOG(INFO) << "pos: " << nodeToSearch->lastPositionIndex
              << ", cost: " << nodeToSearch->cost
              << ", str: " << std::string(current, end);

    const int numNodes = dictionary->getTrie()->commonPrefixSearch(
        current, trieResults.data(), maxTrieResults,
        static_cast<int>(end - current));

    if (numNodes > maxTrieResults)
      return absl::InternalError("Cannot search trie");

    if (numNodes == 0) {
      // TODO(jeongukjae): space penalty
      auto category = dictionary->getCharClass(current);
      auto morpheme =
          dictionary->getUnkDictionary()->morphememap().at(category);

      // TODO(jeongukjae): connection cost
      nodes.emplace(new internal::TrieNode(
          nodeToSearch->cost + morpheme.wordcost(),
          nodeToSearch->lastPositionIndex + 1, 1, &morpheme, nodeToSearch));
    }

    for (int k = 0; k < numNodes; ++k) {
      // TODO(jeongukjae): connection cost
      auto morphemelist =
          dictionary->getTokenDictionary()->morphemelistmap().at(
              trieResults[k].value);
      for (int j = 0; j < morphemelist.morphemes_size(); j++) {
        auto morpheme = morphemelist.morphemes(j);

        nodes.emplace(new internal::TrieNode(
            nodeToSearch->cost + morpheme.wordcost(),
            nodeToSearch->lastPositionIndex + trieResults[k].length,
            trieResults[k].length, &morpheme, nodeToSearch));
      }
    }
  }

  LOG(INFO) << minimumCost;

  return absl::OkStatus();
}

}  // namespace nori
