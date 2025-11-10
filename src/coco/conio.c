#include <stdbool.h>
#include <coco.h>

static char lastKey = 0;

unsigned char kbhit(void)
{
    return (char)(lastKey || (lastKey = inkey()) || (lastKey = inkey()));
}

char cgetc(void)
{
    char key = lastKey;

    lastKey = 0;

    while (!key)
    {
        key = (char)inkey();
    }

    return key;
}
