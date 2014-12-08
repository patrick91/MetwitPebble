Metwit Pebble
=============
Metwit watchface for Pebble

Requirements
============
- Pebble Watchapp SDK (http://developer.getpebble.com/sdk/)

Build the watchface
===================

    $ export METWITPEBBLE_SERVER_URL=YOUR_SERVER_INSTANCE
    $ ./build.sh

Install the watchface
=====================

    $ export METWITPEBBLE_PHONE_IP=YOUR_PHONE_IP_ADDRESS
    $ ./install.sh

Run server locally
==================

    $ cd server
    $ python weather.py

Run server on Heroku
====================

    git subtree push --prefix server heroku master

License
=======
Relased under MIT license, Copyright (c) 2014
Andrea Stagi (stagi.andrea@gmail.com)
Patrick Guido Arminio (patrick.arminio@gmail.com)