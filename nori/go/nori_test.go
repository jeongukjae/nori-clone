package nori

import "testing"

func TestLoadNoriTokenizer(t *testing.T) {
	tokenizer, err := New("../../dictionary", "../../dictionary/userdict.txt")
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
	defer tokenizer.Free()
}

func TestTokenize(t *testing.T) {
	tokenizer, err := New("../../dictionary", "")
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
	defer tokenizer.Free()

	tokens, err := tokenizer.Tokenize("화학 이외의 것", true)
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
	if len(*tokens) != 6 {
		t.FailNow()
	}

	expectedTerms := []string{
		"BOS/EOS",
		"화학",
		"이외",
		"의",
		"것",
		"BOS/EOS",
	}

	for index, term := range expectedTerms {
		if (*tokens)[index].Surface != term {
			t.Errorf("[%d] term is not equal, \n expected: %s, actual: %s", index, term, (*tokens)[index].Surface)
		}
	}
}

func TestTokenizeWithExpr(t *testing.T) {
	tokenizer, err := New("../../dictionary", "")
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
	defer tokenizer.Free()

	_, err = tokenizer.Tokenize("Nori-clone은 c++로 Nori를 재작성하기 위한 프로젝트입니다.", true)
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
}
