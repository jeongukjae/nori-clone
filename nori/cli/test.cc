#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/dictionary.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/tokenizer.h"
#include "nori/utils.h"

DEFINE_string(dictionary, "./dictionary", "Path to nori dictionary");

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
  std::string inputs = "21세기 세종계획";
  LOG(INFO) << "Input message: " << inputs;
  tokenizer.tokenize(inputs, &lattice).IgnoreError();

  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
