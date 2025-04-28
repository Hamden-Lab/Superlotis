#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#define ELEMENT_SIZE 2
#define WIDTH 2048
#define HEIGHT 2048

#define CARD_SIZE 80
#define MAX_CARDS 32
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



// void add_card(char cards[MAX_CARDS][CARD_SIZE], int* card_count, const char* key, const char* value, const char* comment) {
//     char formatted[CARD_SIZE + 1];
//     memset(formatted, ' ', CARD_SIZE);
//     formatted[CARD_SIZE] = '\0';

//     if (comment && comment[0] != '\0') {
//         snprintf(formatted, CARD_SIZE + 1, "%-8s= %20s / %s", key, value, comment);
//     } else {
//         snprintf(formatted, CARD_SIZE + 1, "%-8s= %20s", key, value);
//     }

//     memcpy(cards[*card_count], formatted, CARD_SIZE);
//     (*card_count)++;
// }

// int write_fits_header(FILE* file) {
//     if (file == NULL) {
//         return -1;
//     }

//     char cards[MAX_CARDS][CARD_SIZE];
//     int card_count = 0;

//     char width_str[16];
//     char height_str[16];
//     snprintf(width_str, sizeof(width_str), "%d", WIDTH);
//     snprintf(height_str, sizeof(height_str), "%d", HEIGHT);

//     add_card(cards, &card_count, "SIMPLE", "T", "Standard FITS format");
//     add_card(cards, &card_count, "BITPIX", "16", "16-bit signed integer");
//     add_card(cards, &card_count, "NAXIS", "2", "Number of axes");
//     add_card(cards, &card_count, "NAXIS1", width_str, "Width");
//     add_card(cards, &card_count, "NAXIS2", height_str, "Height");
//     add_card(cards, &card_count, "END", "", "");

//     int i;
//     for (i = 0; i < card_count; ++i) {
//         if (fwrite(cards[i], 1, CARD_SIZE, file) != CARD_SIZE) {
//             return -1;
//         }
//     }

//     int total_bytes = card_count * CARD_SIZE;
//     int pad = 2880 - (total_bytes % 2880);
//     if (pad != 2880) {
//         char* padding = (char*)calloc(pad, 1);
//         if (!padding) return -1;
//         if (fwrite(padding, 1, pad, file) != pad) {
//             free(padding);
//             return -1;
//         }
//         free(padding);
//     }

//     return 0;
// }

// int write_fits_data(FILE* file, const char* data, int num_pixels) {
//     int total_bytes = num_pixels * ELEMENT_SIZE;

//     if (fwrite(data, 1, total_bytes, file) != total_bytes) {
//         return -1;
//     }

//     int pad = 2880 - (total_bytes % 2880);
//     if (pad != 2880) {
//         char* padding = (char*)calloc(pad, 1);
//         if (!padding) return -1;
//         if (fwrite(padding, 1, pad, file) != pad) {
//             free(padding);
//             return -1;
//         }
//         free(padding);
//     }

//     return 0;
// }

// int save_as_fits(const char* output_file, const char* data) {
//     FILE* file = fopen(output_file, "wb");
//     if (!file) {
//         printf("Error opening output FITS file!\n");
//         return -1;
//     }

//     extern int write_fits_header(FILE*);  // or define it above

//     int header_result = write_fits_header(file);
//     if (header_result != 0) {
//         fclose(file);
//         return -1;
//     }

//     int result = write_fits_data(file, data, WIDTH * HEIGHT);
//     fclose(file);

//     if (result == 0) {
//         printf("FITS file saved: %s\n", output_file);
//     }

//     return result;
// }


int main(){
    if (resize_raw("/opt/PrincetonInstruments/picam/samples/server-client-simple/bin/exposure_file.raw") != 0){
        std::cout << "No resizing needed." << std::endl;
        
    }
}

// const char* input_file = "./bin/exposure_file.raw";
// const char* output_file = "./bin/exposure_file.fits";
    
// int main(int argc, char* argv[]) {


//     // char* resized = nullptr;
//     // int resized_size = 0;

//     // if (resize_raw("image.raw", 512, 512, &resized, &resized_size) == 0) {
//     //     // Use resized data here
//     //     std::cout << "Resized data has " << resized_size << " bytes" << std::endl;
//     //     // Don't forget to free memory when done
//     //     std::free(resized);
//     // }


//     // save_as_fits(output_file, resized, res_width, res_height);
//     // return 0;


// }









//compile: g++ -std=c++11 -o convert_raw_to_fits resize_raw.cpp