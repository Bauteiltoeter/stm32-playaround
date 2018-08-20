#include "LC24.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "misc.h"




static void lc24_writeByte(lc24_t* lc, uint8_t byte, uint8_t should_parity);//0: even, 1: odd
static void lc24_writePixmap(lc24_t* lc);

void fifo_push(fifo_t* fifo, uint16_t value);
void fifo_pop(fifo_t* fifo, uint16_t* value);


static fifo_t datafifo1;
static fifo_t datafifo2;
static fifo_t datafifo3;

static lc24_t button1;
static lc24_t button2;
static lc24_t button3;

uint8_t img_nuke[]= {0x08,0xF0,0xFF,0x21,0x94,0xC1,0xE9,0x10,0x62,0x03,0x07,0x00,0x04,0x63,0x30,0x00,0xC0,0x10,0xC0,0x00,0x00,0x08,0x08,0x0E,0x00,0x00,0xC3,0x63,0x00,0x00,0x30,0x06,0x03,0x00,0x00,0x22,0x10,0x00,0x00,0x40,0x10,0x01,0x00,0x00,0x04,0x11,0x00,0x00,0x60,0x00,0x01,0x0E,0x02,0xC2,0x10,0x9E,0x71,0x20,0x36,0x32,0x10,0x09,0x01,0xC0,0x00,0x91,0x11,0x00,0x00,0x10,0xE1,0xF0,0x04,0x00,0x11,0x00,0x60,0x00,0x18,0x03,0x00,0x02,0x80,0x20,0xC0,0x30,0x00,0x08,0x02,0x18,0x01,0xC0,0x60,0x00,0x07,0x00,0x06,0x0C,0x40,0x00,0x30,0x80,0x01,0x00,0xE0,0x01,0xF0,0x00,};
uint8_t img_do_not_touch[] = {0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0xF0,0x04,0x00,0x00,0x80,0xF8,0x1C,0x1E,0x70,0x84,0x64,0x93,0x81,0x4D,0x48,0x22,0x11,0x88,0x84,0x24,0x12,0x81,0x48,0x48,0x36,0x11,0xD8,0x88,0xC7,0x11,0x01,0x07,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x0E,0xE7,0xF1,0x20,0x12,0x89,0x98,0x0D,0x38,0x9C,0x0F,0x89,0xE0,0x70,0x80,0x90,0x08,0x22,0x91,0x0C,0xD9,0xC0,0xE1,0x70,0x10,0x0F,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,};
uint8_t img_rebel[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x38,0x21,0x00,0x80,0xC8,0x16,0x02,0x00,0x88,0x44,0x12,0x00,0x80,0x48,0x24,0x01,0x00,0x98,0x6C,0x0C,0x00,0x80,0x87,0xC3,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x04,0x00,0x00,0x01,0x40,0x00,0x00,0x10,0x00,0x04,0x00,0x00,0x71,0x78,0x9C,0x07,0x90,0xC8,0x26,0x62,0x00,0xF9,0x44,0x3E,0x04,0x10,0x48,0x04,0x42,0x00,0xC9,0x6C,0x32,0x04,0x10,0x87,0xC7,0x41,0x00,0x00,0x00,0x00,0x00,};
uint8_t img_ohyou[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0xC2,0x01,0x00,0x00,0x20,0x22,0x00,0x00,0xE0,0x33,0x06,0x00,0x00,0x33,0x41,0x00,0x00,0x10,0x12,0x04,0x00,0x00,0x21,0x41,0x00,0x00,0x10,0x32,0x06,0x00,0x20,0x21,0x22,0x00,0x00,0x12,0xC2,0x01,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x38,0x21,0x00,0x80,0xC8,0x16,0x02,0x00,0x88,0x44,0x12,0x00,0x80,0x48,0x24,0x01,0x00,0x98,0x6C,0x0C,0x00,0x80,0x87,0xC3,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};

void fifo_push(fifo_t* fifo, uint16_t value)
{
    if(fifo->status == FIFO_FULL)
    {
            DEBUG_writeString("FIFO full\r\n”");
        return;
    }

    fifo->data[fifo->write] = value;
    fifo->write++;

    if(fifo->write>=FIFO_SIZE)
        fifo->write=0;

    if(fifo->read==fifo->write)
        fifo->status = FIFO_FULL;
    else
        fifo->status = FIFO_NORM;
}

