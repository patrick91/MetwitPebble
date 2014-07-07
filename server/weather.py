# -*- coding: utf-8 -*-

import math

from metwit import Metwit

from flask import Flask, request, jsonify, render_template

app = Flask(__name__)
app.debug = True

error_codes = {
    'NO_RESULTS': 1
}

weather_statuses = {
    'unknown': 0,
    'sunny': 1,
    'rainy': 2,
    'stormy': 3,
    'snowy': 4,
    'partly_cloudy': 5,
    'cloudy': 6,
    'hailing': 7,
    'heavy_seas': 8,
    'calm_seas': 9,
    'foggy': 10,
    'snow_flurries': 11,
    'windy': 12,
    'clear_moon': 13,
    'partly_moon': 14,
    'twitter': 15,
    'instagram': 15,
}


def get_result_with_temperature(results):
    """
    Returns the first result with temperature found in results.
    If no result with temperature is found returns None.
    """

    for result in results:
        weather = result['weather']

        if 'measured' in weather and 'temperature' in weather['measured']:
            return result

    return None


@app.route('/weather', methods=['GET'])
def weather():

    latitude = float(request.args.get('lat')) / 10000
    longitude = float(request.args.get('lon')) / 10000

    format = request.args.get('unit')

    results = Metwit.weather.get(location_lat=latitude, location_lng=longitude)

    result = get_result_with_temperature(results)

    if result is not None:
        weather = result['weather']
        temperature = weather['measured']['temperature']

        icon_name = result['icon'].split('/')[-1]

        if icon_name in weather_statuses:
            status = weather_statuses[icon_name]
        else:
            status = weather_statuses['unknown']

        # temperature is in Kelvin

        if format == 'C':
            temperature = temperature - 273.15
        else:
            temperature = temperature * 9 / 5.0 - 459.67

        formatted_temperature = format_temperature(temperature)

        return jsonify({
            'status': status,
            'temperature': formatted_temperature
        }), 200
    else:
        error_code = error_codes['NO_RESULTS']

    return jsonify({'0': error_code}), 503

def format_temperature(temperature):
    return str(int(round(temperature)))

@app.route('/')
def home():
    return render_template('index.html')

if __name__ == "__main__":
    app.run(host='0.0.0.0')
