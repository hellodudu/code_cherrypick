package main

import (
	"bufio"
	"encoding/json"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
	"sync"
)

type config struct {
	Filters []string `json:"filters"`
	MinSize int      `json:"min_size"`
	MinLine int      `json:"min_line"`
	Path    []string `json:"path"`
	Prefix  []string `json:"prefix"`
}

var cfg config

func main() {

	// read config file
	data, err := ioutil.ReadFile("config.json")
	if err != nil {
		log.Fatal(err)
	}

	err = json.Unmarshal(data, &cfg)
	if err != nil {
		log.Fatal("unmarshal config.json error:", err)
	}

	// all file path
	allFiles := make([]string, 0)
	chFile := make(chan struct{}, 1)
	go func() {
		for _, p := range cfg.Path {
			if err := filepath.Walk(p, func(path string, info os.FileInfo, err error) error {
				ext := filepath.Ext(info.Name())
				for _, filter := range cfg.Filters {
					if ext == filter && info.Size() >= int64(cfg.MinSize) {
						allFiles = append(allFiles, path)
						break
					}
				}

				return nil
			}); err != nil {
				log.Fatal(err)
			}
		}

		chFile <- struct{}{}
	}()
	<-chFile

	var wg sync.WaitGroup
	var mu sync.Mutex
	finalCodes := make([]string, 0)
	for _, name := range allFiles {
		wg.Add(1)
		go func(n string) {
			defer wg.Done()

			codes := readFile(n)

			mu.Lock()
			finalCodes = append(finalCodes, codes...)
			mu.Unlock()
		}(name)
	}

	wg.Wait()

	// write to file
	ioutil.WriteFile("out.txt", []byte(strings.Join(finalCodes, "")), 0666)

	// server exit
	// c := make(chan os.Signal, 1)
	// signal.Notify(c, syscall.SIGHUP, syscall.SIGQUIT, syscall.SIGTERM, syscall.SIGINT)
	// for {
	// 	sig := <-c
	// 	fmt.Println("closing down (signal: %v)", sig)

	// 	switch sig {
	// 	case syscall.SIGQUIT, syscall.SIGTERM, syscall.SIGSTOP, syscall.SIGINT:
	// 		fmt.Println("exit safely")
	// 		return
	// 	case syscall.SIGHUP:
	// 	default:
	// 		return
	// 	}
	// }
	os.Exit(0)
}

func readFile(name string) []string {
	retCodes := make([]string, 0)

	file, err := os.Open(name)
	defer file.Close()

	if err != nil {
		log.Fatal(err)
	}

	// Start reading from 1/4 size pos.
	reader := bufio.NewReader(file)
	_, err = reader.Discard(reader.Size() / 4)
	if err == io.EOF {
		return retCodes
	}

	// discarding to the first end of comment
	for {
		line, err := reader.ReadString('\n')
		if err == io.EOF {
			break
		}

		if err != nil {
			log.Fatal(err)
		}

		bHit := false
		for _, v := range cfg.Prefix {
			if strings.HasPrefix(line, v) {
				bHit = true
				break
			}
		}

		if bHit {
			break
		}
	}

	// read at most 50 lines or to the end of file
	var line string
	num := 0
	for {
		if num >= cfg.MinLine {
			break
		}

		if line, err = reader.ReadString('\n'); err != nil {
			break
		}

		retCodes = append(retCodes, line)
		num++
	}

	return retCodes
}
