pub mod uchar {
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
