var HamstroneApp = {
    app: {},
    ws: {},
    init: function() {
        this.app = new Vue({
            el: '#app',
            data: {
                values: [],
                valuekeys: {},
            },
            methods: {},
            mounted: async function() {
                let resp = await axios.get("./definition/value");
                HamstroneApp.app.valuekeys = resp.data;
            },
        });
        this.ws = new WebSocket((location.protocol == "https:" ? "wss:" : "ws:") + "//" + document.location.host + "/ws");
        this.ws.onmessage = function(event) {
            let data = JSON.parse(event.data);
            HamstroneApp.app.values.splice(0);
            for (d in data) {
                HamstroneApp.app.values.push({ name: d, value: data[d] });
            }
        }
    },
}