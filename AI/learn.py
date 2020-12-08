import cv2
from keras.models import *
from keras.layers import *
import numpy as np
import tensorflow as tf
from dataExtractor import getData

trX, trY = getData()

print(trX.shape)
print(trY.shape)
print(trX[0].shape)

seed = 0
np.random.seed(seed)
tf.random.set_seed(seed)

model = Sequential()
model.add(Conv2D(32, (3, 3), input_shape=(trX[0].shape), activation='relu'))
model.add(Dropout(0.25))
model.add(Conv2D(64, (3, 3), activation='relu'))
model.add(Dropout(0.25))
model.add(MaxPooling2D(pool_size=2))
model.add(Dropout(0.25))
model.add(Flatten())
model.add(Dense(512))
model.add(Activation('relu'))
model.add(Dropout(0.25))
model.add(Dense(64))
model.add(Activation('relu'))
model.add(Dropout(0.25))
model.add(Dense(1))
model.add(Activation('tanh'))

model.compile(loss='mean_squared_error', optimizer='adam')
model.fit(trX, trY, epochs=20, batch_size=50)

model.save('./fitted')