1. Go inside the lib folder and inside the conf folder (./lib/conf). configure the conf.h file with your own credentials and necessity. (recomend to only configure the environment variables)

2. the port and websocket variables has to be the same with the python server device's credentials also.

3. ATTENTION !!!
    I. Connect the server to the same local wifi with the microcontroler 
    II. IP of the server's device is the SERVER_STATIC_IP variable
    III. Port is the same with the python server's port
    IV. Route is the server's route (recomend not changing)
    V. WEBSOCKET_HOST_NAME is for mDNS, but is maybe not needed (still works because of the static ip)

4. I left the platformio.ini for a reference, in this project, I'm using an ESP32-WROVER-E CAM devboard and the servo is a MG90S 360 continuous servo.

5. To run, ctrl+shift+p and search for "PlatoformIO: New Terminal"

6. plug in your devboard

7. run "pio run -t clean" to clean the build
8. run "pio run -t erase" to erase the build inside the devboard (remove the program inside the mc)
9. run "pio run -t upload" to upload the build to the chip and simultaneously doing the compiling and building
10. run "pio run -t monitor" to watch the code's output and prints to monitor the program running