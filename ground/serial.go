package main

import (
	"encoding/binary"
	"encoding/json"
	"io"
	"strconv"

	"github.com/dictor/hamstrone_ground/hamstertongue"
	"github.com/sirupsen/logrus"
)

func listenPort(stream io.ReadWriteCloser, bufferSize int, result chan *hamsterTongueMessage) {
	buffer := make([]byte, bufferSize)
	var (
		markerFound bool
		appendCount byte
		message     *hamsterTongueMessage = &hamsterTongueMessage{Payload: []byte{}}
	)
	for {
		readCount, err := stream.Read(buffer)
		if readCount > 0 {
			for _, b := range buffer[0:readCount] {
				if markerFound {
					appendCount++
					switch appendCount {
					case 1:
						message.Length = b
					case 2:
						message.Verb = b
					case 3:
						message.Noun = b
					default:
						if appendCount <= message.Length {
							message.Payload = append(message.Payload, b)
						} else {
							message.CRC = b
							result <- message
							message = &hamsterTongueMessage{Payload: []byte{}}
							markerFound = false
							appendCount = 0
						}
					}
				} else {
					if b == hamstertongue.MessageMarker {
						markerFound = true
					} else {
						globalLogger.Debugf("unknown packet recieved : %d\n", b)
					}
				}
			}
		}
		if err != nil {
			globalLogger.WithField("error", err).Errorln("encounter error while read serial port")
			return
		}
	}
}

func decodeMessage(msgchan chan *hamsterTongueMessage, sendchan chan []byte) {
	for {
		select {
		case msg := <-msgchan:
			globalLogger.WithFields(logrus.Fields{
				"length":  msg.Length,
				"verb":    msg.Verb,
				"noun":    msg.Noun,
				"payload": msg.Payload,
			}).Debugf("serial message income")
			switch msg.Verb {
			case hamstertongue.MessageVerbHeartbeat:
			case hamstertongue.MessageVerbValue:
				Value[strconv.Itoa(int(msg.Noun))] = binary.LittleEndian.Uint32(addArrayPadding(msg.Payload, 4))
			case hamstertongue.MessageVerbSignal:
				data, err := json.Marshal(generalMessage{
					Type: "signal",
					Data: msg,
				})
				if err != nil {
					globalLogger.WithField("error", err).Errorln("error caused while making message")
					continue
				}
				sendchan <- data
			}
		}
	}
}

func addArrayPadding(b []byte, minimumLength int) []byte {
	if len(b) >= minimumLength {
		return b
	}
	nb := make([]byte, minimumLength)
	for i := 0; i < minimumLength; i++ {
		if i < len(b) {
			nb[i] = b[i]
		} else {
			nb[i] = 0
		}
	}
	return nb
}
