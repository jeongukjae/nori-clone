#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>
#include <google/protobuf/repeated_field.h>

#include <memory>
#include <queue>
#include <vector>

#include "icu4c/source/common/unicode/utf.h"
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
  int offset = 0;
  U8_FWD_1_UNSAFE(current, offset);

  for (; dictionary->getCharClass(current + offset) == category;) {
    U8_FWD_1_UNSAFE(current, offset);
  }

  return offset;
}

inline std::shared_ptr<TrieNode> selectParent(
    const std::vector<std::shared_ptr<internal::TrieNode>> candidates,
    const nori::Morpheme* morpheme,
    const nori::dictionary::Dictionary* dictionary) {
  if (candidates.size() == 0) return nullptr;

  std::shared_ptr<TrieNode> result = candidates[0];
  auto minimumCost =
      result->cost + dictionary->getConnectionCost(result->morpheme, morpheme);

  for (int i = 1; i < candidates.size(); i++) {
    auto candidate = candidates[i];

    auto cost = candidate->cost +
                dictionary->getConnectionCost(candidate->morpheme, morpheme);
    if (cost < minimumCost) {
      minimumCost = cost;
      result = candidate;
    }
  }
  return result;
}

}  // namespace internal

// NoriTokenizer class
typedef Darts::DoubleArray::result_pair_type DartsResults;

absl::Status NoriTokenizer::tokenize(const std::string& text, Lattice& lattice,
                                     GraphvizVisualizer* visualizer) const {
  if (visualizer != nullptr) {
    visualizer->reset();
  }

  size_t inputLength = text.size();
  const char* begin = text.data();
  const char* current = begin;
  const char* end = text.data() + inputLength;
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  int nodeId = 0;
  std::unordered_map<size_t, std::vector<std::shared_ptr<internal::TrieNode>>>
      nodesByPos;

  // bos node;
  nodesByPos[0].emplace_back(
      new internal::TrieNode(nodeId++, 0, 0, 0, nullptr));

  size_t minimumCost = MINIMUM_COST_DEFAULT;
  std::shared_ptr<internal::TrieNode> optimalPath;

  size_t offset = 0;
  while ((current = begin + offset) <= end) {
    // skip whitespaces
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
      auto parent =
          internal::selectParent(nodesByPos[offset], nullptr, this->dictionary);
      auto connectionCost =
          this->dictionary->getConnectionCost(parent->morpheme, nullptr);
      auto eosNode = std::shared_ptr<internal::TrieNode>(new internal::TrieNode(
          nodeId++, parent->cost + connectionCost, parent->lastPositionIndex, 0,
          nullptr, parent));

      if (visualizer != nullptr) {
        visualizer->addEos(parent->lastPositionIndex - parent->length,
                           parent->uniqueNodeId, parent->morpheme);
      }

      if (eosNode->cost < minimumCost) {
        minimumCost = eosNode->cost;
        optimalPath = eosNode;
      }

      break;
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
      auto spaceCost = internal::getSpacePenalty(morpheme.postag(), numSpaces);

      auto parent = internal::selectParent(nodesByPos[offset], &morpheme,
                                           this->dictionary);
      auto connectionCost =
          this->dictionary->getConnectionCost(parent->morpheme, &morpheme);

      int length =
          internal::groupingUnknownCharacters(current, category, dictionary);

      auto newNode = new internal::TrieNode(
          nodeId++, parent->cost + wordCost + connectionCost + spaceCost,
          parent->lastPositionIndex + length + numSpaces, length, &morpheme,
          parent);
      nodesByPos[newNode->lastPositionIndex].emplace_back(newNode);

      if (visualizer != nullptr) {
        visualizer->addNode(
            parent->lastPositionIndex - parent->length, parent->uniqueNodeId,
            parent->morpheme, parent->lastPositionIndex + numSpaces,
            newNode->uniqueNodeId, newNode->morpheme,
            std::string(begin + (newNode->lastPositionIndex - newNode->length),
                        begin + newNode->lastPositionIndex),
            wordCost, connectionCost);
      }

      offset += numSpaces + length;
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
        auto spaceCost =
            internal::getSpacePenalty(morpheme.postag(), numSpaces);

        auto parent = internal::selectParent(nodesByPos[offset], &morpheme,
                                             this->dictionary);
        auto connectionCost =
            this->dictionary->getConnectionCost(parent->morpheme, &morpheme);

        auto newNode = new internal::TrieNode(
            nodeId++, parent->cost + wordCost + connectionCost + spaceCost,
            parent->lastPositionIndex + trieResult.length + numSpaces,
            trieResult.length, &morpheme, parent);
        nodesByPos[newNode->lastPositionIndex].emplace_back(newNode);

        if (visualizer != nullptr) {
          visualizer->addNode(
              parent->lastPositionIndex - parent->length, parent->uniqueNodeId,
              parent->morpheme, parent->lastPositionIndex + numSpaces,
              newNode->uniqueNodeId, newNode->morpheme,
              std::string(
                  begin + (newNode->lastPositionIndex - newNode->length),
                  begin + newNode->lastPositionIndex),
              wordCost, connectionCost);
        }
      }
    }

    offset += numSpaces;
    U8_FWD_1_UNSAFE(begin, offset);
  }

  // TODO lattice output
  LOG(INFO) << minimumCost;
  std::shared_ptr<internal::TrieNode> currentNode = optimalPath;
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
