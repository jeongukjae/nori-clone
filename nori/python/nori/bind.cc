#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/tokenizer.h"

namespace py = pybind11;

struct PyToken {
  const std::string surface;
  const nori::Morpheme *morpheme;
  const size_t offset;
  const size_t length;

  PyToken(const std::string surface, const nori::Morpheme *morpheme,
          const size_t offset, const size_t length)
      : surface(surface), morpheme(morpheme), offset(offset), length(length) {}
};

struct PyLattice {
  std::vector<PyToken> tokens;
  std::string sentence;

  PyLattice(const nori::Lattice &lattice) {
    this->sentence = lattice.getSentence();
    this->tokens.reserve(lattice.getTokens()->size());
    for (const auto &token : *lattice.getTokens()) {
      this->tokens.emplace_back(
          std::string(token.surface.data(), token.surface.length()),
          token.morpheme, token.offset, token.length);
    }
  }
};

std::string stringifyExpression(
    const google::protobuf::RepeatedPtrField<nori::Morpheme_ExprToken>
        &expressions) {
  std::vector<std::string> tokens;
  tokens.reserve(expressions.size());

  for (const auto &expression : expressions) {
    tokens.push_back(absl::StrCat(expression.surface(), "/",
                                  nori::POSTag_Name(expression.postag())));
  }

  return absl::StrJoin(tokens, "+");
}

std::vector<std::string> getPostags(
    const google::protobuf::RepeatedField<google::protobuf::int32>
        &postaglist) {
  std::vector<std::string> results;
  results.reserve(postaglist.size());
  std::transform(postaglist.begin(), postaglist.end(),
                 std::back_inserter(results),
                 [](int tag) { return nori::POSTag_Name(tag); });

  return results;
}

PYBIND11_MODULE(bind, m) {
  m.attr("__name__") = "nori.bind";

  py::class_<PyToken>(m, "Token")
      .def_readonly("surface", &PyToken::surface)
      .def_readonly("offset", &PyToken::offset)
      .def_readonly("length", &PyToken::length)
      .def_property_readonly(
          "leftid",
          [](const PyToken &token) { return token.morpheme->leftid(); })
      .def_property_readonly(
          "rightid",
          [](const PyToken &token) { return token.morpheme->rightid(); })
      .def_property_readonly("postag",
                             [](const PyToken &token) {
                               return getPostags(token.morpheme->postag());
                             })
      .def_property_readonly(
          "postype",
          [](const PyToken &token) {
            return nori::POSType_Name(token.morpheme->postype());
          })
      .def_property_readonly(
          "expr",
          [](const PyToken &token) {
            auto exprs = token.morpheme->expression();

            std::vector<std::pair<std::string, std::string>> tokens;
            tokens.reserve(exprs.size());

            for (const auto &expression : exprs) {
              tokens.push_back(
                  std::make_pair(expression.surface(),
                                 nori::POSTag_Name(expression.postag())));
            }

            return tokens;
          })
      .def("__repr__", [](const PyToken &token) {
        return absl::StrCat(
            "<Token sruface=\"", token.surface, "\", postag=\"",
            absl::StrJoin(getPostags(token.morpheme->postag()), "+"),
            "\", expr=\"",
            stringifyExpression(token.morpheme->expression()),
            "\", offset=", token.offset, ", length=", token.length, ">");
      });

  py::class_<PyLattice>(m, "Lattice")
      .def_readonly("tokens", &PyLattice::tokens)
      .def_readonly("sentence", &PyLattice::sentence);

  py::class_<nori::dictionary::Dictionary>(m, "Dictionary")
      .def(py::init<>())
      .def(
          "load_prebuilt_dictionary",
          [](nori::dictionary::Dictionary &dictionary, const std::string path) {
            auto status = dictionary.loadPrebuilt(path);
            if (!status.ok()) {
              throw std::runtime_error("cannot load dictionary");
            }
          })
      .def("load_user_dictionary", [](nori::dictionary::Dictionary &dictionary,
                                      const std::string filename) {
        auto status = dictionary.loadUser(filename);
        if (!status.ok()) {
          throw std::runtime_error("cannot load dictionary:");
        }
      });

  py::class_<nori::NoriTokenizer>(m, "NoriTokenizer")
      .def(py::init<const nori::dictionary::Dictionary *>())
      .def("tokenize", [](const nori::NoriTokenizer &tokenizer,
                          const std::string sentence) {
        nori::Lattice lattice(sentence);
        auto status = tokenizer.tokenize(lattice);
        if (!status.ok()) {
          throw std::runtime_error(
              absl::StrCat("Cannot tokenize string ", sentence));
        }

        return PyLattice(lattice);
      });
}
