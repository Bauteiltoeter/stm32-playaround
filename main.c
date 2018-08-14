#include "main.h"
#include <stdint.h>



#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

static uint16_t __timer = 0;

void Delay(uint16_t n)
{
    __timer = n;
    while (__timer > 0);
}

void LC24_writeWord(uint16_t word);

void DEBUG_writeString(char* str);

void USART_Config(void)
{
    // Enable clock for GPIOB
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // Enable clock for USART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // Connect PB6 to USART1_Tx
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
    // Connect PB7 to USART1_Rx
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

    // Initialization of GPIOB
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialization of USART1
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_HardwareFlowControl =
            USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStruct);

    // Enable USART1
    USART_Cmd(USART1, ENABLE);
}



int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1);
    }

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);


    GPIO_InitTypeDef GPIO_InitDef;
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitDef);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitDef);

    // Call USART1 configuration
    USART_Config();

    // Send "Hello, World!" to PC

    lc24_init();

    Delay(1000);



    uint8_t pixmap[108];
int j=0;
    while (1)
    {
GPIO_SetBits(GPIOG,GPIO_Pin_13);

        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0))
        {

          //  GPIO_SetBits(GPIOG,GPIO_Pin_14);


        }
        else
        {
            uint8_t tmp=0x33;
            lc24_writeData(0xED,&tmp,1); //color


            //GPIO_ResetBits(GPIOG,GPIO_Pin_14);
uint8_t image[] = {0x08,0xF0,0xFF,0x21,0x94,0xC1,0xE9,0x10,0x62,0x03,0x07,0x00,0x04,0x63,0x30,0x00,0xC0,0x10,0xC0,0x00,0x00,0x08,0x08,0x0E,0x00,0x00,0xC3,0x63,0x00,0x00,0x30,0x06,0x03,0x00,0x00,0x22,0x10,0x00,0x00,0x40,0x10,0x01,0x00,0x00,0x04,0x11,0x00,0x00,0x60,0x00,0x01,0x0E,0x02,0xC2,0x10,0x9E,0x71,0x20,0x36,0x32,0x10,0x09,0x01,0xC0,0x00,0x91,0x11,0x00,0x00,0x10,0xE1,0xF0,0x04,0x00,0x11,0x00,0x60,0x00,0x18,0x03,0x00,0x02,0x80,0x20,0xC0,0x30,0x00,0x08,0x02,0x18,0x01,0xC0,0x60,0x00,0x07,0x00,0x06,0x0C,0x40,0x00,0x30,0x80,0x01,0x00,0xE0,0x01,0xF0,0x00,};
            lc24_loadImage(image);

        }






        Delay(10);



        //DEBUG_writeString("Hallo motherfucker\r\n");
    }
}

void DEBUG_writeString(char* str)
{
    char c;
    while( c=*str)
    {
        str++;
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
        // Send a char using USART1
        USART_SendData(USART1, c);
    }
}

void SysTick_Handler(void)
{
    if (__timer > 0)
        __timer--;
}

void assert_param(int x)
{
    if(!x)
    {
        while(1)
        {
            GPIO_ToggleBits(GPIOG,GPIO_Pin_14);
            Delay(100);
        }
    }
}
