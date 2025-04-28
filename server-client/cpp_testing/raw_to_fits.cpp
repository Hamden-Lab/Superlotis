#include <cstdio>
#include <cstdlib>
extern "C" {
    #include "fitsio.h"
}
#include <iostream>
#include <fstream>
#include <cstring>

#define WIDTH 2048
#define HEIGHT 2048
#define ELEMENT_SIZE 2


int resize_raw(const char* filename) {
    // const int element_size = 2; // sizeof(uint16_t)
    int target_elements = WIDTH * HEIGHT;
    int target_bytes = target_elements * ELEMENT_SIZE;

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return -1;
    }

    int file_size = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);
    int num_elements = file_size / ELEMENT_SIZE;

    std::cout << "Original file size: " << file_size << " bytes" << std::endl;
    std::cout << "Target size: " << target_bytes << " bytes" << std::endl;

    char* data = (char*)std::malloc(file_size);
    if (!data) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return -1;
    }

    file.read(data, file_size);
    if (!file) {
        std::cerr << "Error reading the file!" << std::endl;
        std::free(data);
        return -1;
    }
    file.close();

    char* output_data = (char*)std::malloc(target_bytes);
    if (!output_data) {
        std::cerr << "Output memory allocation failed!" << std::endl;
        std::free(data);
        return -1;
    }

    if (num_elements < target_elements) {
        std::memcpy(output_data, data, num_elements * ELEMENT_SIZE);
        std::memset(output_data + num_elements * ELEMENT_SIZE, 0, (target_elements - num_elements) * ELEMENT_SIZE);
        std::cout << "Padded with " << (target_elements - num_elements) << " zeros" << std::endl;
    } else if (num_elements > target_elements) {
        std::memcpy(output_data, data, target_bytes);
        std::cout << "Trimmed extra data" << std::endl;
    } else {
        std::memcpy(output_data, data, target_bytes);
        std::cout << "No resizing needed." << std::endl;
    }

    std::free(data);
    return 0;
}

int convert_raw_to_fits(const char *filename, const char *fits_filename) {

    if (resize_raw(filename) == 0){

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
    FILE *fp = fopen(filename, "rb");
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
    }else{
        return -1;
    }
}

int main() {
    // const char *raw_filename = "/opt/PrincetonInstruments/picam/samples/server-client/bin/exposure_file.raw";
    // const char *fits_filename = "!exposure_file_cpp.fits";  // '!' allows overwrite

    int status = convert_raw_to_fits("/opt/PrincetonInstruments/picam/samples/server-client/bin/exposure_file.raw", "!exposure_file_cpp2.fits");
    if (status == 0) {
        printf("FITS file created successfully.\n");
    } else {
        printf("FITS conversion failed with status %d.\n", status);
    }
    return status;
}
//g++ raw_to_fits.cpp -o raw_to_fits_cpp -lcfitsio
