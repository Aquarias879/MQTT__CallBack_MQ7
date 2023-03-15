import re
from typing import NamedTuple
from flask import Flask, request
from flask_mqtt import Mqtt
# packages for swagger
from flasgger import Swagger
from flasgger import swag_from

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = 'broker.emqx.io'
app.config['MQTT_BROKER_PORT'] = 1883

mqtt = Mqtt(app)
# setup swagger online document
swagger = Swagger(app)

@app.route('/relay/<device_id>', methods=['POST'])
@swag_from('apidocs/api_publisher.yml')
def led(device_id):
    state = request.form.get('state')
    mqtt.publish('gas_detect/device' + device_id + '/relay_control', state)
    return "LED state changed to {} for device {}".format(state, device_id)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5005, debug=True)

