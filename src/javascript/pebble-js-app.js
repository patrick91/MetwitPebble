var initialized = false;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  initialized = true;
});


function fetchWeather(latitude, longitude, unit) {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', base_url + "/weather?" +
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
  var uofm = localStorage.getItem("uofm");
  if (!uofm) {
    uofm = 'C'
  }
  fetchWeather(coordinates.latitude, coordinates.longitude, uofm);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "0":0,
    "1":"N/A"
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 };


Pebble.addEventListener("appmessage",
                        function(e) {
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log("Received message: " + e.payload);
                        });

Pebble.addEventListener("webviewclosed",
                        function(e) {
                          console.log("webview closed");
                          console.log(e.type);
                          console.log(e.response);
                        });

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL(base_url + '/pebble/settings');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log("Options = " + JSON.stringify(options));
  localStorage.setItem("uofm", options['unit-measure']);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
});
