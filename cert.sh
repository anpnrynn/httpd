#!/bin/bash
openssl req -new -x509 -newkey rsa:2048 -keyout httpdkey.pem -out httpdcert.pem -days 3650
