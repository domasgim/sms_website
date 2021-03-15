# SMS website

A Laravel website which sends out and receives SMS messages.

# Dependencies

* `libzmq3-dev`
* `php-zmq`

These are the packages that can be installed with `apt`.

# Usage

* There are two C programs in *c_programs* directory. Compile them with `make` and run them with `sudo`
* You should start *sms_receiver* first before starting *sms_sender*

# Credits
* *sms_sender* program uses tools from [SMS Server Tools 3](http://smstools3.kekekasvi.com/). All credit to Stefan Frings (original author of SMS Server Tools), Keijo "Keke" Kasvi (current maintainer) and other contributors.
* *sms_receiver* program uses tools from [hitmoon pdu decoder](https://github.com/hitmoon/sms-pdu).