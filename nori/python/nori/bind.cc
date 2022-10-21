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
  const nori::protos::Morpheme *morpheme;
  const size_t offset;
  const size_t length;

  PyToken(const std::string surface, const nori::protos::Morpheme *morpheme,
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
    const google::protobuf::RepeatedPtrField<nori::protos::Morpheme_ExprToken>
        &expressions) {
  std::vector<std::string> tokens;
  tokens.reserve(expressions.size());

  for (const auto &expression : expressions) {
    tokens.push_back(
        absl::StrCat(expression.surface(), "/",
                     nori::protos::POSTag_Name(expression.pos_tag())));
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
                 [](int tag) { return nori::protos::POSTag_Name(tag); });

  return results;
}

class PyNoriTokenizer {
 public:
  PyNoriTokenizer() : tokenizer_(&dictionary_) {}

  void load_prebuilt_dictionary(const std::string &path) {
    auto status = dictionary_.loadPrebuilt(path);
    if (!status.ok()) {
      throw std::runtime_error("cannot load dictionary");
    }
  }

  void load_user_dictionary(const std::string &path) {
    auto status = dictionary_.loadUser(path);
    if (!status.ok()) {
      throw std::runtime_error("cannot load dictionary");
    }
  }

  PyLattice tokenize(const std::string sentence) {
    if (!dictionary_.isInitialized()) {
      throw std::runtime_error("dictionary is not initialized");
    }

    nori::Lattice lattice;
    auto status = lattice.setSentence(sentence, dictionary_.getNormalizer());

    if (!status.ok()) {
      throw std::runtime_error(
          absl::StrCat("Cannot normalize string ", sentence));
    }

    status = tokenizer_.tokenize(lattice);
    if (!status.ok()) {
      throw std::runtime_error(
          absl::StrCat("Cannot tokenize string ", sentence));
    }

    return PyLattice(lattice);
  }

 private:
  nori::NoriTokenizer tokenizer_;
  nori::dictionary::Dictionary dictionary_;
};

PYBIND11_MODULE(bind, m) {
  m.attr("__name__") = "nori.bind";

  py::class_<PyToken>(m, "Token")
      .def_readonly("surface", &PyToken::surface)
      .def_readonly("offset", &PyToken::offset)
      .def_readonly("length", &PyToken::length)
      .def_property_readonly(
          "leftid",
          [](const PyToken &token) { return token.morpheme->left_id(); })
      .def_property_readonly(
          "rightid",
          [](const PyToken &token) { return token.morpheme->right_id(); })
      .def_property_readonly("postag",
                             [](const PyToken &token) {
                               return getPostags(token.morpheme->pos_tags());
                             })
      .def_property_readonly(
          "postype",
          [](const PyToken &token) {
            return nori::protos::POSType_Name(token.morpheme->pos_type());
          })
      .def_property_readonly(
          "expr",
          [](const PyToken &token) {
            auto exprs = token.morpheme->expression();

            std::vector<std::pair<std::string, std::string>> tokens;
            tokens.reserve(exprs.size());

            for (const auto &expression : exprs) {
              tokens.push_back(std::make_pair(
                  expression.surface(),
                  nori::protos::POSTag_Name(expression.pos_tag())));
            }

            return tokens;
          })
      .def("__repr__", [](const PyToken &token) {
        return absl::StrCat(
            "<Token sruface=\"", token.surface, "\", postag=\"",
            absl::StrJoin(getPostags(token.morpheme->pos_tags()), "+"),
            "\", expr=\"", stringifyExpression(token.morpheme->expression()),
            "\", offset=", token.offset, ", length=", token.length, ">");
      });

  py::class_<PyLattice>(m, "Lattice")
      .def_readonly("tokens", &PyLattice::tokens)
      .def_readonly("sentence", &PyLattice::sentence);

  py::class_<PyNoriTokenizer>(m, "NoriTokenizer")
      .def(py::init())
      .def("load_prebuilt_dictionary",
           &PyNoriTokenizer::load_prebuilt_dictionary)
      .def("load_user_dictionary", &PyNoriTokenizer::load_user_dictionary)
      .def("tokenize", &PyNoriTokenizer::tokenize);
}
