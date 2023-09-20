#include "nori/lib/tokenizer.h"

#include <google/protobuf/repeated_field.h>

#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "absl/log/log.h"
#include "darts_ac/darts_ac.h"
#include "icu4c/source/common/unicode/uchar.h"
#include "icu4c/source/common/unicode/uscript.h"
#include "icu4c/source/common/unicode/utf.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"

namespace nori {

namespace internal {

int getSpacePenalty(const nori::protos::Morpheme* morpheme, int numSpaces) {
  if (numSpaces == 0) return 0;
  if (morpheme->pos_tags_size() == 0) {
    LOG(ERROR) << "Cannot get postag";
    return 0;
  }

  switch (nori::protos::POSTag(morpheme->pos_tags(0))) {
    case nori::protos::POSTag::E:
    case nori::protos::POSTag::J:
    case nori::protos::POSTag::VCP:
    case nori::protos::POSTag::XSA:
    case nori::protos::POSTag::XSN:
    case nori::protos::POSTag::XSV:
      return 3000;
    default:
      return 0;
  }
}

inline bool isCommonOrInherited(UScriptCode sc) {
  return sc == USCRIPT_COMMON || sc == USCRIPT_INHERITED;
}

inline bool isPunctuation(UChar32 ch) {
  if (ch == 4510) {  // Hangul Letter Araea
    return true;
  }

  switch (u_charType(ch)) {
    case UCharCategory::U_SPACE_SEPARATOR:
    case UCharCategory::U_LINE_SEPARATOR:
    case UCharCategory::U_PARAGRAPH_SEPARATOR:
    case UCharCategory::U_CONTROL_CHAR:
    case UCharCategory::U_FORMAT_CHAR:
    case UCharCategory::U_DASH_PUNCTUATION:
    case UCharCategory::U_START_PUNCTUATION:
    case UCharCategory::U_END_PUNCTUATION:
    case UCharCategory::U_CONNECTOR_PUNCTUATION:
    case UCharCategory::U_OTHER_PUNCTUATION:
    case UCharCategory::U_MATH_SYMBOL:
    case UCharCategory::U_CURRENCY_SYMBOL:
    case UCharCategory::U_MODIFIER_SYMBOL:
    case UCharCategory::U_OTHER_SYMBOL:
    case UCharCategory::U_INITIAL_PUNCTUATION:
    case UCharCategory::U_FINAL_PUNCTUATION:
      return true;
  }
  return false;
}

int groupingUnknownCharacters(const char* begin, const char* end,
                              nori::protos::CharacterClass& category,
                              const nori::dictionary::Dictionary* dictionary,
                              const bool doGroup) {
  UChar32 currentChar;
  size_t length = end - begin;
  size_t offset = 0;
  size_t offsetToReturn;
  U8_NEXT_UNSAFE(begin, offset, currentChar);

  UErrorCode err = U_ZERO_ERROR;
  auto firstUScript = uscript_getScript(currentChar, &err);
  bool isFirstCommonOrInherited = isCommonOrInherited(firstUScript);
  bool isFirstPunctuation = isPunctuation(currentChar);
  bool isFirstDigit = u_isdigit(currentChar);
  if (U_FAILURE(err)) {
    return offset;
  }

  if (!doGroup) {
    return offset;
  }

  while (offset < length) {
    offsetToReturn = offset;
    U8_NEXT_UNSAFE(begin, offset, currentChar);
    auto currentUScript = uscript_getScript(currentChar, &err);
    if (U_FAILURE(err)) {
      return offsetToReturn;
    }

    bool isCurrentCommonOrInherited = isCommonOrInherited(currentUScript);
    bool isSameScript =
        ((firstUScript == currentUScript) || isFirstCommonOrInherited ||
         isCurrentCommonOrInherited) &&
        !u_hasBinaryProperty(currentChar, UCHAR_WHITE_SPACE);
    bool isCurrentPunctuation = isPunctuation(currentChar);
    bool isCurrentDigit = u_isdigit(currentChar);

    if (!isSameScript || (isFirstPunctuation != isCurrentPunctuation) ||
        (isFirstDigit != isCurrentDigit)) {
      return offsetToReturn;
    }

    if (isFirstCommonOrInherited && !isCurrentPunctuation) {
      firstUScript = currentUScript;
      category = dictionary->getCharClass(begin + offsetToReturn, end);
    }
  }

  return offset;
}

TrieNode* selectParent(std::vector<internal::TrieNode>& candidates,
                       const nori::protos::Morpheme* morpheme,
                       const nori::dictionary::Dictionary* dictionary,
                       int& connectionCost) {
  if (candidates.empty()) return nullptr;

  connectionCost = 1e+6;
  internal::TrieNode* result = nullptr;
  int minCost = connectionCost;

  for (auto& candidate : candidates) {
    if (!(candidate.position == 0 && candidate.length == 0) &&
        candidate.parent == nullptr)
      continue;

    auto currentConnectionCost = dictionary->getConnectionCost(
        candidate.morpheme->right_id(), morpheme->left_id());
    auto cost = candidate.cost + currentConnectionCost;
    if (cost < minCost) {
      minCost = cost;
      connectionCost = currentConnectionCost;
      result = &candidate;
    }
  }

  return result;
}

}  // namespace internal

// NoriTokenizer class
typedef darts_ac::DoubleArrayAhoCorasick::result_pair_type DartsResults;

absl::Status NoriTokenizer::tokenize(Lattice& lattice,
                                     GraphvizVisualizer* visualizer) const {
  const auto bosEosMorpheme = this->dictionary->getBosEosMorpheme();
  absl::string_view inputText = lattice.getSentence();
  std::vector<DartsResults> trieResults(maxTrieResults + 1);

  int nodeId = 0;
  std::vector<std::vector<internal::TrieNode>> nodesByEndPos(
      inputText.length() + 1);

  // Mark spaces first.
  std::vector<bool> isSpace(inputText.length() + 1, false);

  {
    int offset = 0;
    const char* begin = inputText.begin();

    while ((begin + offset) < inputText.end()) {
      U8_FWD_1_UNSAFE(begin, offset);
      if (std::isspace(inputText[offset])) {
        isSpace[offset] = true;
      }

      nodesByEndPos[offset].reserve(64);
    }
  }

  // Node found vector for unknown tokens.
  std::vector<bool> nodeFound(inputText.length() + 1, false);

  // Push BOS node.
  internal::TrieNode bosNode(nodeId++, 0, 0, 0, 0, bosEosMorpheme);
  nodesByEndPos[0].push_back(bosNode);
  nodeFound[0] = true;

  // Search all dictionaries.
  absl::Status status;

  status = this->findUserDictionaryTokens(lattice, nodesByEndPos, isSpace,
                                          nodeFound, nodeId);
  if (!status.ok()) {
    return status;
  }

  status = this->findPreBuiltTokens(lattice, nodesByEndPos, isSpace, nodeFound,
                                    nodeId);
  if (!status.ok()) {
    return status;
  }

  status = this->findUnknownTokens(lattice, nodesByEndPos, isSpace, nodeFound,
                                   nodeId);
  if (!status.ok()) {
    return status;
  }

  // Connecting all nodes.
  for (int i = 1; i <= inputText.length(); i++) {
    if (nodesByEndPos[i].empty()) {
      continue;
    }

    for (auto& node : nodesByEndPos[i]) {
      int connectionCost;
      const auto parent = internal::selectParent(
          nodesByEndPos[node.position - node.spaceBeforeToken], node.morpheme,
          this->dictionary, connectionCost);

      if (parent == nullptr) {
        // no parent node. skipping.
        continue;
      }

      node.parent = parent;
      node.cost = parent->cost + node.cost + connectionCost;
    }
  }

  // Handling EOS node
  // end of parsing of this path
  int lastSpaces = 0;
  for (int i = inputText.length() - 1; i > 0 && isSpace[i]; i--) {
    lastSpaces++;
  }
  int eosConnectionCost;
  internal::TrieNode* bestPath = internal::selectParent(
      nodesByEndPos[inputText.length() - lastSpaces], bosEosMorpheme,
      this->dictionary, eosConnectionCost);
  internal::TrieNode eosNode(nodeId++, 0, inputText.length(), 0, lastSpaces,
                             bosEosMorpheme);
  eosNode.parent = bestPath;

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
    // BOS or EOS. just check pointer address.
    if (node->morpheme == bosEosMorpheme) {
      outputTokens->emplace_back(this->dictionary->getBosEosSurface(),
                                node->morpheme, node->position, node->length);
    } else {
      outputTokens->emplace_back(inputText.substr(node->position, node->length),
                                node->morpheme, node->position, node->length);
    }
  }

