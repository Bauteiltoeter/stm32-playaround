#include "main.h"
#include <stdint.h>



#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "LC24.h"

static uint16_t __timer = 0;
static int timer_bt1=0;
static int timer_bt2=0;
static int timer_bt3=0;
static unsigned int time=0;

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

void loadRandomImage(lc24_t* lc)
{
    int  i=rand()%3;

    switch(i)
    {
        case 0:
        lc24_loadImage(lc, img_ohyou);
        lc24_setBackground(lc,lc24_bg_bright_green);
        break;
        case 1:
        lc24_loadImage(lc, img_rebel);
        lc24_setBackground(lc,lc24_bg_bright_green);
        break;
        case 2:
        lc24_loadImage(lc, img_nuke);
        lc24_setBackground(lc,lc24_bg_bright_green);
        break;

    }


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

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitDef);

    // Call USART1 configuration
    USART_Config();

    // Send "Hello, World!" to PC

    lc24_init();

    Delay(1000);

    lc24_t* lc1, *lc2,*lc3;
    lc1 = lc24_getDisplay(0);
    lc2 = lc24_getDisplay(1);
    lc3 = lc24_getDisplay(2);

    lc24_loadImage(lc1, img_do_not_touch);
    lc24_loadImage(lc2, img_do_not_touch);
    lc24_loadImage(lc3, img_do_not_touch);


    while (1)
    {
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14)==0)
        {
            loadRandomImage(lc1);
            Delay(100);
            timer_bt1=1500;
        }

        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)==0)
        {
            loadRandomImage(lc2);
            Delay(100);
            timer_bt2=1500;
        }

        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==0)
        {
            loadRandomImage(lc3);
            Delay(100);
            timer_bt3=1500;
        }

        if(timer_bt1==0)
        {
            lc24_loadImage(lc1, img_do_not_touch);
            lc24_setBackground(lc1,lc24_bg_bright_red);
            timer_bt1=-1;
        }

        if(timer_bt2==0)
        {
            lc24_loadImage(lc2, img_do_not_touch);
            lc24_setBackground(lc2,lc24_bg_bright_red);
            timer_bt2=-1;
        }

        if(timer_bt3==0)
        {
            lc24_loadImage(lc3, img_do_not_touch);
            lc24_setBackground(lc3,lc24_bg_bright_red);
            timer_bt3=-1;
        }

        Delay(10);
    }
}

void DEBUG_writeString(char* str)
{
    char c;
    while((c=*str))
    {
        str++;
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
        // Send a char using USART1
        USART_SendData(USART1, c);
    }
}

void SysTick_Handler(void)
{
    time++;
    if (__timer > 0)
        __timer--;

    if(timer_bt1>0)
        timer_bt1--;
    if(timer_bt2>0)
        timer_bt2--;
    if(timer_bt3>0)
        timer_bt3--;
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
