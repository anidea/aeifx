/*
Heavily modified by El_Supremo (Arduino forums) to decode DTMF tones
It is also public domain and provided on an AS-IS basis. There's no warranty
or guarantee of ANY kind whatsoever.

The Goertzel algorithm is long standing so see
http://en.wikipedia.org/wiki/Goertzel_algorithm for a full description.
It is often used in DTMF tone detection as an alternative to the Fast
Fourier Transform because it is quick with low overheard because it
is only searching for a single frequency rather than showing the
occurrence of all frequencies.
This work is entirely based on the Kevin Banks code found at
http://www.eetimes.com/design/embedded/4024443/The-Goertzel-Algorithm
so full credit to him for his generic implementation and breakdown. I've
simply massaged it into an Arduino library. I recommend reading his article
for a full description of whats going on behind the scenes.

Created by Jacob Rosenthal, June 20, 2012.
Released into the public domain.
*/

// ensure this library description is only included once
#ifndef DTMF_h
#define DTMF_h

// include types & constants of Wiring core API
#include "Arduino.h"

#include <analog_adc_dma.h>

constexpr auto DTMF_ADC_DATA_SIZE = 160;

// library interface description
class DTMF
{
  // user-accessible "public" interface
  public:
    DTMF(float,float);
	~DTMF();
	//void sample(void);
	//void sample(int sensorPin);
	void detect(adc_data_t* sampleData, float dtmf_mag[],int adc_centre);

	const static int sampleLength = DTMF_ADC_DATA_SIZE;
	
char button(float mags[],float magnitude);
  // library-accessible "private" interface
  private:


	char last_dtmf = 0;

	void ProcessSample(int,int);
	void ResetDTMF(void);

	float SAMPLING_RATE;
	float TARGET;
	int N;
	float coeff[8];
	float Q1[8];
	float Q2[8];
	float cosine;

};

#endif

