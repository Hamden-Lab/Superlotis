#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

#define ELEMENT_SIZE 2

int res_width = 2048;
int res_height = 2048;

// Resizes the RAW file and returns resized data
std::vector<uint16_t> resize_raw(const char* filename) {
    std::cout << "Resize raw file to (" << res_width << "," << res_height << ")" << std::endl;

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return {};
    }

    int file_size = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    int num_elements = file_size / ELEMENT_SIZE;
    std::cout << "Original file size: " << file_size << " bytes" << std::endl;
    std::cout << "Element size: " << ELEMENT_SIZE << " bytes" << std::endl;
    std::cout << "Number of elements: " << num_elements << std::endl;

    std::vector<uint16_t> data(num_elements);
    file.read(reinterpret_cast<char*>(data.data()), num_elements * ELEMENT_SIZE);
    if (!file) {
        std::cerr << "Error reading the file!" << std::endl;
        return {};
    }
    file.close();

    int target_elements = res_width * res_height;
    std::vector<uint16_t> resized_data(target_elements);

    if (num_elements < target_elements) {
        std::copy(data.begin(), data.end(), resized_data.begin());
        std::fill(resized_data.begin() + num_elements, resized_data.end(), 0);
        std::cout << "Padding with " << (target_elements - num_elements) << " zeros" << std::endl;
    } else if (num_elements > target_elements) {
        std::copy(data.begin(), data.begin() + target_elements, resized_data.begin());
        std::cout << "Trimming data by: " << (num_elements - target_elements) << " elements" << std::endl;
    } else {
        resized_data = std::move(data);
        std::cout << "No resizing needed." << std::endl;
    }

    return resized_data;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.raw>" << std::endl;
        return 1;
    }

    const char* input_file = argv[1];
    auto resized_data = resize_raw(input_file);
    if (resized_data.empty()) {
        return 1;
    }
}