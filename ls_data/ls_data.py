from flask import Flask, request
from pymongo import MongoClient
# packages for swagger
from flasgger import Swagger
from flasgger import swag_from

app = Flask(__name__)
swagger = Swagger(app)

# Connect to the MongoDB database
client = MongoClient("mongodb://localhost:27017/")
db = client["db_gas"]
sensor_data_collection = db["key"]

@app.route('/')
def index():
    return 'Flask-MQTT'

@app.route('/sensordata/<device>', methods=['GET'])
@swag_from('apidocs/api_ls_data.yml')
def get_device_data(device):
    # Get the list of measurements from the URL query parameter
    measurements = request.args.get('measurements').split(',')

    # Query the database to get the latest data for the specified device and measurements
    latest_data = list(sensor_data_collection.find({"device": device, "measurement": {"$in": measurements}}).sort("datetime", -1).limit(1))

    # Convert the ObjectId to a string representation
    for data in latest_data:
        data['_id'] = str(data['_id'])

    # Return the latest data as a JSON response
    return latest_data

    # Get the list of measurements from the URL query parameter
    #measurements = request.args.get('measurements').split(',')

    # Query the database to get the data for the specified device and measurements
    #device_data = list(sensor_data_collection.find({"device": device, "measurement": {"$in": measurements}}))
    
    # Convert the ObjectId to a string representation
    #for data in device_data:
    #    data['_id'] = str(data['_id'])

    # Return the device data as a JSON response
    #return device_data

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)

