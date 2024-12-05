#include <stm32l432xx.h>


uint32_t SystemCoreClock = 4000000UL;   // reset default using MSI clock @ 4 MHz


void SystemInit(void) {
    // SystemInit is automagically called from the startup code before main(). 
    // enable FPU, needed for printf and your own floating point stuff
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2)); // Set CP10 and CP11 Full Access
}


void SystemClock_Config(void) {
    // Enable HSI clock
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));  // Wait for HSI to be ready

    // Enable Power Interface clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;  // Enable Power Interface clock

    // Configure Voltage Scaling to Range 1
    PWR->CR1 |= PWR_CR1_VOS_0;  // Set voltage scaling to Range 1 (VOS bits set to 01)
    while ((PWR->SR2 & PWR_SR2_VOSF) != 0);  // Wait until voltage scaling is ready

    // Configure Flash latency
    FLASH->ACR |= FLASH_ACR_LATENCY_4WS;  // Set Flash latency for 80 MHz

    // Configure PLL
    RCC->PLLCFGR = 0;  // Reset PLL configuration
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;  // Set PLL source to HSI
    RCC->PLLCFGR |= (10 << RCC_PLLCFGR_PLLN_Pos);  // Set PLL multiplication factor (N = 10)
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLM_Pos);  // Set PLL pre-division factor (M = 1)
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLR_Pos);  // Set PLL post-division factor (R = 2)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;  // Enable PLL output

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));  // Wait for PLL to be ready

    // Switch system clock to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait for the switch to complete

    SystemCoreClock = 16000000UL / 1UL * 10UL / 2UL;      //    / M * N / R
}
