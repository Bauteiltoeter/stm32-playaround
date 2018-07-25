#include "main.h"
#include <stdint.h>



#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

static uint16_t __timer = 0;

void Delay(uint16_t n)
{
    __timer = n;
    while (__timer > 0);
}



int main()
{

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	GPIO_InitTypeDef init;
    init.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	init.GPIO_Mode = GPIO_Mode_OUT;
	init.GPIO_Speed = GPIO_Low_Speed;
	init.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOG,&init);
    GPIO_SetBits(GPIOG, GPIO_Pin_13);

	

    if (SysTick_Config(SystemCoreClock / 1000))
    { 
        while (1);
    }

    while (1) {

        GPIO_ToggleBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14);

        Delay(100);

    }
}

void SysTick_Handler(void)
{
    if (__timer > 0)
        __timer--;
} 

void assert_param(int x)
{

}
