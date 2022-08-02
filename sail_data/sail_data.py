import cv2
import numpy as np
from os import listdir
import pandas as pd
from mss import mss
from PIL import Image
import time
from datetime import datetime

# load digits
root_path = 'digits/'
digits_paths = sorted([f for f in listdir(root_path) if f != '.DS_Store'])

digits = []
for path in digits_paths:
    digit = cv2.imread(root_path + path)
    digits.append(digit.mean(axis=2))
digits = np.array(digits)

# window and numbers
w_size = np.array([100, 70])
mon = {'left': 0, 'top': 158, 'width': 760, 'height': 400}
features ={
  "awa": {"offset":  np.array([52,   63]), "decimal": False},
  "aws": {"offset":  np.array([250,  83]), "decimal": True},
  "sog": {"offset":  np.array([447,  83]), "decimal": True},
  "rollen": {"offset": np.array([644,  63]), "decimal": False},
  "twa": {"offset":  np.array([52,   1100 + 63]), "decimal": False},
  "tws": {"offset":  np.array([250,  1100 + 83]), "decimal": True},
  "stw": {"offset":  np.array([447,  1100 + 83]), "decimal": True},
  "kurs": {"offset": np.array([644,  1100 + 63]), "decimal": False},
}

count = 0
export_data = []
init_time = time.time()
reffen = float(input("Reffstufe:"))
print("Alles klar Matrose!")
init_date = datetime.now().strftime("%d-%m-%Y_%H-%M-%S")

with mss() as sct:
    while True:
      if time.time() - init_time - count > 1:
        count += 1

        screenShot = sct.grab(mon)
        frame = np.array(Image.frombytes(
            'RGB', 
            (screenShot.width, screenShot.height), 
            screenShot.rgb, 
        ))

        # cv2.imwrite("temp/frame%d.jpg"%count, frame)

        value_dict = {}
        for name, feature in features.items():
          current_value = 0
          for i in range(3):
            current_digit = frame[feature["offset"][0]:feature["offset"][0]+ w_size[0], feature["offset"][1]+ i * w_size[1]:feature["offset"][1]+ (i+1) * w_size[1], :]
            if feature["decimal"] and i == 2:
              current_digit = frame[feature["offset"][0]:feature["offset"][0]+ w_size[0], feature["offset"][1]+ i * w_size[1] + 20:feature["offset"][1]+ (i+1) * w_size[1] + 20, :]

            # if name == "kurs": cv2.imwrite("temp/frame_%d_%s_%d.jpg"%(count, name, i), current_digit)
            diff = digits - np.mean(current_digit, axis=2)
            prediction = np.argmin(np.abs(diff).mean(axis=(1,2))) % 10
            current_value += 10**(1-i) * prediction if feature["decimal"] else 10**(2-i) * prediction

          value_dict[name] = current_value

        export_data.append([datetime.now().strftime("%d-%m-%Y %H:%M:%S"), reffen] + list(value_dict.values()))

        if count % 10 == 0:
          # Export data
          export_data_pd = pd.DataFrame(export_data, columns=["time", "reffen"] + list(value_dict.keys()))
          export_data_pd.to_csv("export/sailing_data_%s.csv"%init_date)