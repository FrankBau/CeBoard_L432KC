#include <tone.h>


#include <stm32l432xx.h>


// assume that all relevant clocks (SYSCLK, HCLK, PCLK, TPCLK are 4 MHz)


void tone_init() __attribute__((constructor));


void tone_init() {
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;   // enable peripheral clock
    (void)RCC->APB2ENR;  // ensure that the last write command finished and the clock is on

    TIM15->PSC = 4 - 1;     // prescaler (divider 4:1) 4 MHz -> 1 MHz

    // 0b00 CH1  channel is configured as output
    TIM15->CCMR1 = (TIM15->CCMR1 &~TIM_CCMR1_CC1S_Msk) | (0 << TIM_CCMR1_CC1S_Pos);

    // 0b0110 CH2  PWM mode 1: In up-counting, channel 1 is active as long as CNT < CCR1
    TIM15->CCMR1 = (TIM15->CCMR1 &~TIM_CCMR1_OC2M_Msk) | (6<<TIM_CCMR1_OC2M_Pos);
    TIM15->CCMR1 |= TIM_CCMR1_OC2PE; 	// OC preload enable (see reference manual)
    TIM15->CCER |= TIM_CCER_CC2E;     	// enable channel and channel output pin

    // AF14 for PA3 is TIM15_CH2 see data sheet table "alternate function mapping" 
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL3_Msk) | (14 << GPIO_AFRL_AFSEL3_Pos); // AF14
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE3_Msk) | (2 << GPIO_MODER_MODE3_Pos); // 0b10=AF mode

    TIM15->ARR = 0;     // set length of PWM period
    TIM15->CCR2 = 0;    // set length of active (high) phase of PWM pulse
    
    TIM15->BDTR |= TIM_BDTR_MOE;    // master output enable (for timer with break functions)
    TIM15->CR1 |= TIM_CR1_CEN;      // enable the timer
}


// play a tone of frequency for a duration in milliseconds
void tone(float frequency, unsigned int duration_ms) {
    if(frequency > 0.0f) {
        TIM15->ARR  = (uint16_t)(1000000UL/frequency);
        TIM15->CCR2 = (uint16_t)( 500000UL/frequency);
        TIM15->CR1 |= TIM_CR1_CEN;    // enable the timer
        for(volatile unsigned int i=0; i<duration_ms*500UL; ++i);
        TIM15->CR1 &= ~TIM_CR1_CEN;    // disable the timer
    }
}
