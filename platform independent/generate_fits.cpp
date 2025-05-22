#include <iostream>
#include <fitsio.h>

void createFitsFile(const char* filename) {
    fitsfile *fptr; // FITS file pointer
    int status = 0; // CFITSIO status value MUST be initialized to zero
    long naxes[2] = {300, 200}; // Size of the image: 300 x 200 pixels
    int bitpix = LONG_IMG; // 32-bit integer pixel values
    long fpixel[2] = {1, 1}; // First pixel to write (1-based index)
    long nelements = naxes[0] * naxes[1];
    long *array = new long[nelements];

    // Initialize the array with some values
    for (long i = 0; i < nelements; i++) {
        array[i] = i;
    }

    // Create a new FITS file
    if (fits_create_file(&fptr, filename, &status)) {
        fits_report_error(stderr, status); // Print out any error messages
        return;
    }

    // Create the primary array image (2D array)
    if (fits_create_img(fptr, bitpix, 2, naxes, &status)) {
        fits_report_error(stderr, status);
        return;
    }

    // Write the array of integers to the FITS file
    if (fits_write_pix(fptr, TLONG, fpixel, nelements, array, &status)) {
        fits_report_error(stderr, status);
        return;
    }

    // Close the FITS file
    if (fits_close_file(fptr, &status)) {
        fits_report_error(stderr, status);
        return;
    }

    delete[] array;
    std::cout << "FITS file created successfully!" << std::endl;
}

int main() {
    const char* filename = "!output.fits"; // '!' means overwrite if file already exists
    createFitsFile(filename);
    return 0;
}
