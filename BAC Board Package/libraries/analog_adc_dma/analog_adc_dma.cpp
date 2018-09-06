// adcdma
//  analog A1
//   could use DAC to provide input voltage   A0
//   http://www.atmel.com/Images/Atmel-42258-ASF-Manual-SAM-D21_AP-Note_AT07627.pdf pg 73

#include "Arduino.h"
#include "analog_adc_dma.h"





bool AnalogAdcDma::dmaDone = false;

AnalogAdcDma::AnalogAdcDma(int pin, int size)
{
	adcBuf = new adc_data_t(size);
	inputPin = pin;

	chnl = 0;

	Serial.println("AnalogAdcDma Init");
}

AnalogAdcDma::AnalogAdcDma()
{
	delete adcBuf;
}

void AnalogAdcDma::begin()
{
	adc_init();
	dma_init();

}

void DMAC_Handler() {
	// interrupts DMAC_CHINTENCLR_TERR DMAC_CHINTENCLR_TCMPL DMAC_CHINTENCLR_SUSP
	uint8_t active_channel;

	// disable irqs ?
	__disable_irq();
	active_channel = DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk; // get channel number
	DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
	AnalogAdcDma::dmaDone = DMAC->CHINTFLAG.reg;
	DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL; // clear
	DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
	DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;

	__enable_irq();

	//Serial.println("AnalogAdcDma DMAC_Handler");
}


void AnalogAdcDma::dma_init() {
	// probably on by default
	PM->AHBMASK.reg |= PM_AHBMASK_DMAC;
	PM->APBBMASK.reg |= PM_APBBMASK_DMAC;
	
	NVIC_EnableIRQ(DMAC_IRQn);

	DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
	DMAC->WRBADDR.reg = (uint32_t)wrb;
	DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);

	//Serial.println("AnalogAdcDma dma_init");
}

void AnalogAdcDma::adc_dma(void *rxdata, size_t hwords) {
	uint32_t temp_CHCTRLB_reg;

	DMAC->CHID.reg = DMAC_CHID_ID(chnl);
	DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
	DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
	DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << chnl));
	temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) |
		DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY) | DMAC_CHCTRLB_TRIGACT_BEAT;
	DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
	DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK; // enable all 3 interrupts
	AnalogAdcDma::dmaDone = 0;
	descriptor.descaddr = 0;
	descriptor.srcaddr = (uint32_t)&ADC->RESULT.reg;
	descriptor.btcnt = hwords;
	descriptor.dstaddr = (uint32_t)rxdata + hwords * sizeof(adc_data_t);   // end address
	descriptor.btctrl = DMAC_BTCTRL_BEATSIZE_HWORD | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
	memcpy(&descriptor_section[chnl], &descriptor, sizeof(dmacdescriptor));

	// start channel
	DMAC->CHID.reg = DMAC_CHID_ID(chnl);
	DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;

	//Serial.println("AnalogAdcDma adc_dma");
}

static __inline__ void ADCsync() __attribute__((always_inline, unused));
static void   ADCsync() {
	while (ADC->STATUS.bit.SYNCBUSY == 1); //Just wait till the ADC is free
}


void AnalogAdcDma::adc_init() {
	analogRead(inputPin);  // do some pin init  pinPeripheral()
	ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
	ADCsync();
	//ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0_Val; //  2.2297 V Supply VDDANA
	ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;      // Gain select as 1X
	//ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;  // default
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
	ADCsync();    //  ref 31.6.16
	ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[inputPin].ulADCChannelNumber;
	ADCsync();
	ADC->AVGCTRL.reg = 0x05;       // 16 sample averaging
	ADC->SAMPCTRL.reg = 32; ; //sample length in 1/2 CLK_ADC cycles
	ADCsync();
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 | ADC_CTRLB_FREERUN | ADC_CTRLB_RESSEL_10BIT;
	ADCsync();
	ADC->CTRLA.bit.ENABLE = 0x01;
	ADCsync();

	//Serial.println("AnalogAdcDma adc_init");
}

/*
void setup() {
	Serial.begin(9600);
	analogWriteResolution(10);
	analogWrite(A0, 64);   // test with DAC 
	adc_init();
	dma_init();
}

void loop() {
	uint32_t t;

	t = micros();
	adc_dma(adcbuf, HWORDS);
	while (!dmadone);  // await DMA done isr
	t = micros() - t;
	Serial.print(t);  Serial.print(" us   ");
	Serial.println(adcbuf[0]);
	delay(2000);
}
*/

