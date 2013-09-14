import requests, json
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/weather", methods = ['POST', 'GET'])
def weather():
    return jsonify( { "0": "OK" } )

if __name__ == "__main__":
    app.run(host='0.0.0.0')