import cv2
import keras
from keras.models import Sequential
from keras.layers import Dense
import numpy as np
import tensorflow as tf
from dataExtractor import getData

model = keras.models.load_model('./fitted')
teX, teY = getData()

predicted = model.predict(teX).reshape(teY.shape)
errors = predicted - teY

print(teX.shape)
print(teY.shape)
print(predicted.shape)
print(errors.shape)

print("min: ", np.min(errors))
print("max: ", np.max(errors))
print("std: ", np.std(errors))

print(np.count_nonzero(teY*predicted > 0))
print(np.count_nonzero(errors < 0.1))