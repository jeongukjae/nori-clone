#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/dictionary.h"
#include "nori/graphviz_visualize.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/tokenizer.h"
#include "nori/utils.h"

DEFINE_string(dictionary, "./dictionary", "Path to nori dictionary");
DEFINE_string(input, "21세기 세종계획", "Text to analyze");
DEFINE_string(output, "nori.dot", "Output path for dotfile");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.load(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();

  nori::NoriTokenizer tokenizer(&dictionary);
  nori::Lattice lattice;
  nori::GraphvizVisualizer visualizer;

  std::string inputs = FLAGS_input;
  LOG(INFO) << "Input message: " << FLAGS_input;
  lattice.setSentence(FLAGS_input);
  status = tokenizer.tokenize(lattice, &visualizer);
  CHECK(status.ok()) << status.message();

  LOG(INFO) << "Tokenization Result: ";
  for (const auto& token : *lattice.getTokens()) {
    std::string posTagStr = "";
    for (const auto& postag : token->morpheme->postag()) {
      absl::StrAppend(&posTagStr, nori::POSTag_Name(postag), "+");
    }
    posTagStr.pop_back();

    LOG(INFO) << token->surface << ", " << posTagStr;
  }

  LOG(INFO) << "Write output to file " << FLAGS_output;
  std::ofstream ofs(FLAGS_output);
  CHECK(ofs.good()) << "Cannot open file to write";
  ofs << visualizer.str();
  ofs.close();
  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
