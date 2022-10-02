use fst::Automaton;

#[derive(Clone, Debug)]
pub struct CommonPrefix<'a> {
    string: &'a [u8],
    offset: usize,
}

impl<'a> CommonPrefix<'a> {
    /// Constructs automaton for common prefix search
    #[inline]
    pub fn new(input: &'a str, offset: usize) -> CommonPrefix<'a> {
        CommonPrefix {
            string: input.as_bytes(),
            offset: offset,
        }
    }
}

impl<'a> Automaton for CommonPrefix<'a> {
    type State = Option<usize>;

    #[inline]
    fn start(&self) -> Option<usize> {
        Some(0)
    }

    #[inline]
    fn is_match(&self, pos: &Option<usize>) -> bool {
        *pos > Some(0)
    }

    #[inline]
    fn can_match(&self, pos: &Option<usize>) -> bool {
        pos.is_some()
    }

    #[inline]
    fn accept(&self, pos: &Option<usize>, byte: u8) -> Option<usize> {
        // if we aren't already past the end...
        if let Some(pos) = *pos {
            // and there is still a matching byte at the current position...
            if self.string.get(pos + self.offset).cloned() == Some(byte) {
                // then move forward
                return Some(pos + 1);
            }
        }
        // otherwise we're either past the end or didn't match the byte
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use fst::IntoStreamer;
    use fst::Set;

    #[test]
    fn test_common_prefix_search() {
        let paths = vec!["bar", "foo", "he", "hello"];
        let set = Set::from_iter(paths).unwrap();

        // Build our fuzzy query.
        let pref = CommonPrefix::new("hello world", 0);

        // Apply our fuzzy query to the set we built.
        let stream = set.search(pref).into_stream();
        let matches = stream.into_strs().unwrap();

        assert_eq!(matches, vec!["he", "hello"]);
    }
}
