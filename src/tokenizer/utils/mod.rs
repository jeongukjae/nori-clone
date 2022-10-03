pub mod uchar {
    use crate::error::Error;

    pub fn get_next_length(c: u8) -> usize {
        if c < 0x80 {
            1
        } else if c < 0xE0 {
            2
        } else if c < 0xF0 {
            3
        } else {
            4
        }
    }

    pub fn get_next_char(c: &[u8], offset: usize) -> Result<char, Error> {
        let length = get_next_length(c[offset]);
        let chstr = match std::str::from_utf8(&c[offset..offset + length]) {
            Ok(s) => s,
            Err(e) => return Err(format!("Failed to convert to utf8: {:?}", e).into()),
        };

        Ok(chstr.chars().next().unwrap())
    }

    // returns (is_hangul, has_coda)
    pub fn check_coda(c: &str) -> (bool, bool) {
        let chars: Vec<char> = c.chars().collect();
        if chars.is_empty() {
            return (false, false);
        }

        let last_ch = chars.last().unwrap();
        let last_ch_code = *last_ch as u32;
        if !(0xAC00..=0xD7A3).contains(&last_ch_code) {
            return (false, false);
        }

        if ((last_ch_code - 0xAC00) % 28) == 0 {
            (true, false)
        } else {
            (true, true)
        }
    }
}

#[cfg(test)]
mod test {
    use super::uchar;

    #[test]
    fn test_get_next_unicode_char_length() {
        assert_eq!(uchar::get_next_length("$".as_bytes()[0]), 1);
        assert_eq!(uchar::get_next_length("£".as_bytes()[0]), 2);
        assert_eq!(uchar::get_next_length("ह".as_bytes()[0]), 3);
        assert_eq!(uchar::get_next_length("€".as_bytes()[0]), 3);
        assert_eq!(uchar::get_next_length("한".as_bytes()[0]), 3);
        assert_eq!(uchar::get_next_length("𐍈".as_bytes()[0]), 4);
    }

    #[test]
    fn test_get_next_unicode_char() {
        assert_eq!(uchar::get_next_char("$".as_bytes(), 0), Ok('$'));
        assert_eq!(uchar::get_next_char("£".as_bytes(), 0), Ok('£'));
        assert_eq!(uchar::get_next_char("ह".as_bytes(), 0), Ok('ह'));
        assert_eq!(uchar::get_next_char("€".as_bytes(), 0), Ok('€'));
        assert_eq!(uchar::get_next_char("한".as_bytes(), 0), Ok('한'));
        assert_eq!(uchar::get_next_char("𐍈".as_bytes(), 0), Ok('𐍈'));
    }
}
