(function () {
	"use strict";

	const MQTT_WS_URL = "wss://broker.emqx.io:8084/mqtt";
	const TOPIC_TELEMETRY = "finca/fatima/telemetria";
	const TOPIC_STATUS = "finca/fatima/estado";

	/** @type {import("mqtt").MqttClient | null} */
	let client = null;

	/** @type {{ temp: number; hum_aire: number; hum_suelo: number; bomba: number; ventilador: number; wifi_rssi: number } | null} */
	let lastTelemetry = null;

	/** @type {string | null} */
	let deviceStatus = null;

	/** @type {number | null} */
	let lastTelemetryAt = null;

	function $(id) {
		const el = document.getElementById(id);
		if (!el) throw new Error("Missing element: " + id);
		return el;
	}

	function setConnState(kind, label) {
		const dot = $("conn-dot");
		dot.classList.remove("ok", "warn", "err");
		dot.classList.add(kind);
		$("conn-label").textContent = label;
	}

	function setText(id, text) {
		$(id).textContent = text;
	}

	function formatTime(ts) {
		if (ts == null) return "—";
		return new Date(ts).toLocaleString(undefined, {
			hour: "2-digit",
			minute: "2-digit",
			second: "2-digit",
		});
	}

	function onOff(v) {
		const n = Number(v);
		return n === 1 || n === true ? "On" : "Off";
	}

	function render() {
		if (lastTelemetry) {
			setText("val-temp", lastTelemetry.temp.toFixed(1));
			setText("val-hum-aire", lastTelemetry.hum_aire.toFixed(1));
			setText("val-hum-suelo", lastTelemetry.hum_suelo.toFixed(1));
			setText("val-rssi", String(lastTelemetry.wifi_rssi));
			setText("val-pump", onOff(lastTelemetry.bomba));
			setText("val-fan", onOff(lastTelemetry.ventilador));
		}
		setText("val-device", deviceStatus ?? "—");
		setText("val-updated", formatTime(lastTelemetryAt));
	}

	/**
	 * @param {string} topic
	 * @param {string} payload
	 */
	function handleMessage(topic, payload) {
		if (topic === TOPIC_STATUS) {
			deviceStatus = payload.trim();
			render();
			return;
		}
		if (topic !== TOPIC_TELEMETRY) return;

		try {
			const data = JSON.parse(payload);
			const temp = Number(data.temp);
			const humAire = Number(data.hum_aire);
			const humSuelo = Number(data.hum_suelo);
			const bomba = Number(data.bomba);
			const ventilador = Number(data.ventilador);
			const wifiRssi = Number(data.wifi_rssi);

			if (
				Number.isFinite(temp) &&
				Number.isFinite(humAire) &&
				Number.isFinite(humSuelo) &&
				Number.isFinite(bomba) &&
				Number.isFinite(ventilador) &&
				Number.isFinite(wifiRssi)
			) {
				lastTelemetry = {
					temp,
					hum_aire: humAire,
					hum_suelo: humSuelo,
					bomba,
					ventilador,
					wifi_rssi: wifiRssi,
				};
				lastTelemetryAt = Date.now();
				render();
			}
		} catch {
			/* ignore malformed JSON */
		}
	}

	function connect() {
		if (typeof mqtt === "undefined") {
			setConnState("err", "MQTT library failed to load");
			return;
		}

		setConnState("warn", "Connecting…");

		const clientId = "fatima-dash-" + Math.random().toString(16).slice(2, 10);
		client = mqtt.connect(MQTT_WS_URL, {
			clientId,
			clean: true,
			reconnectPeriod: 3000,
		});

		client.on("connect", () => {
			setConnState("ok", "Connected");
			client.subscribe([TOPIC_TELEMETRY, TOPIC_STATUS], (err) => {
				if (err) setConnState("err", "Subscribe error");
			});
		});

		client.on("reconnect", () => {
			setConnState("warn", "Reconnecting…");
		});

		client.on("close", () => {
			if (client && !client.connected) setConnState("warn", "Disconnected");
		});

		client.on("error", () => {
			setConnState("err", "Connection error");
		});

		client.on("message", (topic, buf) => {
			handleMessage(topic, buf.toString());
		});
	}

	if (document.readyState === "loading") {
		document.addEventListener("DOMContentLoaded", connect);
	} else {
		connect();
	}
})();
