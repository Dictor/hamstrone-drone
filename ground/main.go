package main

import (
	"flag"
	"net/http"

	elogrus "github.com/dictor/echologrus"
	ws "github.com/dictor/wswrapper"
	"github.com/jacobsa/go-serial/serial"
	"github.com/labstack/echo/v4"
)

var (
	globalLogger elogrus.EchoLogger
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
	e := echo.New()
	globalLogger = elogrus.Attach(e)

	var (
		serialPort, webListenAddress, valueConfig string
		serialBaud                                int
	)
	flag.StringVar(&serialPort, "sport", "", "path of serial port")
	flag.IntVar(&serialBaud, "sbaud", 115200, "baudrate of serial port")
	flag.StringVar(&webListenAddress, "waddr", ":80", "web server listen address")
	flag.StringVar(&valueConfig, "vconf", "value.json", "path of value key-id config file")
	flag.Parse()

	if err := ReadValueData(valueConfig); err != nil {
		globalLogger.WithField("error", err).Panicln("fail to read value config file")
	}

	options := serial.OpenOptions{
		PortName:        serialPort,
		BaudRate:        uint(serialBaud),
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 1,
	}
	port, err := serial.Open(options)
	if err != nil {
		globalLogger.WithField("error", err).Panicln("fail to open serial port")
	}
	defer port.Close()
	globalLogger.Infoln("serial port opened")

	msgResult := make(chan *hamsterTongueMessage)
	go listenPort(port, 256, msgResult)
	go decodeMessage(msgResult)

	wshub := ws.NewHub()
	go wshub.Run(wsEvent)
	go broadcastValue(wshub, 200)

	e.GET("/ws", func(c echo.Context) error {
		wshub.AddClient(c.Response().Writer, c.Request())
		return nil
	})
	e.GET("/definition/value", func(c echo.Context) error {
		return c.JSON(http.StatusOK, ValueIDToKey)
	})
	e.File("/", "ui/index.html")
	e.File("/style", "ui/style.css")
	e.File("/script", "ui/script.js")
	globalLogger.Panic(e.Start(webListenAddress))
}
