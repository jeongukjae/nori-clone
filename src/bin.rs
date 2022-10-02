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

            match DictionaryBuilder::new(opts.normalize).build(&input_path, &output_path) {
                Ok(_) => info!("Dictionary built successfully."),
                Err(e) => {
                    error!("Failed to build dictionary: {}", e.description());
                    panic!();
                }
            }
        }
        Commands::Tokenize(opts) => {
            info!("Reading dictionary...");
            let system_dictionary =
                match SystemDictionary::load_from_input_directory(opts.dictionary_path.as_str()) {
                    Ok(d) => d,
                    Err(e) => {
                        error!("Failed to load dictionary: {}", e.description());
                        panic!();
                    }
                };

            let user_dictionary = UserDictionary {};

            info!("Constructing tokenizer...");
            let tokenizer = NoriTokenizer::new(system_dictionary, user_dictionary);

            match tokenizer.tokenize(opts.text.as_str()) {
                Ok(tokens) => {
                    info!("Results: {:#?}", tokens);
                }
                Err(e) => {
                    error!("Failed to tokenize text: {}", e.description());
                    panic!();
                }
            };
        }
    }
}
