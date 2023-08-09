//
//  Utils.hpp
//  ComputeVision
//
//  Created by Salvatore on 25/7/23.
//

#ifndef Utils_hpp
#define Utils_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <jpeglib.h>

#include "Image.hpp"

using namespace std;

optional<Image> readJPG(string filename) {
    
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr err;

    unsigned long int imgWidth, imgHeight;
    int numComponents;

    unsigned long int dwBufferBytes;
    unsigned char* lpData;

    unsigned char* lpRowBuffer[1];

    FILE* fHandle;

    fHandle = fopen(filename.c_str(), "rb");
    if(fHandle == NULL) {
        fprintf(stderr, "%s:%u: Failed to read file %s\n", __FILE__, __LINE__, filename.c_str());
        return nullopt;
    }

    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);

    jpeg_stdio_src(&info, fHandle);
    jpeg_read_header(&info, TRUE);

    jpeg_start_decompress(&info);
    imgWidth = info.output_width;
    imgHeight = info.output_height;
    numComponents = info.num_components;

    dwBufferBytes = imgWidth * imgHeight * numComponents; /* We only read RGB, not A */
    lpData = (unsigned char*)malloc(sizeof(unsigned char)*dwBufferBytes);

    Image img((int)imgHeight, (int)imgWidth, numComponents);

    /* Read scanline by scanline */
    while(info.output_scanline < info.output_height) {
        lpRowBuffer[0] = (unsigned char *)(&lpData[numComponents*info.output_width*info.output_scanline]);
        jpeg_read_scanlines(&info, lpRowBuffer, 1);
    }

    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    fclose(fHandle);
    
    for(int r = 0; r < imgHeight; ++r) {
        for(int c = 0; c < imgWidth; ++c){
            
            for(int ch = 0; ch < numComponents; ++ch){

                int index = ((r * imgWidth + c) * numComponents) + ch;
                float channel = ((float)(lpData[index]) / 255.0);

                switch (ch) {
                    case 1:
                        img.at(r, c).g = channel;
                        break;
                    case 2:
                        img.at(r, c).b = channel;
                        break;
                    default:
                        img.at(r, c).r = channel;
                        break;
                }
            }
        }
    }

    return img;
}

inline int writeJPG(Image lpImage, string filename) {
    struct jpeg_compress_struct info;
    struct jpeg_error_mgr err;

    unsigned char* lpRowBuffer[1];

    FILE* fHandle;

    fHandle = fopen(filename.c_str(), "wb");
    if(fHandle == NULL) {
        #ifdef DEBUG
            fprintf(stderr, "%s:%u Failed to open output file %s\n", __FILE__, __LINE__, filename.c_str());
        #endif
        return 1;
    }

    info.err = jpeg_std_error(&err);
    jpeg_create_compress(&info);

    jpeg_stdio_dest(&info, fHandle);

    info.image_width = lpImage.width;
    info.image_height = lpImage.height;
    info.input_components = lpImage.channels;
    info.in_color_space = lpImage.channels == 1 ? JCS_GRAYSCALE : JCS_RGB;
    
    jpeg_set_defaults(&info);
    jpeg_set_quality(&info, 100, TRUE);

    jpeg_start_compress(&info, TRUE);
    unsigned char* lpData = new unsigned char[lpImage.width * lpImage.height * lpImage.channels];
    
    for(int ch = 0; ch < lpImage.channels; ++ch)
        for(int r = 0; r < lpImage.height; ++r)
            for(int c = 0; c < lpImage.width; ++c) {
                int index = ((r * lpImage.width + c) * lpImage.channels) + ch;
                switch (ch) {
                    case 2:
                        lpData[index] = (unsigned char)(255.0 * lpImage.at(r, c).b);
                        break;
                    case 1:
                        lpData[index] = (unsigned char)(255.0 * lpImage.at(r, c).g);
                        break;
                    default:
                        lpData[index] = (unsigned char)(255.0 * lpImage.at(r, c).r);
                        break;
                }
            }

    /* Write every scanline ... */
    while(info.next_scanline < info.image_height) {
        lpRowBuffer[0] = &(lpData[info.next_scanline * (lpImage.width * lpImage.channels)]);
        jpeg_write_scanlines(&info, lpRowBuffer, 1);
    }

    jpeg_finish_compress(&info);
    fclose(fHandle);

    jpeg_destroy_compress(&info);
    return 0;
}

inline void applySaltAndPepperNoise(Image& image, double noise_ratio = 0.05) {
    
    int rows = image.height;
    int cols = image.width;
    int num_pixels = rows * cols;

    int num_noise_pixels = static_cast<int>(num_pixels * noise_ratio);

    for (int i = 0; i < num_noise_pixels; ++i) {
        int row = rand() % rows;
        int col = rand() % cols;

        if (rand() % 2 == 0) {
            // Salt noise (white)
            image.at(row, col) = {1.0, 1.0, 1.0};
        } else {
            // Pepper noise (black)
            image.at(row, col) = {0, 0, 0};
        }
    }
}

inline void applyShotNoise(Image& image, double noise_factor = 0.2) {
    int rows = image.height;
    int cols = image.width;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            
            float noise = (noise_factor * (rand() % 256)) / 255.0;
            auto noisy_pixel = image.at(r, c);
            noisy_pixel = noisy_pixel + noise;
            noisy_pixel.clamp();
            image.at(r, c) = noisy_pixel;
            
        }
    }
}

void applySpeckleNoise(Image& image, double noise_factor = 0.2) {
    int rows = image.height;
    int cols = image.width;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double noise = noise_factor * (2.0 * (rand() / double(RAND_MAX)) - 1.0);  // Generate random noise
            auto noisy_pixel = image.at(r, c) + noise;
            noisy_pixel.clamp();
            image.at(r, c) = noisy_pixel;
        }
    }
}

// Define a function to apply Gaussian noise to a single pixel value
void applyGaussianNoiseToPixel(Pixel &pixel, double mean, double stddev) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distribution(mean, stddev);

    float noise = distribution(gen);
    pixel = pixel + noise;
    pixel.clamp();
    //return std::max(0.0f, std::min(1.0f, noisy_pixel_value)); // Ensure valid range [0, 1]
}

// Define a function to apply Gaussian noise to an RGB image
void applyGaussianNoiseToImage(Image& image, double mean = 0.0, double stddev= 0.1) {
    int rows = image.height;
    int cols = image.width;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto pixel = image.at(r, c);
            applyGaussianNoiseToPixel(pixel, mean, stddev);
            image.at(r, c) = pixel;
        }
    }
}

void normalizeImage(Image &img, float scalar) {
    for(int r = 0; r < img.height; ++r) {
        for(int c = 0; c < img.width; ++c) {
            img.at(r, c) = img.at(r, c) / scalar;
        }
    }
}

void drawRectangle(Image &img, int r, int c, int height, int width) {
    float intensity = 0.5;
    
    for(int rr = r; rr < r + height; ++rr) {
        for(int cc = c; cc < c + width; ++cc) {
            if( rr >= img.height || cc >= img.width ) continue;
            
            auto &pix = img.at(rr, cc);
            
            pix.r = pix.r * (1.0 - intensity);
            pix.g = pix.g + intensity;
            pix.b = pix.b * (1.0 - intensity);
            pix.clamp();
            img.at(r, c) = pix;
        }
    }
}

#endif /* Utils_hpp */
