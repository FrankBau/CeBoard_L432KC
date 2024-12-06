#include <stm32l432xx.h>


uint32_t SystemCoreClock = 4000000UL;   // reset default using MSI clock @ 4 MHz


// constructor priorities from 0 to 100 are reserved
void SystemClock_Config_HSI_80MHz(void) __attribute__((constructor(200)));


void SystemClock_Config_HSI_80MHz(void) {
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

    // Configure PLL, see ref.man Figure 13. Clock tree
    // f(VCO clock) = f(PLL clock input) × (PLLN / PLLM)
    // f(PLL_R) = f(VCO clock) / PLLR
    RCC->PLLCFGR = 0;                               // Reset PLL configuration
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;         // Set PLL source to HSI
    RCC->PLLCFGR |= (10 << RCC_PLLCFGR_PLLN_Pos);   // Set PLL multiplication factor (N = 10)
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLM_Pos);    // Set PLL pre-division factor (M = 1)
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLLR_Pos);    // Set PLL post-division factor (R = 2)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;             // Enable PLL output

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));  // Wait for PLL to be ready

    // Switch system clock to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait for the switch to complete

    SystemCoreClock = 16000000UL / 1UL * 10UL / 2UL;      //    / M * N / R
}


// route SYSCLK to MCO pin PA8 for measurement
void init_MCO(void) {
    RCC->CFGR = (RCC->CFGR &~RCC_CFGR_MCOPRE_Msk) | (4 << RCC_CFGR_MCOPRE_Pos); // MCO divided by 2^4 == 16
    RCC->CFGR = (RCC->CFGR &~RCC_CFGR_MCOSEL_Msk) | (1 << RCC_CFGR_MCOSEL_Pos); // SYSCLK -> MCO

    // PA8 AF0 -> MCO
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // enable clock for peripheral component 
    (void)RCC->AHB2ENR;                  // ensure that the last write command finished and the clock is on
    GPIOA->AFR[1] = (GPIOA->AFR[1] &~GPIO_AFRH_AFSEL8_Msk) | (0 << GPIO_AFRH_AFSEL8_Pos);           // AF 0 
    GPIOA->MODER = (GPIOA->MODER &~GPIO_MODER_MODE8_Msk) | (2 << GPIO_MODER_MODE8_Pos);             // AF mode
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR &~GPIO_OSPEEDR_OSPEED8_Msk) | (3 << GPIO_OSPEEDR_OSPEED8_Pos); // very high speed 
}


void SystemInit(void) {
    // SystemInit is automagically called from the startup code before main(). 
    // enable FPU, needed for printf and your own floating point stuff
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2)); // Set CP10 and CP11 Full Access
}
