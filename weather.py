import math

from metwit import Metwit

from flask import Flask, request, jsonify

app = Flask(__name__)
app.debug = True

error_codes = {
    'NO_RESULTS': 1
}


def get_result_with_temperature(results):
    """
    Returns the first result with temperature found in results.
    If no result with temperature is found returns None.
    """

    for result in results:
        weather = result['weather']

        if 'measured' in weather and 'temperature' in weather['measured']:
            print result
            return result

    return None


@app.route("/weather", methods=['POST'])
def weather():
    """
    INPUT BUNDLE: {'0': latitude, '1': longitude, '2': unit ("F" or "C")}
    E.G. {'0': 439596, '1': 109551, '2': 'F'}

    OUTPUT BUNDLE: {'0': error code, '1': weather condition, '2': temperature}
    E.G. {'0': 0, '1': 1, '2': 78}
    """

    latitude = float(request.json['0']) / 10000
    longitude = float(request.json['1']) / 10000

    format = request.json.get('2', 'C')

    results = Metwit.weather.get(location_lat=latitude, location_lng=longitude)

    result = get_result_with_temperature(results)

    if results is not None:
        weather = result['weather']
        temperature = weather['measured']['temperature']
        status = weather['status']

        # temperature is in Kelvin

        if format == 'C':
            temperature = temperature - 273.15
        else:
            temperature = temperature * 9 / 5.0 - 459.67

        return jsonify({
            '0': 0,
            '1': 'TODO',
            '2': int(math.ceil(temperature))
        })
    else:
        error_code = error_codes['NO_RESULTS']

    return jsonify({'0': error_code})

if __name__ == "__main__":
    app.run(host='0.0.0.0')
