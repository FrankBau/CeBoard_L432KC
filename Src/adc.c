#include <stm32l432xx.h>

#include <adc.h>

#include <gpio.h>

void adc_init(void) __attribute__((constructor));

void adc_init(void)
{
    // the GPIO pins used for ADC are already in analog mode after reset
    // no need to re-config GPIO here (unless used before)

    // let ADC (digital block) be clocked by: SYSCLK
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_ADCSEL_Msk) | (3 << RCC_CCIPR_ADCSEL_Pos);

    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // turn ADC clock on
    (void)RCC->AHB2ENR;                // read back to make sure that clock is on

    ADC1->CR &= ~ADC_CR_DEEPPWD; // disable deep power-down
    ADC1->CR |= ADC_CR_ADVREGEN; // power up ADC voltage regulator

    // wait  t_ADCVREG_STUP (ADC voltage regulator start-up time),
    for (volatile int i = 0; i < 4 * 100; ++i)
        ; // min 100 µs see data sheet

    // config
    ADC1->CFGR &= ~ADC_CFGR_RES;   // Set resolution to 12 bits
    ADC1->CFGR &= ~ADC_CFGR_ALIGN; // Right alignment
    ADC1->CFGR &= ~ADC_CFGR_CONT;  // Single conversion mode

    // do self calibration
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
        ; // wait for calibration to finish
    uint8_t calibration_factor = ADC1->DR;
    ADC1->CALFACT = calibration_factor;

    // "enable the ADC" procedure from RM0490 Rev 3:
    ADC1->ISR |= ADC_ISR_ADRDY; //  Clear the ADRDY bit in ADC_ISR register
    ADC1->CR |= ADC_CR_ADEN;    //  Set ADEN = 1 in the ADC_CR register.
    while (!(ADC1->ISR & ADC_ISR_ADRDY))
        ; //  Wait until ADRDY = 1 in the ADC_ISR register
}

// this could be measured by using the internal bandgap VREFINT
#define Vdda_mV 3300

// measure the analog voltage at pin PA0..PA7.
// returns the measured voltage in millivolts
uint32_t adc_measure(uint8_t Pxy)
{
    uint32_t channel = 5 + GPIO_PIN(Pxy);        // ADC1_IN5 is PA0 and so on
    ADC1->SQR1 &= ~ADC_SQR1_L;                   // set sequence length to 1 conversion
    ADC1->SQR1 |= (channel << ADC_SQR1_SQ1_Pos); // set channel 9 as the first and only conversion channel

    ADC1->CR |= ADC_CR_ADSTART; // start ADC conversion
    while (!(ADC1->ISR & ADC_ISR_EOC))
        ;                             // wait for end of conversion
    uint32_t adc_data_raw = ADC1->DR; // conversion done. store result

    uint32_t adc_data_mV = (adc_data_raw * Vdda_mV) / 4095; // Vdda == 4095 digital reading

    return adc_data_mV;
}
