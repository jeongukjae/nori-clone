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

  std::unique_ptr<nori::Morpheme> morpheme;
  std::shared_ptr<TrieNode> parent;

  TrieNode(int uniqueNodeId, size_t cost, size_t lastPositionIndex,
           size_t length, const nori::Morpheme* morpheme,
           std::shared_ptr<TrieNode> parent = nullptr)
      : uniqueNodeId(uniqueNodeId),
        cost(cost),
        lastPositionIndex(lastPositionIndex),
        length(length),
        parent(parent) {
    this->morpheme.reset(morpheme->New());
    this->morpheme->CopyFrom(*morpheme);
  }
};

inline int getSpacePenalty(
    const google::protobuf::RepeatedField<google::protobuf::int32> posTags,
    int numSpaces) {
  if (numSpaces == 0) return 0;
  if (posTags.size() == 0) {
    LOG(ERROR) << "Cannot get postag";
    return 0;
  }

  switch (nori::POSTag(posTags.at(0))) {
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
  auto candidatesSize = candidates.size();
  if (candidatesSize == 0) return nullptr;

  std::shared_ptr<TrieNode> result = candidates[0];
  auto minimumCost = result->cost + dictionary->getConnectionCost(
                                        result->morpheme.get(), morpheme);

  for (int i = 1; i < candidatesSize; i++) {
    auto candidate = candidates[i];

    auto cost = candidate->cost + dictionary->getConnectionCost(
                                      candidate->morpheme.get(), morpheme);
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
  std::map<size_t, std::vector<std::shared_ptr<internal::TrieNode>>> nodesByPos;

  // bos node;
  nodesByPos[0].emplace_back(
      new internal::TrieNode(nodeId++, 0, 0, 0, bosEosMorpheme));

  size_t minimumCost = MINIMUM_COST_DEFAULT;
  std::shared_ptr<internal::TrieNode> optimalPath;

  size_t offset = 0;
  while ((current = begin + offset) <= end) {
    auto nodesByPosIterator = nodesByPos.find(offset);
    if (nodesByPosIterator == nodesByPos.end()) {
      U8_FWD_1_UNSAFE(begin, offset);
      continue;
    }
    auto nodesForOffset = nodesByPosIterator->second;

    // skip whitespaces
    int numSpaces = 0;
    while (std::isspace(*current)) {
      current++;
      numSpaces++;
    }

    // Handling EOS node
    // end of parsing of this path
    if (current == end) {
      auto parent = internal::selectParent(nodesForOffset, bosEosMorpheme,
                                           this->dictionary);
      auto connectionCost = this->dictionary->getConnectionCost(
          parent->morpheme.get(), this->dictionary->getBosEosMorpheme());
      auto eosNode = std::shared_ptr<internal::TrieNode>(new internal::TrieNode(
          nodeId++, parent->cost + connectionCost, parent->lastPositionIndex, 0,
          this->dictionary->getBosEosMorpheme(), parent));

      if (visualizer != nullptr) {
        visualizer->addEos(parent->lastPositionIndex - parent->length,
                           parent->uniqueNodeId, parent->morpheme.get());
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
      // auto spaceCost = internal::getSpacePenalty(morpheme.postag(),
      // numSpaces);
      int spaceCost = 0;

      auto parent =
          internal::selectParent(nodesForOffset, &morpheme, this->dictionary);
      auto connectionCost = this->dictionary->getConnectionCost(
          parent->morpheme.get(), &morpheme);

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
            parent->morpheme.get(), parent->lastPositionIndex + numSpaces,
            newNode->uniqueNodeId, newNode->morpheme.get(),
            std::string(begin + (newNode->lastPositionIndex - newNode->length),
                        begin + newNode->lastPositionIndex),
            wordCost, connectionCost, newNode->cost);
      }

      offset += numSpaces;
      U8_FWD_1_UNSAFE(begin, offset);
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

        auto parent =
            internal::selectParent(nodesForOffset, &morpheme, this->dictionary);
        auto connectionCost = this->dictionary->getConnectionCost(
            parent->morpheme.get(), &morpheme);

        auto newNode = new internal::TrieNode(
            nodeId++, parent->cost + wordCost + connectionCost + spaceCost,
            parent->lastPositionIndex + trieResult.length + numSpaces,
            trieResult.length, &morpheme, parent);
        nodesByPos[newNode->lastPositionIndex].emplace_back(newNode);

        if (visualizer != nullptr) {
          visualizer->addNode(
              parent->lastPositionIndex - parent->length, parent->uniqueNodeId,
              parent->morpheme.get(), parent->lastPositionIndex + numSpaces,
              newNode->uniqueNodeId, newNode->morpheme.get(),
              std::string(
                  begin + (newNode->lastPositionIndex - newNode->length),
                  begin + newNode->lastPositionIndex),
              wordCost, connectionCost, newNode->cost);
        }
      }
    }

    offset += numSpaces;
    U8_FWD_1_UNSAFE(begin, offset);
  }

  int numNode = 0;
  auto currentNode = optimalPath;
  while (currentNode != NULL) {
    currentNode = currentNode->parent;
    numNode++;
  }

  auto outputTokens = lattice.getMutableTokens();
  outputTokens->resize(numNode);
  currentNode = optimalPath;
  for (int index = numNode - 1; index >= 0;
       index--, currentNode = currentNode->parent) {
    size_t start = currentNode->lastPositionIndex - currentNode->length;

    // BOS or EOS
    if (currentNode->length == 0 &&
        (currentNode->lastPositionIndex == 0 ||
         currentNode->lastPositionIndex == inputText.size())) {
      (*outputTokens)[index] = std::make_shared<Token>(
          this->dictionary->getBosEosSurface(), currentNode->morpheme, start,
          currentNode->length);
    } else {
      (*outputTokens)[index] = std::make_shared<Token>(
          inputText.substr(start, currentNode->length), currentNode->morpheme,
          start, currentNode->length);
    }
  }

  if (visualizer != nullptr) {
    visualizer->finish();
  }

  return absl::OkStatus();
}

}  // namespace nori
