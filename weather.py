import requests
from flask import Flask, request, jsonify

from metwit import Metwit

app = Flask(__name__)
app.debug = True

error_codes = {
    'NO_RESULTS': 1
}

@app.route("/weather", methods = ['POST'])
def weather():
    """
    INPUT BUNDLE: {'0': latitude, '1': longitude, '2': unit (f or c)}
    E.G. {'0': 439596, '1': 109551, '2': 'f'}

    OUTPUT BUNDLE: {'0': error code, '1': weather condition, '2': temperature}
    E.G. {'0': 0, '1': 1, '2': 78}
    """

    latitude = float(request.form['0']) / 1000
    longitude = float(request.form['1']) / 1000

    format = request.form.get('2', 'c')

    results = Metwit.weather.get(location_lat=latitude, location_lng=longitude)

    if len(results) > 1:
        weather = results[0]['weather']

        temperature = weather['measured']['temperature']
        status = weather['status']

        # temperature is in Kelvin

        if format is 'c':
            temperature = temperature - 273.15
        else:
            temperature = temperature * 9 / 5.0 - 459.67

        return jsonify( {'0': 0, '1': 'TODO', '2': temperature})
    else:
        error_code = error_codes['NO_RESULTS']

    return jsonify({'0': error_code})

if __name__ == "__main__":
    app.run(host='0.0.0.0')
