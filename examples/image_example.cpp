#include "Image.h"

int main()
{
    return Image("lenna.jpg").asGray().scaled(100, 100).write("thumb.jpg") ? 0 : 1;
}