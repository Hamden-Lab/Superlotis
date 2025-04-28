"""
This is a Python script that resizes the .raw file acquired from expose(), burst(), or dark().
The image was taken with a 2048 BUV camera (13.5 micrometer pixel size), exptime = 10000 ms (see camera.cpp).
It is resized to 2048 px x 2048 px, imaging area = 27.6 mm x 27.6 mm ##CHECK THIS
"""

import numpy as np
import matplotlib.pyplot as plt
import os

    
def resize(rawfile, dtype, shape):
    """
    Inputs: 
    - rawfile
    - dtype (datatype): uint16
    - shape (tuple): expected shape of the image (e.g., 2048x2048)
    Returns:
    - resized data
    """
    element_size = np.dtype(dtype).itemsize
    file_size = os.path.getsize(rawfile)
    num_elements = file_size // element_size
    print(file_size, element_size, num_elements)

    data = np.fromfile(rawfile, dtype=dtype)
    target_size = np.prod(shape)

    if len(data) < target_size:
        padding = target_size - len(data)
        print("Padding with", padding, "zeros")
        data = np.pad(data, (0, padding), mode='constant')
    elif len(data) > target_size:
        diff = len(data) - target_size
        print("Trimming data by: " + str(diff) + " to fit "+str(shape))
        data = data[:target_size]

        return data

def plot_resized(data):
    image = data.reshape(shape)
    plt.imshow(image, cmap='gray')  # Use grayscale for raw data
    plt.title("Raw Image Display ({})".format(dtype))
    plt.show()




# File path and parameters
rawfile = '/opt/PrincetonInstruments/picam/samples/server-client-simple/bin/exposure_file.raw'
dtype = np.uint16
shape = (2048, 2048) 

data = resize(rawfile, dtype, shape)
plot_resized(data)
