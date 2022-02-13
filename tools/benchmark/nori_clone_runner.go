package main

import (
	"bufio"
	"log"
	"os"
	"strconv"
	"time"

	nori "github.com/jeongukjae/nori-clone/nori/go"
)

func main() {
	tokenizer, err := nori.New("./dictionary/latest-dictionary.nori", "./dictionary/latest-userdict.txt")

	if err != nil {
		log.Fatal(err)
	}
	defer tokenizer.Free()

	file, err := os.Open(os.Args[1])
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var lines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	n, err := strconv.Atoi(os.Args[2])
	if err != nil {
		log.Fatal(err)
	}
	lines = lines[:n]

	start := time.Now()
	for _, line := range lines {
		tokenizer.Tokenize(line)
	}
	elapsed := time.Since(start)
	log.Printf("Elapsed %s", elapsed)
}
