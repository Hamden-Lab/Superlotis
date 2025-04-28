import numpy as np
import matplotlib.pyplot as plt

def read_fits_header(f):
    header = b""
    while True:
        block = f.read(2880)
        header += block
        if b"END" in block:
            break
    return header.decode('ascii'), len(header)

def parse_header(header_str):
    header = {}
    for i in range(0, len(header_str), 80):
        card = header_str[i:i+80].strip()
        if '=' in card:
            parts = card.split('=')
            key = parts[0].strip()
            value = parts[1].split('/')[0].strip()
            header[key] = value
    return header

def fits_viewer(file_path):
    f = open(file_path, 'rb')

    header_str, header_len = read_fits_header(f)
    header = parse_header(header_str)

    # Print the header (Python 2.7-compatible)
    print "FITS Header:"
    for key in header:
        print "{} = {}".format(key, header[key])

    # Get image shape
    naxis = int(header.get('NAXIS', 0))
    if naxis < 2:
        raise ValueError("Not a 2D FITS image.")
    width = int(header['NAXIS1'])
    height = int(header['NAXIS2'])
    bitpix = int(header['BITPIX'])

    dtype_map = {
        8: np.uint8,
        16: np.int16,
        32: np.int32,
        -32: np.float32,
        -64: np.float64
    }

    if bitpix not in dtype_map:
        raise ValueError("Unsupported BITPIX: {}".format(bitpix))

    dtype = dtype_map[bitpix]

    # Read image data
    data = np.fromfile(f, dtype=dtype, count=width * height)
    data = data.reshape((height, width))

    f.close()

    # Plot
    plt.imshow(data, cmap='gray', origin='lower')
    plt.colorbar()
    plt.title('FITS Image (Python 2.7)')
    plt.show()

# Run it
fits_viewer('exposure_file.fits')
