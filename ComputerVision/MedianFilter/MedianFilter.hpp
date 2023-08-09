//
//  MedianFilter.hpp
//  ComputerVision
//
//  Created by Salvatore on 5/8/23.
//

#ifndef MedianFilter_hpp
#define MedianFilter_hpp

#include <stdio.h>

#include "../Utils/Constants.hpp"
#include "../Utils/Image.hpp"

bool comparePixelsByRGB(const Pixel& a, const Pixel& b) {
    float sum_a = a.r + a.g + a.b;
    float sum_b = b.r + b.g + b.b;
    return sum_a < sum_b;
}

Pixel applyMedianFilterToPixel(Image& img, int row, int col, int filter_size) {
    std::vector<Pixel> values;

    // Collect pixel values in the local neighborhood
    for (int i = -filter_size / 2; i <= filter_size / 2; ++i) {
        for (int j = -filter_size / 2; j <= filter_size / 2; ++j) {
            int r = row + i;
            int c = col + j;

            // Check if the pixel is within bounds
            if (r >= 0 && r < img.height && c >= 0 && c < img.width) {
                values.push_back(img.at(r, c));
            }
        }
    }

    // Calculate the median
    int middle = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + middle, values.end(), comparePixelsByRGB);
    return values[middle];
}

// Define a function to apply median filter to an RGB image
Image applyMedianFilterToImage(Image& image, int filter_size = 3) {
    int rows = image.height;
    int cols = image.width;

    Image filtered_image = image;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            filtered_image.at(r, c) = applyMedianFilterToPixel(image, r, c, filter_size);
        }
    }

    return filtered_image;
}
#endif /* MedianFilter_hpp */
