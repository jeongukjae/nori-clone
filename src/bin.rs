use clap::{Args, Parser, Subcommand};
use env_logger::Env;
use log::{error, info};
use nori_clone::DictionaryBuilder;

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
}

#[derive(Args)]
struct BuildOptions {
    /// input directory path
    #[arg(short, long)]
    input_path: Option<String>,
    /// output directory path
    #[arg(short, long)]
    output_path: Option<String>,
    /// whether to normalize input token files
    #[arg(short, long)]
    normalize: Option<bool>,
}

fn main() {
    let cli = Cli::parse();
    env_logger::Builder::from_env(Env::default().default_filter_or("info")).init();

    match &cli.command {
        Commands::Build(opts) => {
            let input_path = match opts.input_path {
                Some(ref path) => path,
                None => {
                    error!("Input path is not specified.");
                    panic!();
                }
            };

            let output_path = match opts.output_path {
                Some(ref path) => path,
                None => {
                    error!("Output path is not specified.");
                    panic!();
                }
            };

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
    }
}
