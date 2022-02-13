package main

import (
	"bufio"
	"log"
	"os"
	"time"

	nori "github.com/jeongukjae/nori-clone/nori/go"
)

func main() {
	tokenizer, err := nori.New("./dictionary/latest-dictionary.nori", "./dictionary/latest-userdict.txt")

	if err != nil {
		log.Fatal(err)
	}
	defer tokenizer.Free()

	file, err := os.Open("tools/benchmark/data.txt")
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var lines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	start := time.Now()
	for _, line := range lines {
		tokenizer.Tokenize(line)
	}
	elapsed := time.Since(start)
	log.Printf("Elapsed %s", elapsed)
}
