import os
import cv2
from keras.models import Sequential
from keras.layers import Dense
import numpy as np
import tensorflow as tf

trX = []; trY = []

for name in os.listdir('./resized'):
    path = './resized/' + name
    img = cv2.imread(path)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    ret, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)
    binary = np.array(binary,np.float32)/255
    trX.append(binary.reshape(binary.shape[0] * binary.shape[1]))
    tokens = name.split('_')
    posNoise = float(tokens[0])
    trY.append(posNoise)
    # angleNoise = float(tokens[1][:-4])

trX = np.array(trX)
trY = np.array(trY)

seed = 0
np.random.seed(seed)
tf.random.set_seed(seed)

model = Sequential()
model.add(Dense(512, input_dim = trX[0].shape[0], activation = 'relu'))
model.add(Dense(64, activation = 'relu'))
model.add(Dense(1))

model.compile(loss='mean_squared_error', optimizer='adam')
model.fit(trX, trY, epochs=2, batch_size=1)

model.save('./fitted')