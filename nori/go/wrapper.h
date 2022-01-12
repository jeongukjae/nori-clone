#ifndef __NORI_WRAPPER_FOR_GO_H__
#define __NORI_WRAPPER_FOR_GO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tokenizer_H Tokenizer;
typedef struct Dictionary_H Dictionary;
typedef struct Lattice_H Lattice;

// initialize using given dictionary path and user dictionary path.
// set dictionary pointer in output.
//
// * this function will return 0 if initializing succeed, 1 if loading prebuilt
//   dictionary failed, or 2 if loading user dictionary failed.
// * all path should be NULL-terminated.
// * if userDictionaryPath is NULL, this function will skip loading user
//   dictionary
int initializeTokenizer(const char* dictionaryPath,
                        const char* userDictionaryPath,
                        Dictionary** dictionaryOutput,
                        Tokenizer** tokenizerOutput);

void freeTokenizer(Dictionary* dictionary, Tokenizer* tokenizer);

int tokenize(const Tokenizer* tokenizer, const char* str, Lattice** latticeOut);

void freeLattice(const Lattice* lattice);

#ifdef __cplusplus
}
#endif

#endif  // __NORI_WRAPPER_FOR_GO_H__
