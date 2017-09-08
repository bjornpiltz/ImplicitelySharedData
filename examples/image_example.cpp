#include "Image.h"

/*
 * The following shows how to use the mockup Image class. 
 * The usage is concise and efficient in that no temporary copies are created.
 */
int main()
{
    bool success = Image("lenna.jpg").asGray().scaled(100, 100).write("thumb.jpg");
    return success ? 0 : 1;
}
