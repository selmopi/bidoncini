#The central server

The central server is the one that collects all the messages sent through MQTT by all the LoRa gateways, and then sends everything to the AWS server.
Since to talk with the AWS console there is the need of having some keys and certificates, there are some passages to do to obtain your certificates.
1. Log into your AWS console
2. Search for the IoT Core module
3. Create a new object
4. Install the files and keys generated by AWS and follow the instructions into the AWS README.md
5. Modify the file < device_connect_package>/aws-iot-device-sdk-python-v2/samples/pubsub.py with the one that you find into this directory.

Practically we have integrated the MQTT receiver that receives the messages from the LoRaWAN gateways with the MQTT publisher that publishes those messages in AWS.