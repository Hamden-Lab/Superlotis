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




//compile: g++ -std=c++11 -o [filename] resize_raw.cpp