  return absl::OkStatus();
}

absl::Status NoriTokenizer::findPreBuiltTokens(
    Lattice& lattice,
    std::vector<std::vector<internal::TrieNode>>& nodesByEndPos,
    std::vector<bool>& isSpaces, std::vector<bool>& nodeFound,
    int& nodeId) const {
  std::vector<DartsResults> trieResults(maxTrieResults + 1);
  const int numNodes = dictionary->getTrieAC()->find(
      lattice.getSentence().data(), trieResults.data(), maxTrieResults,
      lattice.getSentence().length());

  if (numNodes > maxTrieResults)
    return absl::InternalError("Cannot search aho-corasick");

  for (int k = 0; k < numNodes; ++k) {
    auto trieResult = trieResults[k];
    auto morphemeList =
        &this->dictionary->getTokens()->morphemes_list(trieResult.value);
    auto morphemeSize = morphemeList->morphemes_size();

    for (int j = 0; j < morphemeSize; j++) {
      const auto* morpheme = &morphemeList->morphemes(j);

      int numSpaces = 0;
      for (int i = trieResult.position - 1; i > 0 && isSpaces[i]; i--) {
        numSpaces++;
      }

      int spaceCost = internal::getSpacePenalty(morpheme, numSpaces);

      int endPosition = trieResult.position + trieResult.length;
      int cost = spaceCost + morpheme->word_cost();
      nodesByEndPos[endPosition].emplace_back(
          nodeId++, cost, trieResult.position, trieResult.length, numSpaces,
          morpheme);
      nodeFound[trieResult.position] = true;
    }
  }

  return absl::OkStatus();
}

