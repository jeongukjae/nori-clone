#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/builder.h"
#include "nori/protos/dictionary.pb.h"

bool isDirectory(const std::string& path);

DEFINE_string(mecab_dic, "",
              "Path to mecab dictionary. This cli program reads "
              "{matrix,char,unk}.def and all CSV files");
DEFINE_string(normalization_form, "NFKC",
              "Unicode normalization form for dictionary of MeCab");
DEFINE_bool(normalize, true, "whether to normalize dictionary of MeCab");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Build Nori dictionary from MeCab's dictionary.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CHECK(isDirectory(FLAGS_mecab_dic))
      << "Cannot find directory " << FLAGS_mecab_dic;
  LOG(INFO) << "MeCab dictionary path: " << FLAGS_mecab_dic;

  nori::dictionary::builder::MeCabDictionaryBuilder builder(
      FLAGS_normalize, FLAGS_normalization_form);

  google::protobuf::ShutdownProtobufLibrary();
}

bool isDirectory(const std::string& path) {
  struct stat s;
  if ((stat(path.c_str(), &s) == 0) && (s.st_mode & S_IFDIR)) {
    return true;
  }
  return false;
}
