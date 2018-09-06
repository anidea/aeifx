/*
Based on: https://github.com/jacobrosenthal/Goertzel
Heavily modified by Pete (El_Supremo on Arduino forums) to decode DTMF tones
It is also public domain and provided on an AS-IS basis. There's no warranty
or guarantee of ANY kind whatsoever.
This uses Digital Pin 4 to allow measurement of the sampling frequency

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



// include core Wiring API
#include "Arduino.h"

// include this library's description file
#include "DTMF.h"






const int dtmf_tones[8] = {
  697,
  770,
  852,
  941,
 1209,
 1336,
 1477,
 1633
};

const unsigned char dtmf_map[16] = {
  0x11,
  0x21,
  0x41,
  0x12,
  0x22,
  0x42,
  0x14,
  0x24,
  0x44,
  0x28,
  0x81,
  0x82,
  0x84,
  0x88,
  0x18,
  0x48
};

const char dtmf_char[16] = {
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '0',
  'A',
  'B',
  'C',
  'D',
  '*',
  '#',
};


DTMF::DTMF(float BLOCK,float SAMPLING_FREQ)
{


	SAMPLING_RATE = SAMPLING_FREQ;	//on 16mhz, ~8928.57142857143, on 8mhz ~44444
//>>>  TARGET=TARGET_FREQUENCY; //must be integer of SAMPLING_RATE/N
	 N  =BLOCK;	//Block size

  float	omega;
  // Calculate the coefficient for each DTMF tone
  for(int i = 0;i < 8;i++) {
    omega = (2.0 * PI * dtmf_tones[i]) / SAMPLING_RATE;
// DTMF detection doesn't need the phase.
// Computation of the magnitudes (which DTMF does need) does not
// require the value of the sin.
// not needed    sine = sin(omega);
    coeff[i] = 2.0 * cos(omega);
  }
  ResetDTMF();
}

DTMF::~DTMF()
{

}

/* Call this routine before every "block" (size=N) of samples. */
void DTMF::ResetDTMF(void)
{
  for(int i=0; i<8 ; i++) {
    Q2[i] = 0;
    Q1[i] = 0;
  }
}


/* Call this routine for every sample. */
//El_Supremo - change to int (WHY was it byte??)
void DTMF::ProcessSample(int sample,int adc_centre)
{
  float Q0;
//EL_Supremo subtract adc_centre to offset the sample correctly
  for(int i=0;i < 8;i++) {
    Q0 = coeff[i] * Q1[i] - Q2[i] + (float) (sample - adc_centre);
    Q2[i] = Q1[i];
    Q1[i] = Q0;
  }
}




// return the magnitudes of the 8 DTMF frequencies
void DTMF::detect(adc_data_t* sampleData, float dtmf_mag[],int adc_centre)
{
  int index;
  float d_tmp;


  /* Process the samples. */
  for (index = 0; index < N; index++)
  {
    ProcessSample(sampleData[index],adc_centre);
  }
  // Calculate the magnitude of each tone.
  for (int i=0;i < 8;i++) {
// El_Supremo 150318 the compilers in Arduino verisons 1.6.0 and 1.6.1
// generated "unable to find a register to spill" error in the original statement
// here. Breaking it into pieces worked around the problem.
//     dtmf_mag[i] = sqrt(Q1[i]*Q1[i] + Q2[i]*Q2[i] - coeff[i]*Q1[i]*Q2[i]);

  	// This is the equivalent of sqrt(real*real + imag*imag)
  	d_tmp = Q1[i]*Q1[i];
  	d_tmp += Q2[i]*Q2[i];
  	d_tmp -= coeff[i]*Q1[i]*Q2[i];

    dtmf_mag[i] = sqrt(d_tmp);
  }
  ResetDTMF();
}



// Detect which button was pressed using magnitude as the
// cutoff. Returns the character or a zero
char DTMF::button(float mags[],float magnitude)
{
  int bit = 1;
  int j;
  int dtmf = 0;



  for(int i=0;i<8;i++) {
    if(mags[i] > magnitude) {
      dtmf |= bit;
    }
    bit <<= 1;
  }
  for(j=0;j<16;j++) {
    if(dtmf_map[j] == dtmf)break;
  }
  if(j < 16) {
    // wait for the button to be released
    if(dtmf_char[j] == last_dtmf)return((char) 0);
    last_dtmf = dtmf_char[j];
    return(dtmf_char[j]);
  }
  last_dtmf = 0;
  return((char) 0);
}