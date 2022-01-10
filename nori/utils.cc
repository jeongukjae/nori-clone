#include "nori/utils.h"

#include <dirent.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "icu4c/source/common/unicode/errorcode.h"
#include "icu4c/source/common/unicode/normalizer2.h"
#include "icu4c/source/common/unicode/unistr.h"

namespace nori {
namespace utils {
namespace internal {

absl::Status normalizeUTF8(const std::string input, std::string& output,
                           absl::string_view normalizationForm) {
  icu::ErrorCode icuError;
  const icu::Normalizer2* normalizer;
  // TODO(jeongukjae): add normalization form
  if (normalizationForm == "NFKC") {
    normalizer = icu::Normalizer2::getNFKCInstance(icuError);
  } else {
    return absl::InvalidArgumentError(absl::StrCat(
        "Cannot find proper normalizer for form ", normalizationForm));
  }

  if (!icuError.isSuccess())
    return absl::InternalError(
        absl::StrCat("Cannot normalize unicode strings. Cannot get ",
                     normalizationForm, " Instance."));

  icu::StringByteSink<std::string> byte_sink(&output);
  normalizer->normalizeUTF8(0, icu::StringPiece(input.c_str(), input.size()),
                            byte_sink, nullptr, icuError);

  if (!icuError.isSuccess())
    return absl::InternalError(
        "Cannot normalize unicode strings. Cannot normalize input string.");

  return absl::OkStatus();
}

std::string lowercaseUTF8(const absl::string_view input) {
  std::string output;

  icu::UnicodeString us(input.data(), "UTF-8");
  us.toLower();
  us.toUTF8String(output);

  return output;
}

void listDirectory(absl::string_view directory, std::vector<std::string>& paths,
                   std::function<bool(std::string)> functor) {
  std::string direcotryString = absl::StrCat(directory);
  dirent* entry;
  DIR* dir = opendir(direcotryString.c_str());
  std::string path;

  if (dir != NULL) {
    while ((entry = readdir(dir)) != NULL) {
      path = joinPath(directory, entry->d_name);

      if (functor(path)) {
        paths.push_back(path);
      }
    }

    closedir(dir);
  }
  std::sort(paths.begin(), paths.end());
}

std::string joinPath(absl::string_view directory, absl::string_view filename) {
  if (absl::EndsWith(directory, "/")) return absl::StrCat(directory, filename);
  return absl::StrCat(directory, "/", filename);
}

void parseCSVLine(std::string line, std::vector<std::string>& entries) {
  bool insideQuote = false;
  int start = 0;

  for (int index = 0; index < line.length(); index++) {
    char c = line.at(index);

    if (c == '"') {
      insideQuote = !insideQuote;
    } else if (c == ',' && !insideQuote) {
      int length = index - start, offset = 0;
      while (offset < (index - start) / 2 &&
             (line.at(start + offset) == '"' &&
              line.at(index - offset - 1) == '"')) {
        length -= 2;
        start += 1;
        offset++;
      }
      entries.push_back(line.substr(start, length));
      start = index + 1;
    }
  }
  entries.push_back(line.substr(start));
}

std::vector<std::string> parseCSVLine(std::string line) {
  std::vector<std::string> entries;
  parseCSVLine(line, entries);
  return entries;
}

void trimWhitespaces(std::string& text) {
  // trim from start (in place)
  text.erase(text.begin(),
             std::find_if(text.begin(), text.end(),
                          [](unsigned char ch) { return !std::isspace(ch); }));

  // trim from end (in place)
  text.erase(std::find_if(text.rbegin(), text.rend(),
                          [](unsigned char ch) { return !std::isspace(ch); })
                 .base(),
             text.end());
}

int simpleAtoi(absl::string_view input) {
  int output;
  CHECK(absl::SimpleAtoi(input, &output)) << "Atoi error: " << input;
  return output;
}

int simpleHexAtoi(absl::string_view input) {
  int output;
  CHECK(absl::SimpleHexAtoi(input, &output)) << "Atoi error: " << input;
  return output;
}

}  // namespace internal

nori::POSType resolvePOSType(absl::string_view name) {
  if (name == "*") {
    return nori::POSType::MORPHEME;
  }

  nori::POSType output;
  CHECK(nori::POSType_Parse(absl::AsciiStrToUpper(name), &output))
      << "Cannot resolve POS type. name: " << name;
  return output;
}

nori::POSTag resolvePOSTag(absl::string_view name) {
  const auto tagUpper = absl::AsciiStrToUpper(name);

  if (absl::StartsWith(tagUpper, "J")) {
    return nori::POSTag::J;
  }
  if (absl::StartsWith(tagUpper, "E")) {
    return nori::POSTag::E;
  }

  nori::POSTag output;
  CHECK(nori::POSTag_Parse(tagUpper, &output))
      << "Cannot resolve POS tag. name: " << name;
  return output;
}

bool isDirectory(const std::string& path) {
  struct stat s;
  if ((stat(path.c_str(), &s) == 0) && (s.st_mode & S_IFDIR)) {
    return true;
  }
  return false;
}

}  // namespace utils
}  // namespace nori
