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

bool Image::write(const char* filename)
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

Image& Image::scale(int width, int height)
{
    // Make sure the const access happens through the const reference
    // otherwise a detach might be triggered unnecessarily.
    const auto& data = d.constData();
    if (data.width==width && data.height==height)
        return *this;

    // Change is necessary.
    d->width = width;
    d->height = height;

    return *this;
}

Image Image::scaled(int width, int height)const
{
    return Image(*this).scale(width, height);
}

Image Image::toGray()const
{
    Image copy(*this);
    if (copy.colorspace() != ColorSpace::Gray)
        copy.d->colorSpace = ColorSpace::Gray;
    return copy;
}
