#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>
#include <google/protobuf/repeated_field.h>

#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "icu4c/source/common/unicode/utf.h"
#include "nori/protos/dictionary.pb.h"

namespace nori {

namespace internal {

struct TrieNode {
  int uniqueNodeId;
  size_t cost;
  size_t lastPositionIndex;
  size_t length;

  const nori::Morpheme* morpheme;
  TrieNode* parent;

  TrieNode(int uniqueNodeId, size_t cost, size_t lastPositionIndex,
           size_t length, const nori::Morpheme* morpheme,
           TrieNode* parent = nullptr)
      : uniqueNodeId(uniqueNodeId),
        cost(cost),
        lastPositionIndex(lastPositionIndex),
        length(length),
        morpheme(morpheme),
        parent(parent) {}
};

int getSpacePenalty(const nori::Morpheme* morpheme, int numSpaces) {
  if (numSpaces == 0) return 0;
  if (morpheme->postag_size() == 0) {
    LOG(ERROR) << "Cannot get postag";
    return 0;
  }

  switch (nori::POSTag(morpheme->postag(0))) {
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

int groupingUnknownCharacters(const char* current,
                              nori::CharacterClass category,
                              const nori::dictionary::Dictionary* dictionary) {
  int offset = 0;
  U8_FWD_1_UNSAFE(current, offset);

  while (dictionary->getCharClass(current + offset) == category) {
    U8_FWD_1_UNSAFE(current, offset);
  }

  return offset;
}

TrieNode* selectParent(std::vector<internal::TrieNode>& candidates,
                       const nori::Morpheme* morpheme,
                       const nori::dictionary::Dictionary* dictionary,
                       int& connectionCost) {
  auto candidatesSize = candidates.size();
  if (candidatesSize == 0) return nullptr;

  int result = 0;
  connectionCost = candidates[0].cost +
                   dictionary->getConnectionCost(
                       candidates[0].morpheme->rightid(), morpheme->leftid());

  for (int i = 1; i < candidatesSize; i++) {
    auto cost = candidates[i].cost +
                dictionary->getConnectionCost(candidates[i].morpheme->rightid(),
                                              morpheme->leftid());
    if (cost < connectionCost) {
      connectionCost = cost;
      result = i;
    }
  }
  return &candidates[result];
}

}  // namespace internal

// NoriTokenizer class
typedef Darts::DoubleArray::result_pair_type DartsResults;

absl::Status NoriTokenizer::tokenize(Lattice& lattice,
                                     GraphvizVisualizer* visualizer) const {
  if (visualizer != nullptr) {
    visualizer->reset();
  }

  const nori::Morpheme* bosEosMorpheme = this->dictionary->getBosEosMorpheme();
  absl::string_view inputText = lattice.getSentence();

  const char* begin = inputText.begin();
  const char* current = begin;
  const char* end = inputText.end();
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  int nodeId = 0;
  std::vector<std::vector<internal::TrieNode>> nodesByPos(inputText.length() +
                                                          1);

  // bos node;
  nodesByPos[0].emplace_back(nodeId++, 0, 0, 0, bosEosMorpheme);

  int offset = 0, numSpaces = 0;
  while ((current = begin + offset) < end) {
    if (nodesByPos[offset].size() == 0) {
      U8_FWD_1_UNSAFE(begin, offset);
      continue;
    }

    // skip whitespaces
    numSpaces = 0;
    while (std::isspace(*current)) {
      current++;
      numSpaces++;

      if (current == end) break;
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
      const nori::Morpheme* morpheme =
          &dictionary->getUnkDictionary()->morphememap().at(category);
      const int wordCost = morpheme->wordcost();
      auto spaceCost = internal::getSpacePenalty(morpheme, numSpaces);
      int connectionCost;
      auto parent = internal::selectParent(nodesByPos[offset], morpheme,
                                           this->dictionary, connectionCost);

      int length =
          internal::groupingUnknownCharacters(current, category, dictionary);

      auto lastPositionIndex = parent->lastPositionIndex + numSpaces + length;
      auto lastNodeId = nodeId;
      auto cost = parent->cost + wordCost + connectionCost + spaceCost;

      nodesByPos[lastPositionIndex].emplace_back(
          nodeId++, cost, lastPositionIndex, length, morpheme, parent);

      if (visualizer != nullptr) {
        visualizer->addNode(
            parent->lastPositionIndex - parent->length, parent->uniqueNodeId,
            parent->morpheme, parent->lastPositionIndex + numSpaces, lastNodeId,
            morpheme,
            std::string(begin + (parent->lastPositionIndex + numSpaces),
                        begin + lastPositionIndex),
            wordCost, connectionCost, cost);
      }

      offset += numSpaces;
      U8_FWD_1_UNSAFE(begin, offset);
      continue;
    }

    for (int k = 0; k < numNodes; ++k) {
      auto trieResult = trieResults[k];
      auto morphemeList =
          &this->dictionary->getTokenDictionary()->morphemelistmap().at(
              trieResult.value);
      auto morphemeSize = morphemeList->morphemes_size();

      for (int j = 0; j < morphemeSize; j++) {
        const auto* morpheme = &morphemeList->morphemes(j);

        int wordCost = morpheme->wordcost();
        int spaceCost = internal::getSpacePenalty(morpheme, numSpaces);
        int connectionCost;
        internal::TrieNode* parent = internal::selectParent(
            nodesByPos[offset], morpheme, this->dictionary, connectionCost);

        int lastPositionIndex =
            parent->lastPositionIndex + numSpaces + trieResult.length;
        int lastNodeId = nodeId;
        int cost = parent->cost + wordCost + connectionCost + spaceCost;
        nodesByPos[lastPositionIndex].emplace_back(
            nodeId++, cost, lastPositionIndex, trieResult.length, morpheme,
            parent);

        if (visualizer != nullptr) {
          visualizer->addNode(
              parent->lastPositionIndex - parent->length, parent->uniqueNodeId,
              parent->morpheme, parent->lastPositionIndex + numSpaces,
              lastNodeId, morpheme,
              std::string(begin + (parent->lastPositionIndex + numSpaces),
                          begin + lastPositionIndex),
              wordCost, connectionCost, cost);
        }
      }
    }

    offset += numSpaces;
    U8_FWD_1_UNSAFE(begin, offset);
  }

  // Handling EOS node
  // end of parsing of this path
  int eosConnectionCost;
  internal::TrieNode* bestPath = internal::selectParent(
      nodesByPos[offset], bosEosMorpheme, this->dictionary, eosConnectionCost);
  if (visualizer != nullptr) {
    visualizer->addEos(bestPath->lastPositionIndex - bestPath->length,
                       bestPath->uniqueNodeId, bestPath->morpheme);
  }
  internal::TrieNode eosNode(0, 0, inputText.length(), 0, bosEosMorpheme,
                             bestPath);

  // count node from eos to bos
  int numNode = 0;
  internal::TrieNode* currentNode = &eosNode;
  std::vector<internal::TrieNode*> nodes;
  while (currentNode != NULL) {
    nodes.push_back(currentNode);
    currentNode = currentNode->parent;
    numNode++;
  }
  std::reverse(nodes.begin(), nodes.end());

  // set outputs
  auto outputTokens = lattice.getMutableTokens();
  outputTokens->reserve(numNode);
  currentNode = &eosNode;

  for (const auto& node : nodes) {
    size_t start = node->lastPositionIndex - node->length;

    // BOS or EOS
    if (node->length == 0 && (node->lastPositionIndex == 0 ||
                              node->lastPositionIndex == inputText.size())) {
      outputTokens->emplace_back(this->dictionary->getBosEosSurface(),
                                 node->morpheme, start, node->length);
    } else {
      outputTokens->emplace_back(inputText.substr(start, node->length),
                                 node->morpheme, start, node->length);
    }
  }

  if (visualizer != nullptr) {
    visualizer->finish();
  }

  return absl::OkStatus();
}

}  // namespace nori
