#pragma once
#include "COW.h"

 enum class ColorSpace{Invalid, Gray, RGB};
/*
 * This is a fully fledged image class. It just lacks the pixels.
*/
class Image
{
public:
    Image();
    Image(int width, int height, ColorSpace colorspace);
    Image(const char* filename);

    bool write(const char* filename);

    ColorSpace colorspace()const;
    int height()const;
    int width()const;

    bool isValid()const;

    Image& scale(int width, int height);
    Image scaled(int width, int height)const;

    Image toGray()const;

private:
    COW<struct ImageData> d;
};