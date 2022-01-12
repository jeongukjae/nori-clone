package nori

/*
#include <stdlib.h>
#include <nori/go/wrapper.h>
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

func (nt *NoriTokenizer) clear() {
	C.freeTokenizer(nt.dictionary, nt.tokenizer)
}
