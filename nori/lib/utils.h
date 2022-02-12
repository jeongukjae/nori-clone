#ifndef __NORI_UTILS_H__
#define __NORI_UTILS_H__

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/protos/dictionary.pb.h"

namespace nori {
namespace utils {
namespace internal {

namespace LastCharType {
enum LastCharType {
  NNG = 1,
  NNG_T = 2,
  NNG_F = 3,
};
}

// detect that term has jongsung at last character to use in
// nori::dictionary::UserDictionary.
LastCharType::LastCharType detectLastCharacterType(absl::string_view input);

// normalize utf8 string
absl::Status normalizeUTF8(const std::string input, std::string& output,
                           absl::string_view normalizationForm = "NFKC");

// list all files in the directory. This function returns paths as sorted order.
// If functor returns false for given paths, this function will filter them.
//
// Example:
//   internal::listDirectory("./testdata/listDirectory", paths,
//     [](absl::string_view path) { return true; });
void listDirectory(absl::string_view directory, std::vector<std::string>& paths,
                   std::function<bool(std::string)> functor);

// join path
std::string joinPath(absl::string_view directory, absl::string_view filename);

// Parse csv from raw string `line` and put outputs into `entries`.
void parseCSVLine(std::string line, std::vector<std::string>& entries);

// Parse csv rows
std::vector<std::string> parseCSVLine(std::string line);

// trim whitespaces of input text
void trimWhitespaces(std::string& text);

// wrapping absl::SimpleAtoi
int simpleAtoi(absl::string_view input);

// wrapping absl::SimpleHexAtoi
int simpleHexAtoi(absl::string_view input);

}  // namespace internal

// resolve string pos type to proto's enum value
nori::protos::POSType resolvePOSType(absl::string_view name);

// resolve string pos tag to proto's enum value
nori::protos::POSTag resolvePOSTag(absl::string_view name);

// check is directory
bool isDirectory(const std::string& path);

// lowercase string
std::string lowercaseUTF8(const absl::string_view input);

}  // namespace utils
}  // namespace nori

#endif  // __NORI_UTILS_H__
