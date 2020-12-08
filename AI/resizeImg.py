import os
import cv2

for name in os.listdir('./imgs'):
    img = cv2.imread('./imgs/' + name)
    img = cv2.resize(img, (16, 16))
    cv2.imwrite('./resized/' + name, img)