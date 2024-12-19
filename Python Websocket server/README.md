To setup the server

1. Makesure to download all requirements with "pip install -r requirements.txt"
    you can download it with virtual environments like Anaconda or venv

2. create new .env file to this exact folder. Copy the .env.template templates and paste it to the .env and configure the environment variables. After you configure which port, configure the port's access in firewall.
    I. Configure port access in windows firewall
        - open Windows Defender Firewall with Advance Security
        - Make an inbound rule for port WEBSOCKET_AND_HTTP_PORT that you used for the server, with the connection type TCP
        - bikin inbound rule baru untuk port mDNS standard, yaitu 5353 dengan tipe koneksi UDP (not mandatory because not meant to)

3. make sure to get the yolov5.pt models by download / cloning the yolov5 project from github "https://github.com/ultralytics/yolov5.git" or directly downloading the .pt file from the github. Our tests runs on yolov5s.pt model.

4. put the .pt file on the same hierarchy / directory as the main.py file

5. 



Disclamer: 
    - The mDNS is a future upgrade, until now, the connection is done with static IP.

