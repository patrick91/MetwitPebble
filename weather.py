import requests, json
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/weather", methods = ['POST'])
def weather():
    """
    INPUT BUNDLE: {'0': latitude, '1': longitude, '2': unit (f or c)}
    E.G. {'0': 439596, '1': 109551, '2': 'f'}

    OUTPUT BUNDLE: {'0': weather condition, '1': temperature}
    E.G. {'0': 1, '1': 78}
    """
    return jsonify( {'0': 1, '1': 78} )

if __name__ == "__main__":
    app.run(host='0.0.0.0')