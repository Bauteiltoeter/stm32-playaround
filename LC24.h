#include <stdint.h>

#define FIFO_SIZE 512
#define LCD_WIDTH 36
#define LCD_HEIGHT 24

typedef enum
{
    FIFO_EMPTY,
    FIFO_NORM,
    FIFO_FULL
} fifo_status_t;

typedef struct
{
    uint16_t data[FIFO_SIZE];
    int write;
    int read;
    fifo_status_t status;

} fifo_t;


typedef struct
{
    uint16_t datapin;
    uint8_t pixmap[108];
    fifo_t* fifo;

} lc24_t;

typedef enum
{
    lc24_bg_off = 0x00,
    lc24_bg_dark_green = 0x03,
    lc24_bg_bright_green = 0x33,
    lc24_bg_dark_red = 0x0C,
    lc24_bg_bright_red = 0xCC,
    lc24_bg_dark_orange = 0x0F,
    lc24_bg_bright_orange = 0xFF,
    lc24_bg_greenish_orange = 0x3F,
    lc24_bg_reddish_orange = 0xCF
} lc24_bg_t;

void lc24_init(void);
lc24_t* lc24_getDisplay(int number);
void lc24_writeData(lc24_t* lc, uint8_t reg, uint8_t* data, int length);
void lc24_setPixel(lc24_t* lc, int x, int y);
void lc24_drawLine(lc24_t* lc, int x1, int y1, int x2, int y2);
void lc24_loadImage(lc24_t* lc, uint8_t* image);
void lc24_setBackground(lc24_t* lc, lc24_bg_t bg);

extern uint8_t img_nuke[];
extern uint8_t img_do_not_touch[];
extern uint8_t img_rebel[];
extern uint8_t img_ohyou[];
