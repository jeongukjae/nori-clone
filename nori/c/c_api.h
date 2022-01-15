#ifndef __NORI_C_API_H__
#define __NORI_C_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct Tokenizer_H Tokenizer;
typedef struct Dictionary_H Dictionary;

typedef struct {
  int leftId;
  int rightId;
  int wordCost;
  int posType;
  int* posTag;
  int posTagLength;

  int exprLength;
  int* exprPosTag;
  const char** exprSurface;
} Morpheme;

typedef struct {
  size_t offset;
  size_t length;
  Morpheme morpheme;
} Token;

typedef struct {
  Token* tokens;
  int tokenLength;
} Lattice;

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

// Free tokenizer resources
void freeTokenizer(const Dictionary* dictionary, const Tokenizer* tokenizer);

// tokenize input str
// input string must be NULL-terminated
int tokenize(const Tokenizer* tokenizer, const char* str, Lattice** latticeOut);

// free lattice resource
void freeLattice(const Lattice* lattice);

#ifdef __cplusplus
}
#endif

#endif  // __NORI_C_API_H__
