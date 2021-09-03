import paho.mqtt.publish as publish

publish.single("ku/mqtt/test", "payload", hostname="test.mosquitto.org")
