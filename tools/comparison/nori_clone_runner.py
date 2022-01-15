import nori

dictionary = nori.Dictionary()
dictionary.load_prebuilt_dictionary("./dictionary/legacy")
dictionary.load_user_dictionary("./dictionary/legacy/userdict.txt")
tokenizer = nori.NoriTokenizer(dictionary)

result = tokenizer.tokenize("2018 평창 동계 올림픽")
print(result)

for token in result.tokens:
    print(token.surface, token.postag, token.postype)
