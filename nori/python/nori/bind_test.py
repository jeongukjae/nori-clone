import unittest

from nori.bind import NoriTokenizer

class TestNoriTokenizer(unittest.TestCase):
    def test_get_dictionary_info(self):
        tokenizer = NoriTokenizer()
        tokenizer.load_prebuilt_dictionary("./dictionary/latest-dictionary.nori")

        result = tokenizer.tokenize("화학 이외의 것")

        self.assertEqual(result.sentence, "화학 이외의 것")
        self.assertEqual(result.tokens[0].surface, "BOS/EOS")
        self.assertEqual([token.surface for token in result.tokens[1:-1]], ['화학', '이외', '의', '것'])
        self.assertEqual([token.postype for token in result.tokens[1:-1]], ['MORPHEME', 'MORPHEME', 'MORPHEME', 'MORPHEME'])

    def test_get_expressions(self):
        tokenizer = NoriTokenizer()
        tokenizer.load_prebuilt_dictionary("./dictionary/latest-dictionary.nori")

        result = tokenizer.tokenize("붕어빵")
        self.assertEqual(len(result.tokens), 3)
        self.assertEqual(len(result.tokens[1].expr), 2)
        self.assertEqual(result.tokens[1].expr, [('붕어', 'NNG'), ('빵', 'NNG')])


if __name__ == "__main__":
    unittest.main()
