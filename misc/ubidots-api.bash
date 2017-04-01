#!/bin/bash

set -e
set -x 

function call() {
  curl -s -H "X-Auth-Token: $TOKEN" -H "Content-Type:application/json" "$@"
}

#call -X POST http://things.ubidots.com/api/v1.6/devices/dev0 -d '{"temperature": 20, "x": 20}' | python -m json.tool

#call -X GET  http://things.ubidots.com/api/v1.6/devices/dev0 | python -m json.tool

#[Messenger] INFO: Putting:  {"clock0.hour":"1","clock0.minute":"1","clock0.advanced":"false","led0.on":"0","led1.on":"0"}
#call -X POST http://things.ubidots.com/api/v1.6/devices/dev0 -d '{"clock0.hour":"1","clock0.minute":"1","clock0.advanced":"false","led0.on":"0","led1.on":"0"}'

#[Messenger] INFO: Putting:  {"clock0.hour":"1","clock0.minute":"1","clock0.advanced":"false","led0.on":"0","led1.on":"0"}
call -X POST http://things.ubidots.com/api/v1.6/devices/dev0 -d '{"clock0.hour":1,"clock0.minute":1,"clock0.advanced":false,"led0.on":false,"led1.on":false}'

call -X POST http://things.ubidots.com/api/v1.6/devices/dev0 -d '{"clock0.hour":2,"clock0.minute":0,"clock0.advanced":false,"led0.on":true,"led1.on":true}'

call -X POST http://things.ubidots.com/api/v1.6/devices/dev0 -d '{"clock0.hour":3,"clock0.minute":0,"clock0.advanced":false,"led0.on":true,"led1.on":false}'

call -X GET  http://things.ubidots.com/api/v1.6/devices/dev0/values | python -m json.tool

#call -X GET  http://things.ubidots.com/api/v1.6/devices/dev0/temperature/values | python -m json.tool

#call -X GET  http://things.ubidots.com/api/v1.6/devices/dev0/temperature/values?page_size=1 | python -m json.tool