void fifo_pop(fifo_t* fifo, uint16_t* value)
{
    if(fifo->status == FIFO_EMPTY)
    {

        *value=0xFFFF;
        return;
    }

    *value = fifo->data[fifo->read];
    fifo->read++;

    if(fifo->read>=FIFO_SIZE)
        fifo->read=0;

    if(fifo->read==fifo->write)
        fifo->status = FIFO_EMPTY;
    else
        fifo->status = FIFO_NORM;

}

void lc24_writeByte(lc24_t* lc, uint8_t byte, uint8_t should_parity) //0: even, 1: odd
{
    uint16_t word=0xFFFF & ~(0x1FF<<1);
    uint8_t parity=0;
    uint8_t paritybit=0;

    for(int i=0; i < 8; i++)
    {
        if(byte&(1<<i))
            parity^=0xFF;
    }

    //parity = 0 means even, 1 means odd

    if(parity == 0 && should_parity == 0)
        paritybit = 0;
    else if (parity != 0 && should_parity == 0 )
        paritybit = 1;
    else if (parity == 0 && should_parity == 1 )
        paritybit = 1;
    else if (parity != 0 && should_parity == 1 )
        paritybit = 0;


    word |= byte<<1;
    word &=~(1<<0);
    word |= paritybit<<9;
    word |= (1<<10) | 1<<11;

    fifo_push(lc->fifo, word);
}

void lc24_writeData(lc24_t *lc, uint8_t reg, uint8_t* data, int length)
{
    lc24_writeByte(lc,0x00,0); //startbyte with even
    lc24_writeByte(lc,reg,1); //Write register with odd

    for(int i=0; i < length; i++)
        lc24_writeByte(lc,data[i],1);

    lc24_writeByte(lc,0xAA,0); //endbyte with even
}

void lc24_init(void)
{
    DEBUG_writeString("\r\nHallo?\r\n");
    datafifo1.status=FIFO_EMPTY;
    datafifo1.read =0;
    datafifo1.write=0;
    datafifo2.status=FIFO_EMPTY;
    datafifo2.read =0;
    datafifo2.write=0;
    datafifo3.status=FIFO_EMPTY;
    datafifo3.read =0;
    datafifo3.write=0;

    button1.datapin = GPIO_Pin_2;
    button1.fifo = &datafifo1;
    button2.datapin = GPIO_Pin_3;
    button2.fifo = &datafifo2;
    button3.datapin = GPIO_Pin_1;
    button3.fifo = &datafifo3;

    static uint8_t test[2];
    test[0] = 0xFF;
    test[1] = 0x00;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = button1.datapin |  button2.datapin| button3.datapin | GPIO_Pin_4 ; //4: Clock, 1-: Data
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &GPIO_InitStruct);
/*
DMA_InitTypeDef dma;

    dma.DMA_Channel = DMA_Channel_0;
    dma.DMA_PeripheralBaseAddr = &GPIOD->ODR;
    dma.DMA_Memory0BaseAddr = &test;
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma.DMA_BufferSize = 2;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc =     DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize =     DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_Priority = DMA_Priority_Medium;
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0,&dma);

    DMA_Cmd(DMA2_Stream0,ENABLE);*/
   // DMA_ITConfig();


    //Timer config


    // Enable clock for TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // TIM2 initialization overflow every 500ms
    // TIM2 by default has clock of 84MHz
    // Here, we must set value of prescaler and period,
    // so update event is 0.5Hz or 500ms
    // Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) *
    // (TIM_Period + 1))
    // Update Event (Hz) = 84MHz / ((4199 + 1) * (9999 + 1)) = 2 Hz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 2;
    TIM_TimeBaseInitStruct.TIM_Period = 120;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    // TIM2 initialize
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    // Start TIM2
    TIM_Cmd(TIM2, ENABLE);

    // Nested vectored interrupt settings
    // TIM2 interrupt is most important (PreemptionPriority and
    // SubPriority = 0)
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);


    Delay(1000);
    uint8_t freq[2];
    uint8_t tmp=0xCC;
    lc24_writeData(&button1, 0xED,&tmp,1); //color
    lc24_writeData(&button2, 0xED,&tmp,1); //color
    lc24_writeData(&button3, 0xED,&tmp,1); //color

    tmp=0x10;
    lc24_writeData(&button1,0xEE,&tmp,1); //freq
    lc24_writeData(&button2,0xEE,&tmp,1); //freq
    lc24_writeData(&button3,0xEE,&tmp,1); //freq


    freq[0] = 0x07;
    freq[1] = 0x00;
    lc24_writeData(&button1,0xEF,freq,2); //mux1
    lc24_writeData(&button2,0xEF,freq,2); //mux1
    lc24_writeData(&button3,0xEF,freq,2); //mux1

    tmp=0x00;
    lc24_writeData(&button1,0xF0,&tmp,1); //mux2
    lc24_writeData(&button2,0xF0,&tmp,1); //mux2
    lc24_writeData(&button3,0xF0,&tmp,1); //mux2

    for(int i=0; i < 108; i++)
    {

        button1.pixmap[i]=0x00;
        button2.pixmap[i]=0x00;
        button3.pixmap[i]=0x00;

    }
    lc24_writePixmap(&button1);
    lc24_writePixmap(&button2);
    lc24_writePixmap(&button3);
}

