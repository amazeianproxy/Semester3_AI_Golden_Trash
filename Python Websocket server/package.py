import warnings
warnings.filterwarnings('ignore', category=FutureWarning)

import io
import base64
import cv2
import numpy as np
import re
import socket
from dotenv import load_dotenv
import os
import torch


DEBUG = True

model = torch.hub.load("./yolov5", 'yolov5s', source='local')

categories = [[39, 79, 41],[46,47,48,49,50,51,52,53,54,55,73, 27],[76, 42,43, 44, 64, 65, 66, 67,]] # plastic, Degradeable, Metal & electronics

def deprint(message: str | None = ""):
    if DEBUG:
        print(message)

def fillBuff(buffer:io.BytesIO, frame:bytes):
    buffer.seek(0)
    buffer.truncate(0)
    buffer.write(frame)
    buffer.truncate()

async def manage_request(data, client_id: int, currClient:int, ori_buffer:io.BytesIO):
    if(data['type'] == "image"):
        frame = data["frame"]
        if len(frame) > 0:
            print(f"image data received length: {len(frame)}")
            if(data['format'] == "jpeg->base64"):
                frame = base64.b64decode(frame) #move bin file to to 
            deprint("image data received")
            return await ai_process(frame, ori_buffer, currClient, client_id)
        else:
            print(f"Frame not received, data:\n{data}")
    else:
        print(f"Received data:\n{data}")


async def ai_process(frame:bytes, buffer:io.BytesIO, currClient:int, client_id:int):
    image_array = np.frombuffer(frame, dtype=np.uint8)

    deprint("Decoding image")
    image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    deprint("Model doing inferencing")
    results = model(image, size=640)
    
    deprint("getting anotated image")
    annotated_image = results.render()[0]  # Use render() to get annotated image
    annotated_image_bgr = cv2.cvtColor(annotated_image, cv2.COLOR_RGB2BGR)
    _, jpeg_bytes = cv2.imencode('.jpg', annotated_image_bgr)

    deprint("filling buffer")
    if(currClient == client_id):
        fillBuff(buffer, jpeg_bytes.tobytes())

    # Get predictions from the first (and only) image
    deprint("getting predictions")
    predictions = results.pred[0]  # Get detection boxes

    for *box, conf, cls in predictions:  # Unpack detection components
        deprint(f"Detected {cls} with confidence {conf} at {box}")
        clas = int(cls)  # Get class index
        conf = float(conf)  # Get confidence
        class_name = model.names[clas]  # Get class name
        print(f"Device: {currClient}, Class: {class_name}, Confidence: {conf:.2f}")
        da_angle = category_to_angle(clas)
        if(da_angle != 0):
            print(f"THE ANGLE IS {da_angle} <---------")
            return da_angle
    deprint("THE ANGLE IS 0 <---------")
    return 0

def category_to_angle(category_id):
    if(category_id in categories[0]):
        return 270
    elif(category_id in categories[1]):
        return 180
    elif(category_id in categories[2]):
        return 90
    else:
        return 0