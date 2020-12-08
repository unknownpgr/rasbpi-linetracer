import os
import cv2
import keras
from keras.models import Sequential
from keras.layers import Dense
import numpy as np
import tensorflow as tf

model = keras.models.load_model('./fitted')
teX = []; teY = []

for name in os.listdir('./resized'):
    path = './resized/' + name
    img = cv2.imread(path)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    ret, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)
    binary = np.array(binary,np.float32)/255
    teX.append(binary.reshape(binary.shape[0] * binary.shape[1]))
    tokens = name.split('_')
    posNoise = float(tokens[0])
    teY.append(posNoise)

teX = np.array(teX)
teY = np.array(teY)
predicted = model.predict(teX).reshape(teY.shape)
errors = predicted - teY

print(teX.shape)
print(teY.shape)
print(predicted.shape)
print(errors.shape)

print("min: ", np.min(errors))
print("max: ", np.max(errors))
print("std: ", np.std(errors))

print(np.count_nonzero(np.dot(teY, predicted) > 0))
print(np.count_nonzero(errors < 1))