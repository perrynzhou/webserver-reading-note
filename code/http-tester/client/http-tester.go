package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"os/signal"
	"sync"
	"sync/atomic"
	"syscall"
	"time"

	log "github.com/sirupsen/logrus"
)

var (
	requestUrl        = flag.String("u", "127.0.0.1:80", "remote request url")
	nGoroutines       = flag.Int("n", 4, "goroutine for request")
	tickerMillisecond = flag.Int("t", 100, "millisecond for time ticker")
)

type Metric struct {
	Count uint64
	Index int
}

func main() {
	flag.Parse()
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, os.Interrupt, syscall.SIGTERM, syscall.SIGINT)
	ticker := time.NewTicker(time.Duration(*tickerMillisecond) * time.Millisecond)
	url := fmt.Sprintf("http://%s", *requestUrl)
	log.Infof("remote request url on %s\n", url)
	log.Infof("init %d goroutines,ticker is %d millseconds", *nGoroutines, *tickerMillisecond)
	counter := make([]uint64, *nGoroutines)
	wg := &sync.WaitGroup{}
	//seconds
	var duration float64
	//total execute times
	var count uint64
	for {
		select {
		case <-signals:
			ticker.Stop()
			log.Infof("total handle request:%d, consumed %f seconds,tps:%f\n", count, duration, (float64)(count)/duration)
			log.Info("stop http-tester")
			return
		case <-ticker.C:
			for i := 0; i < *nGoroutines; i++ {
				count += counter[i]
			}
			wg.Add(*nGoroutines)
			start := time.Now()
			for i := 0; i < *nGoroutines; i++ {
				go func(url string, count *uint64, wg *sync.WaitGroup) {
					defer wg.Done()
					resp, err := http.Get(url)
					if err != nil {
						log.Error(err)
					}
					defer resp.Body.Close()
					if _, err := ioutil.ReadAll(resp.Body); err != nil {
						log.Error(err)
					}
					atomic.AddUint64(count, 1)
				}(url, &counter[i], wg)
			}
			wg.Wait()
			end := time.Since(start).Seconds()
			duration += end
			log.Info(time.Now().Format("2006-02-01 15:04:05.000"), "start  ticker consumed ", end, "  handle requests ", count)

		}
	}
}
