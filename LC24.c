#include "LC24.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

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


static void lc24_writeByte(uint8_t byte, uint8_t should_parity);//0: even, 1: odd
static void lc24_writePixmap(void);

void fifo_push(fifo_t* fifo, uint16_t value);
void fifo_pop(fifo_t* fifo, uint16_t* value);

uint8_t pixmap[108];

static volatile fifo_t datafifo;

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

void lc24_writeByte(uint8_t byte, uint8_t should_parity) //0: even, 1: odd
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

    fifo_push(&datafifo, word);
}

void lc24_writeData(uint8_t reg, uint8_t* data, int length)
{
    lc24_writeByte(0x00,0); //startbyte with even
    lc24_writeByte(reg,1); //Write register with odd

    for(int i=0; i < length; i++)
        lc24_writeByte(data[i],1);

    lc24_writeByte(0xAA,0); //endbyte with even
}

void lc24_init(void)
{
    DEBUG_writeString("\r\nHallo?\r\n");
    datafifo.status=FIFO_EMPTY;
    datafifo.read =0;
    datafifo.write=0;

    static uint8_t test[2];
    test[0] = 0xFF;
    test[1] = 0x00;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 ; //4: Clock, 1-: Data
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
    lc24_writeData(0xED,&tmp,1); //color

    tmp=0x10;
    lc24_writeData(0xEE,&tmp,1); //freq


    freq[0] = 0x07;
    freq[1] = 0x00;
    lc24_writeData(0xEF,freq,2); //mux1

    tmp=0x00;
    lc24_writeData(0xF0,&tmp,1); //mux2

    for(int i=0; i < 108; i++)
        pixmap[i]=0x00;


    lc24_writeData(0x80,&pixmap,108);

    for(int i=0; i < 108; i++)
        pixmap[i] = 0;

    lc24_writePixmap();
}

void TIM2_IRQHandler()
{
    static int mode  = 0;
    static volatile uint16_t dataword=0;
    static int shift=0;

    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIM2, TIM_IT_Update))
    {
        if(mode == 0)
        {
           GPIO_SetBits(GPIOF,GPIO_Pin_4); //set clock
           mode = 1;

           //update data bit
           if(dataword&(1<<shift)) //set data
           {
               GPIO_SetBits(GPIOF,GPIO_Pin_1);
               GPIO_SetBits(GPIOF,GPIO_Pin_2);
               GPIO_SetBits(GPIOF,GPIO_Pin_3);
           }
           else
           {
               GPIO_ResetBits(GPIOF,GPIO_Pin_2);
               GPIO_ResetBits(GPIOF,GPIO_Pin_1);
               GPIO_ResetBits(GPIOF,GPIO_Pin_3);
            }
           shift++;
        }
        else
        {
            GPIO_ResetBits(GPIOF,GPIO_Pin_4); //set clock
            mode =0;

            if(shift > 15)
            {
                shift = 0;
                fifo_pop(&datafifo, &dataword);
            }
        }

        // Clears the TIM2 interrupt pending bit
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}


void lc24_setPixel(int x, int y)
{
    unsigned int px = (LCD_WIDTH-1-x) + y*LCD_WIDTH;
    unsigned int byte = px/8;
    unsigned int bitoffset = px%8;
    pixmap[byte] |= 1<<bitoffset;
    lc24_writeData(0x80+byte,&pixmap[byte],1);

}

void lc24_writePixmap(void)
{
    lc24_writeData(0x80,&pixmap,108);
}

void lc24_drawLine(int x1, int y1, int x2, int y2)
{
    if(y1==y2)
    {
        //horizontal line
        do {
            if(x1%8 ==0)
            {
                if(x2-x1>=8)
                {
                    pixmap[x1/8 + (y1*LCD_WIDTH/8)]=0xFF;
                    x1+=8;
                }
                else
                {
                    unsigned int pixels=0;
                    int pixelcount = x2-x1;
                    for(unsigned int i=0; i <= pixelcount; i++)
                        pixels|=(1<<i);
                    pixmap[x1/8+ (y1*(LCD_WIDTH/8))]|=pixels;
                    x1+=pixelcount;

                }
            }
        } while(x1<x2);


    }

    lc24_writePixmap();
}

void lc24_loadImage(uint8_t* image)
{
 for(int i=0; i < 108; i++)
          pixmap[i] = image[i];

      lc24_writePixmap();
}
