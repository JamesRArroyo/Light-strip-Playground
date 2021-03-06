# ESP Light Strip Controller

Control multiple light strips each on their own ESP8266. Supports OTA updates.

## Prerequisites

- Install Node.js® and npm if they are not already on your machine.
- Update `dist/ArduinoCode/ArduinoCode.ini` variables (Wifi, Node ip, etc..)

## Start Servers
### Mosca (MQTT Broker)
- From the mosca folder run ```mosca -V | bunyan```

### Node (Updates / MQTT communicator)
- Initially install dependencies by running ```npm install```
- Run `npm start` to spin up the node server.

### Angular (Front End Controller)
- Navigate to the ng-light-show folder and run ```npm install```
- `ng serve` to start the app found on `localhost:4200`

## Exporting Arduino Code
  
  1. Open up dist/ArduinoCode/ArduinoCode.ini in Arduino Sketch
  2. Verify/Compile the code. ```(Sketch->Verify/Compile)```
  3. Export compiled binary. ```(Sketch->Export Compiled Binary)```
  4. The new compiled code is now ready to be pulled for updates.

## Current way of updating (Still in the works)

* OTA Update Setup
  - **IMPORTANT STEP** - Kill Node Server
  - Plug in ESP8266
  - Open up dist/ArduinoCode/ArduinoCode.ini in Arduino Sketch
  - Upload Code to the ESP8266 via USB and Arduino Sketch.
  - **IMPORTANT STEP** - Once upload is finished press the reset button on the ESP8266.
  - Your device may be unplugged and is ready for OTA updates.

* OTA Updates (This process will get better)
  - Increment the `FW_VERSION` number when changes have been made to `dist/ArduinoCode/ArduinoCode.ini` and they are ready to be deployed to your devices.
  - Export compiled binary. ```(Sketch->Export Compiled Binary)```
  - Update the `firmwareVersionNumber` in `src/firmwareUpdate/firmwareUpdate.routes.ts` to match the new version number in the previous step. 
  - Hit the reset button the the ESP8266 or unplug/plug in the power. (Updates are checked when the device initially boots).


