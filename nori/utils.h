#ifndef __NORI_UTILS_H__
#define __NORI_UTILS_H__

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/protos/dictionary.pb.h"

namespace nori {
namespace utils {
namespace internal {

// normalize utf8 string
absl::Status normalizeUTF8(const std::string input, std::string& output,
                           absl::string_view normalizationForm);

// list all files in the directory. This function returns paths as sorted order.
// If functor returns false for given paths, this function will filter them.
//
// Example:
//   internal::listDirectory("./testdata/listDirectory", paths,
//     [](absl::string_view path) { return true; });
void listDirectory(absl::string_view directory, std::vector<std::string>& paths,
                   std::function<bool(std::string)> functor);

// Parse csv from raw string `line` and put outputs into `entries`.
void parsCSVLine(std::string line, std::vector<std::string>& entries);

// trim whitespaces of input text
void trimWhitespaces(std::string& text);

// wrapping absl::SimpleAtoi
int simpleAtoi(absl::string_view input);

}  // namespace internal

// resolve string pos type to proto's enum value
nori::POSType resolveType(absl::string_view name);

}  // namespace utils
}  // namespace nori

#endif  // __NORI_UTILS_H__