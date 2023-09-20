use std::io::BufRead;
use std::io::Write;
use std::{fs::File, io::BufReader, time::Instant};

use clap::{Args, Parser, Subcommand};
use env_logger::Env;
use log::{error, info};
use nori_clone::*;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
#[command(propagate_version = true)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Build dictionary from MeCab dictionary.
    Build(BuildOptions),

    /// Tokenize text with dictionary.
    Tokenize(TokenizeOptions),

    /// Tokenize text in file with dictionary.
    TokenizeFile(TokenizeFileOptions),
}

#[derive(Args)]
struct BuildOptions {
    /// input directory path
    #[arg(short, long)]
    input_path: String,

    /// output directory path
    #[arg(short, long)]
    output_path: String,

    /// whether to normalize input token files
    #[arg(short, long)]
    normalize: Option<bool>,
}

#[derive(Args)]
struct TokenizeOptions {
    /// input directory path
    #[arg(short, long)]
    dictionary_path: String,

    /// text to tokenize
    #[arg(short, long)]
    text: String,

    /// Graphviz output filepath, optional
    #[arg(short, long)]
    graph_out: Option<String>,
}

#[derive(Args)]
struct TokenizeFileOptions {
    /// Dict path
    #[arg(short, long)]
    dictionary_path: String,

    /// data path
    #[arg(long)]
    data: String,

    /// n lines
    #[arg(short, long, default_value_t = 1000)]
    n_lines: usize,
}

fn main() {
    let cli = Cli::parse();
    env_logger::Builder::from_env(Env::default().default_filter_or("info")).init();

    match &cli.command {
        Commands::Build(opts) => {
            let input_path = opts.input_path.as_str();
            let output_path = opts.output_path.as_str();

            info!("Building dictionary from MeCab dictionary...");
            info!(" - Input path: {}", input_path);
            info!(" - Output path: {}", output_path);
            info!(" - Normalize: {}", opts.normalize.unwrap_or(false));

            match DictionaryBuilder::new(opts.normalize).build(input_path, output_path) {
                Ok(_) => info!("Dictionary built successfully."),
                Err(e) => {
                    error!("Failed to build dictionary: {}", e.description());
                    panic!();
                }
            }
        }
        Commands::Tokenize(opts) => {
            info!("Reading dictionary...");

            let load_start = Instant::now();
            let system_dictionary =
                match SystemDictionary::load_from_input_directory(opts.dictionary_path.as_str()) {
                    Ok(d) => d,
                    Err(e) => {
                        error!("Failed to load dictionary: {}", e.description());
                        panic!();
                    }
                };

            let user_dictionary =
                match UserDictionary::load(vec![], &system_dictionary.connection_cost.additional) {
                    Ok(d) => d,
                    Err(e) => {
                        error!("Failed to load dictionary: {}", e.description());
                        panic!();
                    }
                };

            info!("Constructing tokenizer...");
            let tokenizer = NoriTokenizer::new(system_dictionary, user_dictionary);
            info!("Time elapsed in nori tokenizer is: {:?}", load_start.elapsed());

            info!("Tokenizing...");
            let mut graphviz = opts.graph_out.as_ref().map(|_| GraphViz::new());

            match tokenizer.tokenize_and_visualize(opts.text.as_str(), &mut graphviz) {
                Ok(tokens) => {
                    info!("Results: {:#?}", tokens);

                    if let Some(graph_out) = &opts.graph_out {
                        info!("Writing graphviz to {}...", graph_out);
                        let mut file = File::create(graph_out).unwrap();

                        let content = graphviz.unwrap().to_dot();
                        file.write_all(content.as_bytes())
                            .expect("Failed to write graphviz");
                    }
                }
                Err(e) => {
                    error!("Failed to tokenize text: {}", e.description());
                    panic!();
                }
            };
        }
        Commands::TokenizeFile(opts) => {
            info!("Reading dictionary...");

            let load_start = Instant::now();
            let system_dictionary =
                match SystemDictionary::load_from_input_directory(opts.dictionary_path.as_str()) {
                    Ok(d) => d,
                    Err(e) => {
                        error!("Failed to load dictionary: {}", e.description());
                        panic!();
                    }
                };

            let user_dictionary =
                match UserDictionary::load(vec![], &system_dictionary.connection_cost.additional) {
                    Ok(d) => d,
                    Err(e) => {
                        error!("Failed to load dictionary: {}", e.description());
                        panic!();
                    }
                };

            info!("Constructing tokenizer...");
            let tokenizer = NoriTokenizer::new(system_dictionary, user_dictionary);
            info!("Time elapsed in nori tokenizer is: {:?}", load_start.elapsed());

            // read all contents from args.data path.
            let file = File::open(opts.data.as_str()).unwrap();
            let lines: Vec<String> = BufReader::new(file)
                .lines()
                .map(|line| line.unwrap())
                .collect();
            let lines = lines[..opts.n_lines].to_vec();

            info!(
                "Tokenize {} lines in files and check the elapsed time",
                lines.len()
            );
            let start = Instant::now();
            for line in lines {
                let _ = tokenizer.tokenize(line.as_str());
            }
            let duration = start.elapsed();
            info!("Time elapsed in nori tokenizer is: {:?}", duration);
        }
    }
}
