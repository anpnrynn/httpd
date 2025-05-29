#!/bin/bash
openssl req -new -nodes -x509 -newkey rsa:2048 -keyout httpdkey.pem -out httpdcert.pem -days 3650
