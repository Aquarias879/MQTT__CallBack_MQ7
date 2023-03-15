import re
from typing import NamedTuple
from flask import Flask, request
from flask_mqtt import Mqtt
import pymongo
import datetime
import pytz

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = 'broker.emqx.io'
app.config['MQTT_BROKER_PORT'] = 1883

mqtt = Mqtt(app)

TOPIC = 'gas_detect/+/+'
REGEX = 'gas_detect/([^/]+)/([^/]+)'

class SensorData(NamedTuple):
    device : str
    measurement: str
    value: float

def _parse_mqtt_message(topic, payload):
    match = re.match(REGEX, topic)
    if match:
        device = match.group(1)
        data_type = match.group(2)
        # Do something with the device and value here
        #print("Value: {}".format(device, data_type))
        if data_type == 'status':
            return None
        return SensorData(device,data_type, payload)
    else:
        return None
        
def insert_data_to_mongodb(sensor_data):
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    db = client["db_gas"]
    collection = db["key"]
    tz = pytz.timezone('Asia/Taipei')
    current_time = datetime.datetime.now(tz)
    data = {
        "device": sensor_data.device,
        "measurement": sensor_data.measurement,
        "value": sensor_data.value,
        "datetime": current_time
    }
    collection.insert_one(data)


@mqtt.on_connect()
def connect_broker(client, userdata, flags, rc):
    print('Connected with result code ' + str(rc))
    client.subscribe(TOPIC)

@app.route('/')
def index():
    return 'Flask-MQTT'


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    print(message.topic + ' ' + str(message.payload.decode('utf-8')))
    sensor_data = _parse_mqtt_message(message.topic, message.payload.decode('utf-8'))
    if sensor_data is not None:
        insert_data_to_mongodb(sensor_data)
#def handle_mqtt_message(client, userdata, message):
#    print(message.topic + ' ' + str(message.payload.decode('utf-8')))
#    sensor_data = _parse_mqtt_message(message.topic, message.payload.decode('utf-8'))
#    return sensor_data

if __name__ == '__main__':
    app.run()