void TIM2_IRQHandler()
{
    static int mode  = 0;
    static volatile uint16_t dataword1=0;
    static volatile uint16_t dataword2=0;
    static volatile uint16_t dataword3=0;
    static int shift=0;

    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIM2, TIM_IT_Update))
    {
        if(mode == 0)
        {
           GPIO_SetBits(GPIOF,GPIO_Pin_4); //set clock
           mode = 1;

           //update data bit
           if(dataword1&(1<<shift)) //set data
               GPIO_SetBits(GPIOF,button1.datapin);
           else
               GPIO_ResetBits(GPIOF,button1.datapin);

           if(dataword2&(1<<shift)) //set data
               GPIO_SetBits(GPIOF,button2.datapin);
           else
               GPIO_ResetBits(GPIOF,button2.datapin);

           if(dataword3&(1<<shift)) //set data
               GPIO_SetBits(GPIOF,button3.datapin);
           else
               GPIO_ResetBits(GPIOF,button3.datapin);

           shift++;
        }
        else
        {
            GPIO_ResetBits(GPIOF,GPIO_Pin_4); //set clock
            mode =0;

            if(shift > 15)
            {
                shift = 0;
                fifo_pop(button1.fifo, &dataword1);
                fifo_pop(button2.fifo, &dataword2);
                fifo_pop(button3.fifo, &dataword3);
            }
        }

        // Clears the TIM2 interrupt pending bit
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}


void lc24_setPixel(lc24_t *lc, int x, int y)
{
    unsigned int px = (LCD_WIDTH-1-x) + y*LCD_WIDTH;
    unsigned int byte = px/8;
    unsigned int bitoffset = px%8;
    lc->pixmap[byte] |= 1<<bitoffset;
    lc24_writeData(lc, 0x80+byte,&lc->pixmap[byte],1);

}

void lc24_writePixmap(lc24_t *lc)
{
    lc24_writeData(lc, 0x80,&lc->pixmap,108);
}

void lc24_drawLine(lc24_t* lc, int x1, int y1, int x2, int y2)
{
    if(y1==y2)
    {
        //horizontal line
        do {
            if(x1%8 ==0)
            {
                if(x2-x1>=8)
                {
                    lc->pixmap[x1/8 + (y1*LCD_WIDTH/8)]=0xFF;
                    x1+=8;
                }
                else
                {
                    unsigned int pixels=0;
                    int pixelcount = x2-x1;
                    for(unsigned int i=0; i <= pixelcount; i++)
                        pixels|=(1<<i);
                    lc->pixmap[x1/8+ (y1*(LCD_WIDTH/8))]|=pixels;
                    x1+=pixelcount;

                }
            }
        } while(x1<x2);


    }

    lc24_writePixmap(lc);
}

void lc24_loadImage(lc24_t* lc, uint8_t* image)
{
    for(int i=0; i < 108; i++)
          lc->pixmap[i] = image[i];

    lc24_writePixmap(lc);
}

lc24_t *lc24_getDisplay(int number)
{
    switch(number)
    {
    case 0: return &button1; break;
    case 1: return &button2; break;
    case 2: return &button3; break;
    }
}

void lc24_setBackground(lc24_t *lc, lc24_bg_t bg)
{
    uint8_t tmp=bg;
    lc24_writeData(lc, 0xED,&tmp,1); //color
}
