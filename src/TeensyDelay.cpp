#include "config.h"

namespace TeensyDelay
{
    void(*callbacks[maxChannel])(void);

    void begin()
    {
        if (USE_TIMER == TIMER_TPM1)                 // enable clocks for tpm timers (ftm clocks are enabled by teensyduino)
        {
            SIM_SCGC2 |= SIM_SCGC2_TPM1;
            SIM_SOPT2 |= SIM_SOPT2_TPMSRC(2);
        }
        else if (USE_TIMER == TIMER_TPM2)
        {
            SIM_SCGC2 |= SIM_SCGC2_TPM2;
            SIM_SOPT2 |= SIM_SOPT2_TPMSRC(2);
        }

        //Default Mode for  FTM is (nearly) TPM compatibile 
        timer->SC = FTM_SC_CLKS(0b00);              // Disable clock		        
        timer->MOD = 0xFFFF;                        // Full counter range

        for (unsigned i = 0; i < maxChannel; i++)
        {                                           // (teensyduino enabled them for PMW generation)
            if (isFTM) {
                timer->CH[i].SC &= ~FTM_CSC_CHF;    // FTM requires to clear flag by setting bit to 0    
            }
            else {
                timer->CH[i].SC |= FTM_CSC_CHF;     // TPM requires to clear flag by setting bit to 1
            }
            timer->CH[i].SC &= ~FTM_CSC_CHIE;       // Disable channel interupt
            timer->CH[i].SC = 0;                    // disable channel
        }

        timer->SC = FTM_SC_CLKS(0b01) | FTM_SC_PS(prescale);  // Start clock
        NVIC_ENABLE_IRQ(irq);                        // Enable interrupt request for selected timer
    }

    void addDelayChannel(void(*callback)(void), const int channel = 0)
    {     
        callbacks[channel] = callback;               //Just store the callback function, the rest is done in Trigger function
    }
}

//-------------------------------------------------------------------------------------------
// Interupt service routine of the timer selected in config.h. 
// The code doesn't touch the other FTM/TPM ISRs so they can still be used for other purposes
//
// Unfortunately we can not inline the ISR because inlinig will generate a "weak" function. 
// Since the original ISR (dummy_isr) is also defined weak the linker
// is allowed to choose any of them. In this case it desided to use dummy_isr :-(
// Using a "strong" (not inlined) function overrides the week dummy_isr
//--------------------------------------------------------------------------------------------

using namespace TeensyDelay;

#if USE_TIMER == TIMER_FTM0
void ftm0_isr(void)
#elif USE_TIMER == TIMER_FTM1
void ftm1_isr(void)
#elif USE_TIMER == TIMER_FTM2
void ftm2_isr(void)
#elif USE_TIMER == TIMER_FTM3
void ftm3_isr(void)
#elif USE_TIMER == TIMER_TPM1
void tpm1_isr(void)
#elif USE_TIMER == TIMER_TPM2
void tpm2_isr(void)
#endif
{  
    uint32_t status = TeensyDelay::timer->STATUS & 0x0F;   // STATUS collects all channel event flags (bit0 = ch0, bit1 = ch1....) 

    unsigned i = 0;
    while (status > 0)
    {        
        if (status & 0x01)
        {
            if (isFTM)
            {                                                        
                timer->CH[i].SC &= ~(FTM_CSC_CHF | FTM_CSC_CHIE);   // reset channel and interrupt enable (we only want one shot per trigger)
            }
            else
            {
                timer->CH[i].SC |= FTM_CSC_CHF;
                timer->CH[i].SC &= ~FTM_CSC_CHIE;
            }
       
            TeensyDelay::callbacks[i]();							         // invoke callback function for the channel								                     
        }       
        i++;
        status >>= 1;
    }
}