absl::Status NoriTokenizer::findUserDictionaryTokens(
    Lattice& lattice,
    std::vector<std::vector<internal::TrieNode>>& nodesByEndPos,
    std::vector<bool>& isSpaces, std::vector<bool>& nodeFound,
    int& nodeId) const {
  // Skip if user dictionary is not initialized.
  if (!dictionary->isUserInitialized()) {
    return absl::OkStatus();
  }

  std::vector<DartsResults> trieResults(maxTrieResults + 1);
  const int numNodes = dictionary->getUserDict()->getTrieAC()->find(
      lattice.getSentence().data(), trieResults.data(), maxTrieResults,
      lattice.getSentence().length());

  if (numNodes > maxTrieResults)
    return absl::InternalError("Cannot search aho-corasick");

  for (int k = 0; k < numNodes; ++k) {
    auto trieResult = trieResults[k];
    const auto morpheme =
        &dictionary->getUserDict()->getMorphemes()->at(trieResults[k].value);

    int numSpaces = 0;
    for (int i = trieResult.position - 1; i > 0 && isSpaces[i]; i--) {
      numSpaces++;
    }

    int spaceCost = internal::getSpacePenalty(morpheme, numSpaces);
    int endPosition = trieResult.position + trieResult.length;

    int cost = spaceCost + morpheme->word_cost();
    nodesByEndPos[endPosition].emplace_back(nodeId++, cost, trieResult.position,
                                            trieResult.length, numSpaces,
                                            morpheme);
    nodeFound[trieResult.position] = true;
  }

  return absl::OkStatus();
}

absl::Status NoriTokenizer::findUnknownTokens(
    Lattice& lattice,
    std::vector<std::vector<internal::TrieNode>>& nodesByEndPos,
    std::vector<bool>& isSpaces, std::vector<bool>& nodeFound,
    int& nodeId) const {
  const char* begin = lattice.getSentence().begin();
  const char* end = lattice.getSentence().end();
  const char* current;

  int offset = 0;

  while ((current = begin + offset) < end) {
    if (isSpaces[offset]) {
      offset++;
      continue;
    }

    auto charDef = dictionary->getCharDef(current, end);
    if ((!nodeFound[offset]) || (charDef->invoke() == 1)) {
      auto category = dictionary->getCharClass(current, end);
      int length = internal::groupingUnknownCharacters(
          current, end, category, dictionary, charDef->group() == 1);

      int numSpaces = 0;
      for (int i = offset - 1; i > 0 && isSpaces[i]; i--) {
        numSpaces++;
      }

      const nori::protos::Morpheme* morpheme =
          &dictionary->getUnkTokens()->morpheme_map().at(category);
      const int wordCost = morpheme->word_cost();
      auto spaceCost = internal::getSpacePenalty(morpheme, numSpaces);
      auto cost = wordCost + spaceCost;

      nodesByEndPos[offset + length].emplace_back(nodeId++, cost, offset,
                                                  length, numSpaces, morpheme);
      nodeFound[offset] = true;
    }

    U8_FWD_1_UNSAFE(begin, offset);
  }

  return absl::OkStatus();
}

}  // namespace nori
