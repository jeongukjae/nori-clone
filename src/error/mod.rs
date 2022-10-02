#[derive(Debug, PartialEq)]
pub struct Error(pub String);

impl Error {
    pub fn description(&self) -> &str {
        self.0.as_str()
    }
}

impl<T> From<T> for Error
where
    T: std::fmt::Display,
{
    fn from(e: T) -> Self {
        Self(format!("{}", e))
    }
}
