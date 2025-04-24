"""
This is a python script that resizes the .raw file acquired from expose(), burst(), dark().
The image was taken with a 2048 BUV camera (13.5 micrometer pixel size), exptime= 10000ms (see camera.cpp)
It is resized to 2048 px x 2048 px, imaging area = 27.6mm x 27.6mm ##CHECK THIS
"""

import numpy as np
import matplotlib.pyplot as plt
import os

def try_dtype(rawfile, dtype, shape_hint=None):
    element_size = np.dtype(dtype).itemsize
    file_size = os.path.getsize(rawfile)
    num_elements = file_size // element_size

    print("\nTesting dtype:", dtype)
    print("File size: {} bytes".format(file_size))
    print("Element size: {} bytes".format(element_size))
    print("Number of elements:", num_elements)

    try:
        data = np.fromfile(rawfile, dtype=dtype)
        print("Data array length:", len(data))
        print("Min:", data.min(), "Max:", data.max())
        print("Memory dtype:", data.dtype)

        # Show dtype of beginning, middle, and end
        print("\nData types of selected values:")
        print("Beginning (index 0):", type(data[0]))
        print("Middle (index {}):".format(len(data)//2), type(data[len(data)//2]))
        print("End (index {}):".format(len(data)-1), type(data[-1]))

        if shape_hint:
            target_size = np.prod(shape_hint)
            if len(data) < target_size:
                padding = target_size - len(data)
                print("Padding with", padding, "zeros")
                data = np.pad(data, (0, padding), mode='constant')
            elif len(data) > target_size:
                print("Trimming data to fit shape_hint")
                data = data[:target_size]

            image = data.reshape(shape_hint)
            plt.imshow(image, cmap='gray')
            plt.title("Raw Image Display ({})".format(dtype))
            plt.show()
        else:
            print("No shape_hint provided.")

    except Exception as e:
        print("Error reading as {}: {}".format(dtype, e))


def show_raw_data():
    rawfile = '/opt/PrincetonInstruments/picam/samples/server-client-simple/bin/exposure_file.raw'

    shape_hint = (2048, 2048)
    for dtype in [np.uint16]:
    # for dtype in [np.uint16, np.int16]:
        try_dtype(rawfile, dtype, shape_hint)

show_raw_data()