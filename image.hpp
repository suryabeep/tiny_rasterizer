#pragma once

#include "geometry.hpp"
#include <string.h>
#include <cfloat>

class Image {
public:
    unsigned int _width, _height;
    unsigned int *pixels;
    float *zbuffer;

public:
    Image() {

    }

    Image(unsigned int width , unsigned int height) {
        _width = width;
        _height = height;
        pixels = new unsigned int[_width * _height];
        zbuffer = new float[_width * _height];
        clear();
    }

    ~Image() {
        delete pixels;
        delete zbuffer;

    }

    virtual void setPixel(unsigned int x, unsigned int y, Vec3i RGB, float zDepth){
         unsigned int color = 0;

        color |= RGB.x;
        color |= (RGB.y << 8);
        color |= (RGB.z << 16);


        unsigned int idx = y * _width + x; 


        if (zDepth > zbuffer[idx]) {
            pixels[idx] = color;
            zbuffer[idx] = zDepth;
        } 
    }

    virtual void clear() {
        memset(pixels, 0, _width * _height * sizeof(unsigned int));

        //can't use memset for floats
        for(int i = 0; i < (_width * _height); i++) {
            zbuffer[i] = -FLT_MAX;
        }
    }
};