package main

import (
	"encoding/json"
	"io/ioutil"
)

type (
	ValueInfo struct {
		Name string `json:"name"`
		Unit string `json:"unit"`
	}
)

var (
	// ValueIDToKey is definition of string value key's integer id
	ValueIDToKey map[string]ValueInfo = map[string]ValueInfo{}
	Value        map[string]uint32    = map[string]uint32{}
)

func ReadValueData(path string) error {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return err
	}
	if err := json.Unmarshal(data, &ValueIDToKey); err != nil {
		return err
	}
	return nil
}
