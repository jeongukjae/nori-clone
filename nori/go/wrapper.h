#ifndef __NORI_WRAPPER_FOR_GO_H__
#define __NORI_WRAPPER_FOR_GO_H__

// initialize using given dictionary path and user dictionary path.
// set dictionary pointer in output.
//
// * this function will return 0 if initializing succeed, 1 if loading prebuilt
//   dictionary failed, or 2 if loading user dictionary failed.
// * all path should be NULL-terminated.
// * if userDictionaryPath is NULL, this function will skip loading user
//   dictionary
int initializeTokenizer(const char* dictionaryPath,
                        const char* userDictionaryPath, void** dictionaryOutput,
                        void** tokenizerOutput);

void freeTokenizer(void* dictionary, void* tokenizer);

int tokenize(const void* tokenizer, const char* str, void** latticeOut);

void freeLattice(const void* lattice);

#endif  // __NORI_WRAPPER_FOR_GO_H__
