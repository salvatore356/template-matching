//
//  Image.hpp
//  ComputeVision
//
//  Created by Salvatore on 25/7/23.
//

#ifndef Image_hpp
#define Image_hpp

#include <stdio.h>

#include "Pixel.hpp"

using namespace std;

struct Image {
    vector<Pixel> pixels;
    int width, height, channels;

    Image(int height, int width, int channels) {
        this->width = width;
        this->height = height;
        this->channels = channels;
        pixels.resize(height * width);
    }
    
    Pixel &at(int y, int x) {
        return pixels[y * width + x];
    }
    
};
#endif /* Image_hpp */
