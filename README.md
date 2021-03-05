# SMS website

A Laravel website which sends out and receives SMS messages.

# Dependencies

* `libzmq3-dev`
* `php-zmq`

These are the packages that can be installed with `apt`.

# Usage

* There are two C programs in *c_programs* directory. Compile them with `make` and run them with `sudo`
* You should start *sms_receiver* first before starting *sms_sender*

# TODO

* Fix hardcoded values into C programs (ZMQ addresses and URL address)