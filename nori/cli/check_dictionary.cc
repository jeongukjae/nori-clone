#include <sys/stat.h>
#include <sys/types.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"

ABSL_FLAG(std::string, dictionary, "./dictionary/latest-dictionary.nori",
          "Path to nori dictionary");
ABSL_FLAG(std::string, user_dictionary, "./dictionary/latest-userdict.txt",
          "Path to nori's user dictionary");

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Check nori dictionary files");
  absl::ParseCommandLine(argc, argv);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto dictionaryFlag = absl::GetFlag(FLAGS_dictionary);
  auto userDictionaryFlag = absl::GetFlag(FLAGS_user_dictionary);

  LOG(INFO) << "Dictionary path: " << dictionaryFlag;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(dictionaryFlag);
  CHECK(status.ok()) << status.message();

  if (userDictionaryFlag != "") {
    LOG(INFO) << "Read user dictionary: " << userDictionaryFlag;
    status = dictionary.loadUser(userDictionaryFlag);
    CHECK(status.ok()) << status.message();
  }

  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
