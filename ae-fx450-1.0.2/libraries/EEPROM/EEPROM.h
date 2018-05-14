#pragma once

#include "I2C_EEPROM.h"

#define MEMORY_SIZE 0x8000

struct EEPROMClass
{
	bool init = false;

	I2C_eeprom ee{ 0x50, MEMORY_SIZE };

    uint8_t read( int idx )
	{
		checkInit();
		return ee.readByte(idx);
	}
    void write( int idx, uint8_t val )
	{
		checkInit();
		ee.writeByte(idx, val);
	}
	
	void checkInit()
	{
		if (!init)
		{
			init = true;
			ee.begin();
		}
		
		delay(50);
	}
};

static EEPROMClass EEPROM;