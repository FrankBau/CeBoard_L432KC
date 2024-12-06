#include <stm32l432xx.h>

#include <tone.h>

#include <delay.h>

void tone_init() __attribute__((constructor));

extern uint32_t SystemCoreClock;

void tone_init()
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN; // enable peripheral clock
    (void)RCC->APB2ENR;                  // ensure that the last write command finished and the clock is on

    TIM15->PSC = (SystemCoreClock / 1000000UL) - 1; // prescaler divider SYSCLK -> 1 MHz

    // 0b00 CH1  channel is configured as output
    TIM15->CCMR1 = (TIM15->CCMR1 & ~TIM_CCMR1_CC1S_Msk) | (0 << TIM_CCMR1_CC1S_Pos);

    // 0b0110 CH2  PWM mode 1: In up-counting, channel 1 is active as long as CNT < CCR1
    TIM15->CCMR1 = (TIM15->CCMR1 & ~TIM_CCMR1_OC2M_Msk) | (6 << TIM_CCMR1_OC2M_Pos);
    TIM15->CCMR1 |= TIM_CCMR1_OC2PE; // OC preload enable (see reference manual)
    TIM15->CCER |= TIM_CCER_CC2E;    // enable channel and channel output pin

    // AF14 for PA3 is TIM15_CH2 see data sheet table "alternate function mapping"
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL3_Msk) | (14 << GPIO_AFRL_AFSEL3_Pos); // AF14
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE3_Msk) | (2 << GPIO_MODER_MODE3_Pos);    // 0b10=AF mode

    TIM15->ARR = 0;  // set length of PWM period
    TIM15->CCR2 = 0; // set length of active (high) phase of PWM pulse

    TIM15->EGR = TIM_EGR_UG; // generate update event to set internal PSC and ARR registers

    TIM15->BDTR |= TIM_BDTR_MOE; // master output enable (for timer with break functions)
    TIM15->CR1 |= TIM_CR1_CEN;   // enable the timer
}

// play a tone of frequency hz [Hertz] for a duration ms [milliseconds]
void tone(int hz, unsigned int ms)
{
    if (hz > 0.0f)
    {
        TIM15->CNT = 0;
        TIM15->ARR = (uint16_t)(1000000UL / hz);
        TIM15->CCR2 = (uint16_t)(500000UL / hz);
        TIM15->CR1 |= TIM_CR1_CEN; // enable the timer
        delay(ms);
        TIM15->CR1 &= ~TIM_CR1_CEN; // disable the timer
        delay(10);                  // staccato
    }
}
