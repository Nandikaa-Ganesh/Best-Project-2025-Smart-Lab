import cv2
import numpy as np
import imutils
import requests
import os
import time

# Replace with your actual IP Webcam URL
url = "http://192.168.1.4:8080/shot.jpg"
save_dir = "/home/mukeshbala/Desktop/input"

os.makedirs(save_dir, exist_ok=True)

def manage_images():
    images = sorted(os.listdir(save_dir))
    while len(images) > 30:
        os.remove(os.path.join(save_dir, images[0]))
        images.pop(0)

while True:
    try:
        print("Getting IP Camera feed...")
        img_resp = requests.get(url, timeout=5)
        img_arr = np.array(bytearray(img_resp.content), dtype=np.uint8)
        img = cv2.imdecode(img_arr, -1)
        img = imutils.resize(img, width=800, height=600)

        # Save the image every 5 seconds
        timestamp = time.strftime("%Y%m%d-%H%M%S")
        image_path = os.path.join(save_dir, f"image_{timestamp}.jpg")
        cv2.imwrite(image_path, img)

        manage_images()
        print("Saved picture ",image_path)
        #time.sleep(5)  # Capture every 5 seconds
    except Exception as e:
        print(f"Error: {e}")
        time.sleep(2)
