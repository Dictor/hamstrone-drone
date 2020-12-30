package hamstertongue

const (
	/* MESSAGE STRUCTURE */
	MessageMarker = 0xFF

	/* MESSAGE VERB */
	MessageVerbHeartbeat = 0x00
	MessageVerbValue     = 0x01
	MessageVerbSignal    = 0x02

	/* MESSAGE NOUN */
	MessageNounSignalInitOK   = 0x00
	MessageNounSignalInitFail = 0x01
)
