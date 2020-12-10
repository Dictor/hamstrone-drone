package main

import (
	"encoding/json"
	"time"

	ws "github.com/dictor/wswrapper"
	"github.com/sirupsen/logrus"
)

func wsEvent(evt *ws.WebsocketEvent) {
	switch evt.Kind {
	case ws.EVENT_RECIEVE:
		//when some message received.
	case ws.EVENT_REGISTER:
		globalLogger.WithFields(logrus.Fields{
			"id":             evt.Client.Id(),
			"remote_address": evt.Client.Connection().RemoteAddr().String(),
			"local_address":  evt.Client.Connection().LocalAddr().String(),
		}).Info("websocker client registered")
	case ws.EVENT_UNREGISTER:
		globalLogger.WithFields(logrus.Fields{
			"id":             evt.Client.Id(),
			"remote_address": evt.Client.Connection().RemoteAddr().String(),
			"local_address":  evt.Client.Connection().LocalAddr().String(),
		}).Info("websocker client unregistered")
	case ws.EVENT_ERROR:
		globalLogger.WithField("error", evt.Err).Errorln("websocket error")
	}
}

func broadcastValue(h *ws.WebsocketHub, interval int) {
	for {
		data, err := json.Marshal(&Value)
		if err != nil {
			globalLogger.WithField("error", err).Errorln("error caused while value broadcast")
		}
		h.SendAll(data)
		time.Sleep(time.Millisecond * time.Duration(interval))
	}
}
