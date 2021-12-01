#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/dictionary.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/utils.h"

DEFINE_string(dictionary, "./dictionary", "Path to nori dictionary");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CHECK(nori::utils::isDirectory(FLAGS_dictionary))
      << "Cannot find directory " << FLAGS_dictionary;
  LOG(INFO) << "Dictionary path: " << FLAGS_dictionary;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.load(FLAGS_dictionary);
  CHECK(status.ok()) << status;

  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
