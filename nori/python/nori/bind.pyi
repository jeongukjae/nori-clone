from typing import List, Tuple

class Token:
    surface: str
    offset: int
    length: int
    leftid: int
    rightid: int
    postag: List[str]
    postype: str
    expr: List[Tuple[str, str]]
    def __repr__(self) -> str: ...

class Lattice:
    tokens: List[Token]
    sentence: str

class Dictionary:
    def __init__(self): ...
    def load_prebuilt_dictionary(self, filename: str): ...
    def load_user_dictionary(self, filename: str): ...

class NoriTokenizer:
    def __init__(self, dictionary: Dictionary): ...
    def tokenize(self, input: str) -> Lattice: ...
