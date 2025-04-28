#include <cstdio>
#include <cstdlib>
extern "C" {
    #include "fitsio.h"
}
#define WIDTH 2048
#define HEIGHT 2048

int convert_raw_to_fits() {
    const char *raw_filename = "/opt/PrincetonInstruments/picam/samples/server-client/bin/exposure_file.raw";
    const char *fits_filename = "!exposure_file_cpp.fits";  // '!' allows overwrite

    long naxes[2] = {WIDTH, HEIGHT};
    size_t npixels = WIDTH * HEIGHT;

    fitsfile *fptr;      // FITS file pointer
    int status = 0;
    long fpixel = 1;

    // Allocate memory for the image
    unsigned short *image = (unsigned short *)malloc(npixels * sizeof(unsigned short));
    if (!image) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    // Open the raw binary file
    FILE *fp = fopen(raw_filename, "rb");
    if (!fp) {
        perror("Error opening raw file");
        free(image);
        return 1;
    }

    // Read raw image data
    if (fread(image, sizeof(unsigned short), npixels, fp) != npixels) {
        fprintf(stderr, "Error reading raw data.\n");
        fclose(fp);
        free(image);
        return 1;
    }
    fclose(fp);

    // Create a new FITS file
    if (fits_create_file(&fptr, fits_filename, &status)) {
        fits_report_error(stderr, status);
        free(image);
        return status;
    }

    // Define image type and dimensions
    if (fits_create_img(fptr, SHORT_IMG, 2, naxes, &status)) {
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        free(image);
        return status;
    }

    // Write the image data
    if (fits_write_img(fptr, TUSHORT, fpixel, npixels, image, &status)) {
        fits_report_error(stderr, status);
    }

    // Close the FITS file
    if (fits_close_file(fptr, &status)) {
        fits_report_error(stderr, status);
    }

    free(image);
    return status;
}

int main() {
    int status = convert_raw_to_fits();
    if (status == 0) {
        printf("FITS file created successfully.\n");
    } else {
        printf("FITS conversion failed with status %d.\n", status);
    }
    return status;
}
//g++ raw_to_fits.cpp -o raw_to_fits_cpp -lcfitsio
