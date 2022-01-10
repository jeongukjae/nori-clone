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

#define MINIMUM_COST_DEFAULT 1e+10

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
                       const nori::dictionary::Dictionary* dictionary) {
  auto candidatesSize = candidates.size();
  if (candidatesSize == 0) return nullptr;

  int result = 0;
  auto minimumCost = candidates[0].cost +
                     dictionary->getConnectionCost(
                         candidates[0].morpheme->rightid(), morpheme->leftid());

  for (int i = 1; i < candidatesSize; i++) {
    auto cost = candidates[i].cost +
                dictionary->getConnectionCost(candidates[i].morpheme->rightid(),
                                              morpheme->leftid());
    if (cost < minimumCost) {
      minimumCost = cost;
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

  internal::TrieNode bestEos(nodeId++, MINIMUM_COST_DEFAULT, inputText.length(),
                             0, bosEosMorpheme);

  int offset = 0, numSpaces = 0;
  while ((current = begin + offset) <= end) {
    if (nodesByPos[offset].size() == 0) {
      U8_FWD_1_UNSAFE(begin, offset);
      continue;
    }

    // skip whitespaces
    numSpaces = 0;
    while (std::isspace(*current)) {
      current++;
      numSpaces++;
    }

    // Handling EOS node
    // end of parsing of this path
    if (current == end) {
      internal::TrieNode* parent = internal::selectParent(
          nodesByPos[offset], bosEosMorpheme, this->dictionary);
      if (visualizer != nullptr) {
        visualizer->addEos(parent->lastPositionIndex - parent->length,
                           parent->uniqueNodeId, parent->morpheme);
      }

      int connectionCost = this->dictionary->getConnectionCost(
          parent->morpheme->rightid(),
          this->dictionary->getBosEosMorpheme()->leftid());
      int eosCost = parent->cost + connectionCost;

      if (eosCost < bestEos.cost) {
        bestEos.cost = eosCost;
        bestEos.parent = parent;
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
      const nori::Morpheme* morpheme =
          &dictionary->getUnkDictionary()->morphememap().at(category);
      const int wordCost = morpheme->wordcost();
      // auto spaceCost = internal::getSpacePenalty(morpheme->postag(),
      // numSpaces);
      const int spaceCost = 0;

      auto parent = internal::selectParent(nodesByPos[offset], morpheme,
                                           this->dictionary);
      auto connectionCost = this->dictionary->getConnectionCost(
          parent->morpheme->rightid(), morpheme->leftid());

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
      auto morphemeSize = this->dictionary->getTokenDictionary()
                              ->morphemelistmap()
                              .at(trieResult.value)
                              .morphemes_size();

      for (int j = 0; j < morphemeSize; j++) {
        const auto* morpheme = &this->dictionary->getTokenDictionary()
                                    ->morphemelistmap()
                                    .at(trieResult.value)
                                    .morphemes(j);

        int wordCost = morpheme->wordcost();
        int spaceCost = internal::getSpacePenalty(morpheme, numSpaces);

        internal::TrieNode* parent = internal::selectParent(
            nodesByPos[offset], morpheme, this->dictionary);
        int connectionCost = this->dictionary->getConnectionCost(
            parent->morpheme->rightid(), morpheme->leftid());

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

  int numNode = 0;
  internal::TrieNode* currentNode = &bestEos;
  while (currentNode != NULL) {
    currentNode = currentNode->parent;
    numNode++;
  }

  auto outputTokens = lattice.getMutableTokens();
  outputTokens->reserve(numNode);
  currentNode = &bestEos;

  for (int index = numNode - 1; index >= 0;
       index--, currentNode = currentNode->parent) {
    size_t start = currentNode->lastPositionIndex - currentNode->length;

    // BOS or EOS
    if (currentNode->length == 0 &&
        (currentNode->lastPositionIndex == 0 ||
         currentNode->lastPositionIndex == inputText.size())) {
      outputTokens->emplace_back(new Token(this->dictionary->getBosEosSurface(),
                                           currentNode->morpheme, start,
                                           currentNode->length));
    } else {
      outputTokens->emplace_back(
          new Token(inputText.substr(start, currentNode->length),
                    currentNode->morpheme, start, currentNode->length));
    }
  }
  std::reverse(outputTokens->begin(), outputTokens->end());

  if (visualizer != nullptr) {
    visualizer->finish();
  }

  return absl::OkStatus();
}

}  // namespace nori
