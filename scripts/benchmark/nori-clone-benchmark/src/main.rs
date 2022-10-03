use clap::Parser;
use std::{io::{BufReader, BufRead}, fs::File, time::Instant};
use nori_clone::{NoriTokenizer, SystemDictionary, UserDictionary};

/// Run nori-clone benchmark
#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Dict path
    #[arg(short, long)]
    dict: String,

    /// data path
    #[arg(long)]
    data: String,

    /// n lines
    #[arg(short, long, default_value_t = 100)]
    n_lines: usize,
}

fn main() {
    let args = Args::parse();

    let system_dictionary = SystemDictionary::load_from_input_directory(args.dict.as_str()).unwrap();
    let user_dictionary = UserDictionary::load_from_input_directory(args.dict.as_str(), &system_dictionary.connection_cost.additional).unwrap();

    let tokenizer = NoriTokenizer::new(system_dictionary, user_dictionary);

    // read all contents from args.data path.
    let file = File::open(args.data).unwrap();
    let lines: Vec<String> = BufReader::new(file).lines().map(|line| line.unwrap()).collect();
    let lines = lines[..args.n_lines].to_vec();

    println!("{} lines", lines.len());
    let start = Instant::now();
    for line in lines {
        let _ = tokenizer.tokenize(line.as_str());
    }
    let duration = start.elapsed();

    println!("Time elapsed in nori tokenizer is: {:?}", duration);
}
