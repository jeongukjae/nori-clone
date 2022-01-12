package nori

import "testing"

func TestLoadNoriTokenizer(t *testing.T) {
	tokenizer, err := New("../../dictionary", "../../dictionary/userdict.txt")
	if err != nil {
		t.Log(err)
		t.FailNow()
	}
	tokenizer.clear()
}
