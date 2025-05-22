#include <fitsio.h>
#include <iostream>
#include <cstdlib>
#include "picam.h"
#include "test.h"

//globals
GainPtcArgs params;

int main() {
    fitsfile *fptr; // FITS file pointer
    int status = 0; // Status variable

    // Example data to be written
    int id[] = {1, 2, 3};
    const char *name[] = {"John", "Jane", "Doe"};
    int age[] = {25, 30, 40};
    int nrows = 3; // Number of rows

    // Create a new FITS file
    const char *fits_file = "example.fits";
    if (fits_create_file(&fptr, fits_file, &status)) {
        fits_report_error(stderr, status);
        return status;
    }

    // Create a new table
    const char *ttype[] = {"ID", "Name", "Age"};
    const char *tform[] = {"1I", "10A", "1I"};
    const char *tunit[] = {"", "", ""};

    // Create ASCII table
    if (fits_create_tbl(fptr, BINARY_TBL, 0, 3,
                        (char**)ttype, (char**)tform, (char**)tunit,
                        "Data", &status)) {
        fits_report_error(stderr, status);
        return status;
    }

    // Write data to table
    long firstrow = 1;
    if (fits_write_col(fptr, TINT, 1, firstrow, 1, nrows, id, &status)) {
        fits_report_error(stderr, status);
        return status;
    }
    if (fits_write_col(fptr, TSTRING, 2, firstrow, 1, nrows, name, &status)) {
        fits_report_error(stderr, status);
        return status;
    }
    if (fits_write_col(fptr, TINT, 3, firstrow, 1, nrows, age, &status)) {
        fits_report_error(stderr, status);
        return status;
    }

    // Close FITS file
    if (fits_close_file(fptr, &status)) {
        fits_report_error(stderr, status);
        return status;
    }

    std::cout << "Created " << fits_file << std::endl;

    return 0;
}
