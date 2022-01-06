#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>
#include <google/protobuf/repeated_field.h>

#include <memory>
#include <queue>
#include <vector>

#include "nori/protos/dictionary.pb.h"

#define MINIMUM_COST_DEFAULT 1e+10

namespace nori {

namespace internal {

struct TrieNode {
  int uniqueNodeId;
  size_t cost;
  size_t lastPositionIndex;
  size_t length;

  nori::Morpheme* morpheme;
  std::shared_ptr<TrieNode> parent;

  TrieNode(int uniqueNodeId, size_t cost, size_t lastPositionIndex,
           size_t length, nori::Morpheme* morpheme,
           std::shared_ptr<TrieNode> parent = nullptr)
      : uniqueNodeId(uniqueNodeId),
        cost(cost),
        lastPositionIndex(lastPositionIndex),
        length(length),
        morpheme(morpheme),
        parent(parent) {}
};

inline int getSpacePenalty(
    google::protobuf::RepeatedField<google::protobuf::int32> tagList,
    int numSpaces) {
  if (numSpaces == 0) return 0;
  if (tagList.size() == 0) return 0;

  switch (nori::POSTag(tagList.at(0))) {
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

absl::Status NoriTokenizer::tokenize(const std::string& text, Lattice& lattice,
                                     GraphvizVisualizer* visualizer) const {
  if (visualizer != nullptr) {
    visualizer->reset();
  }

  size_t length = text.size();
  const char* begin = text.data();
  const char* current = begin;
  const char* end = text.data() + length;
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  std::shared_ptr<internal::TrieNode> nodesByPos;

  int nodeId = 0;
  auto cmp = [](SharedTrieNode left, SharedTrieNode right) {
    return left->lastPositionIndex < right->lastPositionIndex;
  };
  std::priority_queue<SharedTrieNode, std::vector<SharedTrieNode>,
                      decltype(cmp)>
      nodes(cmp);
  nodes.emplace(
      new internal::TrieNode(nodeId++, 0, 0, 0, nullptr));  // bos node;

  size_t minimumCost = MINIMUM_COST_DEFAULT;
  SharedTrieNode optimalPath;

  while (nodes.size() > 0) {
    SharedTrieNode nodeToSearch = nodes.top();
    nodes.pop();

    // skip whitespaces
    current = begin + nodeToSearch->lastPositionIndex;
    int numSpaces = 0;
    while (std::isspace(*current)) {
      current++;
      numSpaces++;
    }

    // TODO(jeongukjae): debug log
    // LOG(INFO) << "pos: " << nodeToSearch->lastPositionIndex
    //           << ", cost: " << nodeToSearch->cost
    //           << ", str: " << std::string(current, end) << ", previous: "
    //           << std::string(begin + (nodeToSearch->lastPositionIndex -
    //                                   nodeToSearch->length),
    //                          begin + nodeToSearch->lastPositionIndex);

    // Handling EOS node
    // end of parsing of this path
    if (current == end) {
      auto connectionCost =
          this->dictionary->getConnectionCost(nodeToSearch->morpheme, nullptr);
      auto eosNode = std::shared_ptr<internal::TrieNode>(new internal::TrieNode(
          nodeId++, nodeToSearch->cost + connectionCost,
          nodeToSearch->lastPositionIndex, 0, nullptr, nodeToSearch));

      if (eosNode->cost < minimumCost) {
        minimumCost = eosNode->cost;
        optimalPath = eosNode;
      }

      continue;
    }

    // TODO(jeongukjae): search user dictionary first

    // pre-built dictionary
    const int numNodes = dictionary->getTrie()->commonPrefixSearch(
        current, trieResults.data(), maxTrieResults,
        static_cast<int>(end - current));
    if (numNodes > maxTrieResults)
      return absl::InternalError("Cannot search trie");

    // handling unknown characters
    if (numNodes == 0) {
      auto category = dictionary->getCharClass(current);
      auto morpheme =
          dictionary->getUnkDictionary()->morphememap().at(category);

      auto wordCost = morpheme.wordcost();
      auto connectionCost = this->dictionary->getConnectionCost(
          nodeToSearch->morpheme, &morpheme);
      auto spaceCost = internal::getSpacePenalty(morpheme.postag(), numSpaces);

      int length =
          internal::groupingUnknownCharacters(current, category, dictionary) +
          numSpaces;

      auto newNode = new internal::TrieNode(
          nodeId++, nodeToSearch->cost + wordCost + connectionCost + spaceCost,
          nodeToSearch->lastPositionIndex + length, length, &morpheme,
          nodeToSearch);
      nodes.emplace(newNode);

      if (visualizer != nullptr) {
        visualizer->addNode(
            nodeToSearch->lastPositionIndex - nodeToSearch->length,
            nodeToSearch->uniqueNodeId, nodeToSearch->morpheme,
            nodeToSearch->lastPositionIndex + numSpaces, newNode->uniqueNodeId,
            newNode->morpheme,
            std::string(begin + (nodeToSearch->lastPositionIndex + numSpaces),
                        begin + newNode->lastPositionIndex),
            wordCost, connectionCost);
      }

      continue;
    }

    for (int k = 0; k < numNodes; ++k) {
      auto trieResult = trieResults[k];
      auto morphemeList =
          dictionary->getTokenDictionary()->morphemelistmap().at(
              trieResult.value);

      for (int j = 0; j < morphemeList.morphemes_size(); j++) {
        auto morpheme = morphemeList.morphemes(j);

        auto wordCost = morpheme.wordcost();
        auto connectionCost = this->dictionary->getConnectionCost(
            nodeToSearch->morpheme, &morpheme);
        auto spaceCost =
            internal::getSpacePenalty(morpheme.postag(), numSpaces);

        auto newNode = new internal::TrieNode(
            nodeId++,
            nodeToSearch->cost + wordCost + connectionCost + spaceCost,
            nodeToSearch->lastPositionIndex + trieResult.length + numSpaces,
            trieResult.length, &morpheme, nodeToSearch);
        nodes.emplace(newNode);

        if (visualizer != nullptr) {
          visualizer->addNode(
              nodeToSearch->lastPositionIndex - nodeToSearch->length,
              nodeToSearch->uniqueNodeId, nodeToSearch->morpheme,
              nodeToSearch->lastPositionIndex + numSpaces,
              newNode->uniqueNodeId, newNode->morpheme,
              std::string(begin + (nodeToSearch->lastPositionIndex + numSpaces),
                          begin + newNode->lastPositionIndex),
              wordCost, connectionCost);
        }
      }
    }
  }

  // TODO lattice output
  LOG(INFO) << minimumCost;
  SharedTrieNode currentNode = optimalPath;
  while (currentNode != NULL) {
    LOG(INFO) << std::string(
        begin + (currentNode->lastPositionIndex - currentNode->length),
        begin + currentNode->lastPositionIndex);
    currentNode = currentNode->parent;
  }

  if (visualizer != nullptr) {
    visualizer->finish();
  }

  return absl::OkStatus();
}

}  // namespace nori
