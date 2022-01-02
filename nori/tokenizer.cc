#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>

#include <memory>
#include <queue>
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

inline int groupingUnknownCharacters(
    const char* current, nori::CharacterClass category,
    const nori::dictionary::Dictionary* dictionary) {
  int maxLength = 1;
  for (; dictionary->getCharClass(current + maxLength) == category; maxLength++)
    ;

  return maxLength;
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
    bool hasSpace = false;
    while (std::isspace(*current)) {
      current++;
      hasSpace = true;
    }

    if (current == end) {
      // EOS
      minimumCost = std::min(minimumCost, nodeToSearch->cost);
      continue;
    }

    LOG(INFO) << "pos: " << nodeToSearch->lastPositionIndex
              << ", cost: " << nodeToSearch->cost
              << ", str: " << std::string(current, end) << ", previous: "
              << std::string(begin + (nodeToSearch->lastPositionIndex -
                                      nodeToSearch->length),
                             begin + nodeToSearch->lastPositionIndex);

    // TODO user dictionary
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

      auto wordCost = morpheme.wordcost();
      auto connectionCost = this->dictionary->getConnectionCost(
          nodeToSearch->morpheme, &morpheme);
      auto spaceCost =
          hasSpace
              ? internal::getSpacePenalty(nori::POSTag(morpheme.postag().at(0)))
              : 0;

      int length =
          internal::groupingUnknownCharacters(current, category, dictionary);

      // TODO(jeongukjae): connection cost
      nodes.emplace(
          new internal::TrieNode(nodeToSearch->cost + wordCost + connectionCost,
                                 nodeToSearch->lastPositionIndex + length,
                                 length, &morpheme, nodeToSearch));
    }

    for (int k = 0; k < numNodes; ++k) {
      // TODO(jeongukjae): connection cost
      auto morphemelist =
          dictionary->getTokenDictionary()->morphemelistmap().at(
              trieResults[k].value);
      for (int j = 0; j < morphemelist.morphemes_size(); j++) {
        auto morpheme = morphemelist.morphemes(j);

        auto wordCost = morpheme.wordcost();
        auto connectionCost = this->dictionary->getConnectionCost(
            nodeToSearch->morpheme, &morpheme);
        auto spaceCost = hasSpace ? internal::getSpacePenalty(
                                        nori::POSTag(morpheme.postag().at(0)))
                                  : 0;

        nodes.emplace(new internal::TrieNode(
            nodeToSearch->cost + wordCost + connectionCost + spaceCost,
            nodeToSearch->lastPositionIndex + trieResults[k].length,
            trieResults[k].length, &morpheme, nodeToSearch));
      }
    }
  }

  LOG(INFO) << minimumCost;

  return absl::OkStatus();
}

}  // namespace nori
