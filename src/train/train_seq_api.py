import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.datasets import mnist
import numpy as np

nohot = True
nohot_str = ('noHot_' if nohot else '')

SEED = 1337
np.random.seed(SEED)
tf.random.set_seed(SEED)

gestures = ["punch","jump","knee"]
num_files = 1


# Cargar en arreglos las muestras y sus salidas
for g_index in range(len(gestures)):
	for i in range(num_files):
		if (i == 0 and g_index == 0):
			x = np.load(gestures[g_index]+'_accel_'+str(i)+'.npy')
			y = np.load(gestures[g_index]+'_output_' + nohot_str +str(i)+'.npy')
		else:
			x = np.concatenate((x,np.load(gestures[g_index]+'_accel_'+str(i)+'.npy')))
			y = np.concatenate((y,np.load(gestures[g_index]+'_output_' + nohot_str + str(i)+'.npy')))

# Mezclar las entradas y salidas de forma aleatoria
num_inputs = len(x)
randomize = np.arange(num_inputs)
np.random.shuffle(randomize)

x = x[randomize]
y = y[randomize]

#Dividir las muestras en 3 grupos para entrenar, validar y probar
TRAIN_SPLIT = int(0.6 * num_inputs)
TEST_SPLIT = int(0.2 * num_inputs + TRAIN_SPLIT)

x_train, x_test, x_validate = np.split(x, [TRAIN_SPLIT, TEST_SPLIT])
y_train, y_test, y_validate = np.split(y, [TRAIN_SPLIT, TEST_SPLIT])

#Reducir la dimensionalidad de los arreglos para que sean de 1 dimensión
x_train = x_train.reshape(-1,np.shape(x_train)[-1]*np.shape(x_train)[-2]).astype("float64")
x_test = x_test.reshape(-1,np.shape(x_test)[-1]*np.shape(x_test)[-2]).astype("float64")
x_validate = x_validate.reshape(-1,np.shape(x_validate)[-1]*np.shape(x_validate)[-2]).astype("float64")

#Modelos simple con 2 capas (1 entrada, 1 oculta) y 3 salidas
model = tf.keras.Sequential (
	[
		layers.Dense(50,activation='relu'),
		layers.Dense(15,activation='relu'),
		layers.Dense(len(gestures)),
	]
)

#Usando optimizador Adam y métrica de pérdida 'mae'
model.compile(
	loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
	optimizer=tf.keras.optimizers.Adam(learning_rate=0.0001),
	metrics=["mae"],
)

#Entrenar modelo
model.fit(x_train,y_train,batch_size=1,epochs=280)

#Validar y realizar pruebas
model.evaluate(x_test,y_test, batch_size=1)
model.evaluate(x_validate,y_validate, batch_size=1)

predictions = model.predict(x_validate)

#print(predictions.shape)

results = []
print(predictions[0],predictions[1])
for out in predictions:
	results.append(np.where(out == out.max())[0][0])


print("results =\n", np.array(results))
print("actual =\n", y_validate)


converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Guardar modelo en archivo
open("gesture_model.tflite", "wb").write(tflite_model)
  
import os
basic_model_size = os.path.getsize("gesture_model.tflite")
print("Model is %d bytes" % basic_model_size)