#!/bin/sh
echo "var base_url = \"${METWITPEBBLE_SERVER_URL}\";"> src/js/pebble-js-app.js
cat src/javascript/pebble-js-app.js >> src/js/pebble-js-app.js
pebble clean
pebble build