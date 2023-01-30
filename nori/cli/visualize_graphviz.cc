#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/tokenizer.h"
#include "nori/lib/utils.h"

ABSL_FLAG(std::string, dictionary, "./dictionary/latest-dictionary.nori",
          "Path to nori dictionary");
ABSL_FLAG(std::string, user_dictionary, "./dictionary/latest-userdict.txt",
          "Path to nori user dictionary");
ABSL_FLAG(std::string, input,
          "Nori-clone은 C++로 Nori를 재작성하기 위한 프로젝트입니다.",
          "Text to analyze");
ABSL_FLAG(std::string, output, "nori.dot", "Output path for dotfile");

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Visualize Nori's tokenization graph");
  absl::ParseCommandLine(argc, argv);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto dictionaryFlag = absl::GetFlag(FLAGS_dictionary);
  auto userDictionaryFlag = absl::GetFlag(FLAGS_user_dictionary);
  auto inputFlag = absl::GetFlag(FLAGS_input);
  auto outputFlag = absl::GetFlag(FLAGS_output);

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(dictionaryFlag);
  CHECK(status.ok()) << status.message();
  if (userDictionaryFlag != "") {
    LOG(INFO) << "Read user dictionary: " << userDictionaryFlag;
    status = dictionary.loadUser(userDictionaryFlag);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  auto normalizer = dictionary.getNormalizer();
  nori::GraphvizVisualizer visualizer;
  LOG(INFO) << "Input message: " << inputFlag;
  nori::Lattice lattice;
  status = lattice.setSentence(inputFlag, normalizer);
  CHECK(status.ok()) << status.message();

  status = tokenizer.tokenize(lattice, &visualizer);
  CHECK(status.ok()) << status.message();

  LOG(INFO) << "Tokenization Result: ";
  for (const auto& token : *lattice.getTokens()) {
    std::vector<std::string> posTagStr;
    for (const auto& postag : token.morpheme->pos_tags()) {
      posTagStr.push_back(nori::protos::POSTag_Name(postag));
    }

    LOG(INFO) << token.surface << ", " << absl::StrJoin(posTagStr, "+");
  }

  LOG(INFO) << "Write output to file " << outputFlag;
  std::ofstream ofs(outputFlag);
  CHECK(ofs.good()) << "Cannot open file to write";
  ofs << visualizer.str();
  ofs.close();

  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
