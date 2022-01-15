#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/tokenizer.h"
#include "nori/lib/utils.h"

DEFINE_string(dictionary, "./dictionary", "Path to nori dictionary");
DEFINE_string(user_dictionary, "./dictionary/userdict.txt",
              "Path to nori user dictionary");
DEFINE_string(input,
              "Nori-clone은 C++로 Nori를 재작성하기 위한 프로젝트입니다.",
              "Text to analyze");
DEFINE_string(output, "nori.dot", "Output path for dotfile");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();
  if (FLAGS_user_dictionary != "") {
    LOG(INFO) << "Read user dictionary: " << FLAGS_user_dictionary;
    status = dictionary.loadUser(FLAGS_user_dictionary);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  nori::GraphvizVisualizer visualizer;
  LOG(INFO) << "Input message: " << FLAGS_input;
  nori::Lattice lattice(FLAGS_input);

  status = tokenizer.tokenize(lattice, &visualizer);
  CHECK(status.ok()) << status.message();

  LOG(INFO) << "Tokenization Result: ";
  for (const auto& token : *lattice.getTokens()) {
    std::vector<std::string> posTagStr;
    for (const auto& postag : token.morpheme->postag()) {
      posTagStr.push_back(nori::POSTag_Name(postag));
    }

    LOG(INFO) << token.surface << ", " << absl::StrJoin(posTagStr, "+");
  }

  LOG(INFO) << "Write output to file " << FLAGS_output;
  std::ofstream ofs(FLAGS_output);
  CHECK(ofs.good()) << "Cannot open file to write";
  ofs << visualizer.str();
  ofs.close();

  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
