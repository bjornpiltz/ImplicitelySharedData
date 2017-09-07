#include "Image.h"

struct ImageData
{
    ImageData(int width=0, int height=0, ColorSpace colorSpace = ColorSpace::Invalid)
        : width(width)
        , height(height)
        , colorSpace(colorSpace)
    {
    }
    int width, height;
    ColorSpace colorSpace;
};

Image::Image()
{
}

Image::Image(int width, int height, ColorSpace colorspace)
   : d(width, height, colorspace)
{
}

Image::Image(const char* filename)
    : d(128, 128, ColorSpace::RGB)
{
}

bool Image::write(const char* filename)const
{
    return true;
}

ColorSpace Image::colorspace()const
{
    return d->colorSpace;
}

int Image::height()const
{
    return d->height;
}

int Image::width()const
{
    return d->width;
}

bool Image::isValid()const
{
    return d->width>0 && d->height>0 && d->colorSpace != ColorSpace::Invalid;
}

void Image::scale(int newWidth, int newHeight)&
{
    if (width()==newWidth && height()==newHeight)
        return;

    // The following line triggers a copy if necessary.
    d->width = newWidth;
    d->height = newHeight;
}

Image Image::scaled(int width, int height)const&
{
    return Image(*this).scaled(width, height);
}

Image Image::scaled(int width, int height)&&
{
    scale(width, height);
    return std::move(*this);
}

void Image::convertToGray()&
{
    if (colorspace() != ColorSpace::Gray)
        d->colorSpace = ColorSpace::Gray;
}

Image Image::asGray()const&
{
    return Image(*this).asGray();
}

Image Image::asGray()&&
{
    convertToGray();
    return std::move(*this);
}
