package main

import (
	"flag"
	"io"

	"github.com/jacobsa/go-serial/serial"
	logrus "github.com/sirupsen/logrus"
)

var (
	globalLogger *logrus.Logger
)

type (
	hamsterTongueMessage struct {
		Length  byte // byte count of Verb~CRC
		Verb    byte
		Noun    byte
		Payload []byte
		CRC     byte // CRC-8 of Verb ~ Payload
	}
)

func main() {
	globalLogger = logrus.New()

	var (
		serialPort string
		serialBaud int
	)
	flag.StringVar(&serialPort, "sport", "", "path of serial port")
	flag.IntVar(&serialBaud, "sbaud", 115200, "baudrate of serial port")
	flag.Parse()

	options := serial.OpenOptions{
		PortName:        serialPort,
		BaudRate:        uint(serialBaud),
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 1,
	}
	port, err := serial.Open(options)
	if err != nil {
		globalLogger.WithField("error", err).Panicln("fail to open serial port : %s")
	}
	defer port.Close()

	msgResult := make(chan *hamsterTongueMessage)
	go listenPort(port, 256, msgResult)
	go func() {
		for {
			select {
			case msg := <-msgResult:
				globalLogger.Info(msg)
			}
		}
	}()
}

func listenPort(stream io.ReadWriteCloser, bufferSize int, result chan *hamsterTongueMessage) {
	buffer := make([]byte, bufferSize)
	var (
		markerFound bool
		appendCount byte
		message     hamsterTongueMessage = hamsterTongueMessage{Payload: []byte{}}
	)
	for {
		readCount, err := stream.Read(buffer)
		if readCount > 0 {
			for _, b := range buffer[0:readCount] {
				if markerFound {
					switch appendCount {
					case 0:
						message.Length = b
					case 1:
						message.Verb = b
					case 2:
						message.Noun = b
					default:
						if appendCount < message.Length {
							message.Payload = append(message.Payload, b)
						} else {
							message.CRC = b
							result <- &message
							message = hamsterTongueMessage{Payload: []byte{}}
						}
					}
					appendCount++
				} else {
					if b == 0xFF {
						markerFound = true
					}
				}
			}
		}
		if err != nil {
			globalLogger.WithField("error", err).Errorln("encounter error while read serial port")
			break
		}
	}
}
