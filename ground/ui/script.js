var HamstroneApp = {
    app: {},
    ws: {},
    init: function() {
        this.app = new Vue({
            el: '#app',
            data: {
                menus: [
                    { text: "Value", page: "value" },
                    { text: "Signal", page: "signal" },
                ],
                values: [],
                valueKeys: {},
                signals: [],
                currentPage: "value",
            },
            methods: {
                onMenuClick: function(m) {
                    this.currentPage = m.page;
                },
                findSignalNounString: function(noun) {
                    for (n in HamsterTongue.Noun.Signal) {
                        if (HamsterTongue.Noun.Signal[n] === noun) return n;
                    }
                    return noun;
                },
                findValueInfoString: function(key, type) {
                    if (this.valueKeys[key]) {
                        switch (type) {
                            case "name":
                                return this.valueKeys[key].name;
                            case "unit":
                                return this.valueKeys[key].unit;
                        }
                    } else {
                        switch (type) {
                            case "name":
                                return key;
                            case "unit":
                                return "";
                        }
                    }
                }
            },
            mounted: async function() {
                let resp = await axios.get("./definition/value");
                HamstroneApp.app.valueKeys = resp.data;
            },
        });
        this.ws = new WebSocket((location.protocol == "https:" ? "wss:" : "ws:") + "//" + document.location.host + "/ws");
        this.ws.onmessage = function(event) {
            let data = JSON.parse(event.data);
            switch (data.type) {
                case "value":
                    HamstroneApp.app.values.splice(0);
                    for (d in data.data) {
                        HamstroneApp.app.values.push({ name: d, value: data.data[d] });
                    }
                    break;
                case "signal":
                    HamstroneApp.app.signals.push({ time: new Date().toLocaleString(), noun: data.data.noun, payload: data.data.payload });
                    break;
            }
        }
    },
}