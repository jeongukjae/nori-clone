package nori

/*
#cgo LDFLAGS: -lnori_c_api

#include <stdlib.h>
#include "nori/c/c_api.h"
*/
import "C"
import (
	"fmt"
	"unsafe"
)

type NoriTokenizer struct {
	tokenizer  *C.Tokenizer
	dictionary *C.Dictionary
}

type TokenExpression struct {
	Surface string
	POSTag  int
}

type Token struct {
	Surface string

	LeftId     int
	RightId    int
	WordCost   int
	POSType    int
	POSTag     []int
	Expression []TokenExpression
}

func New(dicPath string, userDicPath string) (*NoriTokenizer, error) {
	cDicPath := C.CString(dicPath)
	defer C.free(unsafe.Pointer(cDicPath))

	tokenizer := NoriTokenizer{}
	var ret int32
	if userDicPath == "" {
		ret = int32(C.initializeTokenizer(cDicPath, nil, &tokenizer.dictionary, &tokenizer.tokenizer))
	} else {
		cUserDicPath := C.CString(userDicPath)
		defer C.free(unsafe.Pointer(cUserDicPath))

		ret = int32(C.initializeTokenizer(cDicPath, cUserDicPath, &tokenizer.dictionary, &tokenizer.tokenizer))
	}

	if ret == 1 {
		return nil, fmt.Errorf("Cannot create NoriTokenizer with dicPath %s", dicPath)
	} else if ret == 2 {
		return nil, fmt.Errorf("Cannot create NoriTokenizer with userDicPath %s", userDicPath)
	}

	return &tokenizer, nil
}

func (nt *NoriTokenizer) Tokenize(input string) (*[]Token, error) {
	cInput := C.CString(input)
	defer C.free(unsafe.Pointer(cInput))

	var lattice *C.Lattice
	ret := C.tokenize(nt.tokenizer, cInput, &lattice)
	if ret == 1 {
		return nil, fmt.Errorf("Cannot normalize input string %s", input)
	} else if ret == 2 {
		return nil, fmt.Errorf("Cannot tokenize input string %s", input)
	}
	defer C.freeLattice(lattice)

	tokenLength := int(lattice.tokenLength)
	tokens := make([]Token, tokenLength)
	sentence := C.GoString(lattice.sentence)
	cTokens := (*[1 << 30]C.Token)(unsafe.Pointer(lattice.tokens))

	tokens[0].Surface = "BOS/EOS"
	tokens[tokenLength-1].Surface = "BOS/EOS"

	for i := 1; i < tokenLength-1; i++ {
		cToken := cTokens[i]
		start := int(cToken.offset)
		end := start + int(cToken.length)

		tokens[i].Surface = sentence[start:end]

		cMorpheme := cToken.morpheme
		tokens[i].LeftId = int(cMorpheme.leftId)
		tokens[i].RightId = int(cMorpheme.rightId)
		tokens[i].WordCost = int(cMorpheme.wordCost)
		tokens[i].POSType = int(cMorpheme.posType)

		posTagLength := int(cMorpheme.posTagLength)
		tokens[i].POSTag = make([]int, int(cMorpheme.posTagLength))
		cPOSTag := (*[1 << 30]C.int)(unsafe.Pointer(cMorpheme.posTag))
		for j := 0; j < posTagLength; j++ {
			tokens[i].POSTag[j] = int(cPOSTag[j])
		}

		expressionLength := int(cMorpheme.exprLength)
		tokens[i].Expression = make([]TokenExpression, expressionLength)
		if expressionLength != 0 {
			cExprPOS := (*[1 << 30]C.int)(unsafe.Pointer(cMorpheme.exprPosTag))
			cExprSurface := (*[1 << 30]*C.char)(unsafe.Pointer(cMorpheme.exprSurface))

			for j := 0; j < expressionLength; j++ {
				tokens[i].Expression[j].POSTag = int(cExprPOS[j])
				tokens[i].Expression[j].Surface = C.GoString(cExprSurface[j])
			}
		}
	}

	return &tokens, nil
}

func (nt *NoriTokenizer) Free() {
	C.freeTokenizer(nt.dictionary, nt.tokenizer)
}
