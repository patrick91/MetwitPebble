var initialized = false;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});


function fetchWeather(latitude, longitude, unit) {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://192.168.0.6:5000/weather?" +
    "lat=" + latitude + "&lon=" + longitude + "&unit=" + unit, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var temperature, status;
        if (response) {
          console.log("send" + response.status);
          Pebble.sendAppMessage({
            "0":response.status,
            "1":response.temperature + "\u00B0" + unit
          });
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude, 'C');
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "0":0,
    "1":"N/A"
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 };


Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.watchPosition(
                            locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                          console.log(e.payload.temperature);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });
