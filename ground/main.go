package main

import (
	"flag"
	"net/http"

	elogrus "github.com/dictor/echologrus"
	ws "github.com/dictor/wswrapper"
	"github.com/jacobsa/go-serial/serial"
	"github.com/labstack/echo/v4"
	"github.com/labstack/gommon/log"
	"github.com/sirupsen/logrus"
)

var (
	globalLogger elogrus.EchoLogger
)

type (
	hamsterTongueMessage struct {
		Length  byte   `json:"length"` // byte count of Verb~CRC
		Verb    byte   `json:"verb"`
		Noun    byte   `json:"noun"`
		Payload []byte `json:"payload"`
		CRC     byte   `json:"crc"` // CRC-8 of Verb ~ Payload
	}
)

func main() {
	e := echo.New()
	globalLogger = elogrus.Attach(e)
	globalLogger.SetFormatter(&logrus.TextFormatter{
		ForceColors: true,
	})

	var (
		serialPort, webListenAddress, valueConfig string
		serialBaud                                int
		debug                                     bool
	)
	flag.StringVar(&serialPort, "sport", "", "path of serial port")
	flag.IntVar(&serialBaud, "sbaud", 115200, "baudrate of serial port")
	flag.StringVar(&webListenAddress, "waddr", ":80", "web server listen address")
	flag.StringVar(&valueConfig, "vconf", "value.json", "path of value key-id config file")
	flag.BoolVar(&debug, "debug", false, "enable debug log output")
	flag.Parse()

	if debug {
		globalLogger.SetLevel(log.DEBUG)
		globalLogger.Debugln("debug log enabled")
	}

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
	wsSendQueue := make(chan []byte, 32)
	go listenPort(port, 256, msgResult)
	go decodeMessage(msgResult, wsSendQueue)

	wshub := ws.NewHub()
	go wshub.Run(wsEvent)

	go broadcastData(wshub, wsSendQueue, 200)

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
	e.File("/protocol", "ui/protocol.js")
	globalLogger.Panic(e.Start(webListenAddress))
}
