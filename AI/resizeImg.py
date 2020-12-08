import os
import cv2

for name in os.listdir('./imgs'):
    img = cv2.imread('./imgs/' + name)
    img = cv2.resize(img, (20, 15))
    cv2.imwrite('./resized/' + name, img)