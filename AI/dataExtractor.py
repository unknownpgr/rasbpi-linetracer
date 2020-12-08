import os
import cv2
import numpy as np

def getData():
    trX = []; trY = []

    for name in os.listdir('./resized'):
        path = './resized/' + name
        img = cv2.imread(path)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        ret, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)
        binary = np.array(binary,np.float32)/255
        trX.append(np.expand_dims(binary, axis=2))
        tokens = name.split('_')
        posNoise = float(tokens[0])
        trY.append(posNoise/8)
        # angleNoise = float(tokens[1][:-4])

    trX = np.array(trX)
    trY = np.array(trY)

    return (trX, trY)