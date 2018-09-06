#pragma once

#ifndef _ANALOG_ADC_DMA_H
#define _ANALOG_ADC_DMA_H


typedef uint16_t adc_data_t;


typedef struct {
	uint16_t btctrl;
	uint16_t btcnt;
	uint32_t srcaddr;
	uint32_t dstaddr;
	uint32_t descaddr;
} dmacdescriptor;


class AnalogAdcDma
{
public:
	AnalogAdcDma(int pin, int size);
	AnalogAdcDma();

	void begin();

	void adc_dma(void * rxdata, size_t hwords);
		
	bool done(void) { return AnalogAdcDma::dmaDone; };

	static bool dmaDone;

private:
	
	void DMAC_Handler();
	void dma_init();
	
	void adc_init();
	
	adc_data_t* adcBuf;



	int inputPin;

	uint32_t chnl;  // DMA channel



	volatile dmacdescriptor wrb[12] __attribute__((aligned(16)));
	dmacdescriptor descriptor_section[12] __attribute__((aligned(16)));
	dmacdescriptor descriptor __attribute__((aligned(16)));
};

#endif
