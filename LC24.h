#include <stdint.h>

typedef struct
{
    uint16_t datapin;

    uint8_t pixmap[108];

} lc24_t;

void lc24_init(void);
void lc24_writeData(uint8_t reg, uint8_t* data, int length);
void lc24_setPixel(int x, int y);
void lc24_drawLine(int x1, int y1, int x2, int y2);
void lc24_loadImage(uint8_t* image);
