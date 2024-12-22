1. Go inside the /lib/ folder and into the /conf/ folder (./lib/conf). Configure the conf.h file with your own credentials as necessary. (It is recommended to only configure the environment variables.)

2. The port and websocket variables have to be the same as the python server device's credentials.

3. ATTENTION !!!
   - I. Connect the server to the same local wifi with the microcontroller.
   - II. The IP of the server's device is the SERVER_STATIC_IP variable.
   - III. The port is the same with the python server's port.
   - IV. The route is the server's route (It is recommended to not change this.)
   - V. The WEBSOCKET_HOST_NAME is for mDNS, but it is probably not needed (still works because of the static ip).

4. I left the platformio.ini for a reference, in this project, where I'm using an ESP32-WROVER-E CAM devboard and a servo, which is a MG90S 360 continuous servo.

5. To run, CTRL+SHIFT+P and search for "PlatoformIO: New Terminal".

6. Plug in your devboard.

7. Run "pio run -t clean" to clean the build.
   
8. Run "pio run -t erase" to erase the build inside the devboard. (remove the program inside the mc)

9. Run "pio run -t upload" to upload the build to the chip while simultaneously doing compiling and building.
    
10. Run "pio run -t monitor" to watch the code's output, where it prints to monitor the program running.
