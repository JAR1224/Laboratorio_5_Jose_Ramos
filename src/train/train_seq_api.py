import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.datasets import mnist
import numpy as np

!echo "const unsigned char model[] = {" > /content/model.h
!cat gesture_model.tflite | xxd -i      >> /content/model.h
!echo "};"                              >> /content/model.h

import os
model_h_size = os.path.getsize("model.h")
print(f"Header file, model.h, is {model_h_size:,} bytes.")
print("\nOpen the side panel (refresh if needed). Double click model.h to download the file.")

exit()

nohot = True
nohot_str = ('noHot_' if nohot else '')

SEED = 1337
np.random.seed(SEED)
tf.random.set_seed(SEED)

gestures = ["rest","punch"]
num_files = 1

for g_index in range(len(gestures)):
	for i in range(num_files):
		if (i == 0 and g_index == 0):
			x = np.load(gestures[g_index]+'_accel_'+str(i)+'.npy')
			y = np.load(gestures[g_index]+'_output_' + nohot_str +str(i)+'.npy')
		else:
			x = np.concatenate((x,np.load(gestures[g_index]+'_accel_'+str(i)+'.npy')))
			y = np.concatenate((y,np.load(gestures[g_index]+'_output_' + nohot_str + str(i)+'.npy')))

num_inputs = len(x)
randomize = np.arange(num_inputs)
np.random.shuffle(randomize)

x = x[randomize]
y = y[randomize]

TRAIN_SPLIT = int(0.6 * num_inputs)
TEST_SPLIT = int(0.2 * num_inputs + TRAIN_SPLIT)

x_train, x_test, x_validate = np.split(x, [TRAIN_SPLIT, TEST_SPLIT])
y_train, y_test, y_validate = np.split(y, [TRAIN_SPLIT, TEST_SPLIT])

#print(np.shape(x_train),np.shape(x_test),np.shape(x_validate))
#print(np.shape(y_train),np.shape(y_test),np.shape(y_validate))

#print(y_train)

#(x_train, y_train), (x_test, y_test) = mnist.load_data()
#print(x_train.shape)
#print(y_train.shape)

x_train = x_train.reshape(-1,np.shape(x_train)[-1]*np.shape(x_train)[-2]).astype("float64")
x_test = x_test.reshape(-1,np.shape(x_test)[-1]*np.shape(x_test)[-2]).astype("float64")
x_validate = x_validate.reshape(-1,np.shape(x_validate)[-1]*np.shape(x_validate)[-2]).astype("float64")

#print(x_train.shape)
#print(y_train.shape)

#print(x_train[0])
#print(x_train[1])

#print(y_train[0])
#print(y_train[1])

#exit()

model = tf.keras.Sequential (
	[
		layers.Dense(50,activation='relu'),
		layers.Dense(15,activation='relu'),
		layers.Dense(len(gestures)),

		#layers.Dense(512,activation='relu'),
		#layers.Dense(512,activation='relu'),
		#layers.Dense(256,activation='relu'),
		#layers.Dense(128,activation='relu'),
		#layers.Dense(len(gestures)),

		#layers.Dense(50,activation='relu'),
		#layers.Dense(15,activation='relu'),
		#layers.Dense(len(gestures)),
	]
)

model.compile(
	loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
	optimizer=tf.keras.optimizers.Adam(learning_rate=0.0001),
	#optimizer=tf.keras.optimizers.Adam(learning_rate=0.00001),
	metrics=["mae"],
)

model.fit(x_train,y_train,batch_size=1,epochs=300)

model.evaluate(x_test,y_test, batch_size=1)
model.evaluate(x_validate,y_validate, batch_size=1)


predictions = model.predict(x_validate)

#print(predictions.shape)
#print(predictions[0])
#print(predictions[1])

results = []
for out in predictions:
	results.append(0 if out[0]>out[1] else 1)

#print("predictions =\n", np.round(predictions, decimals=3))
print("results =\n", np.array(results))
print("actual =\n", y_validate)

converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the model to disk
open("gesture_model.tflite", "wb").write(tflite_model)
  
import os
basic_model_size = os.path.getsize("gesture_model.tflite")
print("Model is %d bytes" % basic_model_size)