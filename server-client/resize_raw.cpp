#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <iomanip>

#define ELEMENT_SIZE 2

int res_width = 2048;
int res_height = 2048;

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

void write_fits_header(std::ofstream& fits_file, int width, int height) {
    const int card_size = 80;
    std::vector<std::string> cards;

    auto format_card = [](const std::string& key, const std::string& value, const std::string& comment = "") {
        std::ostringstream oss;
        oss << std::left << std::setw(8) << key << "= " << std::right << std::setw(20) << value << " ";
        if (!comment.empty()) oss << "/ " << comment;
        std::string card = oss.str();
        card.resize(80, ' ');
        return card;
    };

    cards.push_back(format_card("SIMPLE", "T", "Standard FITS format"));
    cards.push_back(format_card("BITPIX", "16", "16-bit signed integer"));
    cards.push_back(format_card("NAXIS", "2", "Number of axes"));
    cards.push_back(format_card("NAXIS1", std::to_string(width), "Width"));
    cards.push_back(format_card("NAXIS2", std::to_string(height), "Height"));
    cards.push_back(format_card("END", ""));

    // Write cards and pad to 2880 bytes
    std::string header_data;
    for (const auto& card : cards) {
        header_data += card;
    }

    while (header_data.size() % 2880 != 0) {
        header_data += ' ';
    }

    fits_file.write(header_data.c_str(), header_data.size());
}

void write_fits_data(std::ofstream& fits_file, const std::vector<uint16_t>& data) {
    fits_file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint16_t));

    std::streamoff data_size = data.size() * sizeof(uint16_t);
    int pad = 2880 - (data_size % 2880);
    if (pad != 2880) {
        std::vector<char> padding(pad, 0);
        fits_file.write(padding.data(), pad);
    }
}

void save_as_fits(const std::string& output_file, const std::vector<uint16_t>& data, int width, int height) {
    std::ofstream fits_file(output_file, std::ios::binary);
    if (!fits_file) {
        std::cerr << "Error opening output FITS file!" << std::endl;
        return;
    }

    write_fits_header(fits_file, width, height);
    write_fits_data(fits_file, data);

    fits_file.close();
    std::cout << "FITS file saved: " << output_file << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.raw> <output.fits>" << std::endl;
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    auto resized_data = resize_raw(input_file);
    if (resized_data.empty()) {
        return 1;
    }

    save_as_fits(output_file, resized_data, res_width, res_height);
    return 0;
}

//compile: g++ -std=c++11 -o convert_raw_to_fits resize_raw.cpp