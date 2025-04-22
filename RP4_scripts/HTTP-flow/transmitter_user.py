from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
import os

app = Flask(__name__, static_folder="static")
CORS(app)

ENCODED_FILE_PATH = "/home/mukeshbala/Desktop/comp_output/encoded_data.txt"

def read_encoded_data():
    if os.path.exists(ENCODED_FILE_PATH):
        with open(ENCODED_FILE_PATH, "r") as file:
            lines = file.readlines()
            if lines:
                return lines[-1].strip()
            else:
                return "No data"
    else:
        return "Error: Encoded data file not found."

@app.route('/data', methods=['GET'])
def get_encoded_data():
    encoded_data = read_encoded_data()
    return jsonify({"encoded": encoded_data})

# Route to serve index.html at root path
@app.route('/')
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
