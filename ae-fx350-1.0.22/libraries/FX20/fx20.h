#ifndef _FX20
#define _FX20

#include <Wire.h>
/*
ABCDEFG

  A
F   B
  G
E   C
  D  DP
  
  
byte: DP G F E D C B A
*/

namespace
{
#define ADDRESS 0x71 // This device

	// First byte, sanity check
#define COMMAND_BYTE 0xFF 

// Set the display command
#define SET_COMMAND 0x00
// Set the brightness command
#define BRIGHTNESS_COMMAND 0x01

	const byte numArray[10] PROGMEM =
	{
		0x3F, /* 0 */
		0x06, /* 1 */
		0x5B, /* 2 */
		0x4F, /* 3 */
		0x66, /* 4 */
		0x6D, /* 5 */
		0x7D, /* 6 */
		0x07, /* 7 */
		0x7F, /* 8 */
		0x6F, /* 9 */
	};

	const byte charTable[128] PROGMEM =
	{
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x3F,
		0x6,
		0x5B,
		0x4F,
		0x66,
		0x6D,
		0x7D,
		0x7,
		0x7F,
		0x6F,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x77,
		0x7F,
		0x39,
		0x3F,
		0x79,
		0x71,
		0x3D,
		0x76,
		0x6,
		0x1E,
		0x76,
		0x38,
		0x55,
		0x54,
		0x3F,
		0x73,
		0x67,
		0x77,
		0x6D,
		0x31,
		0x3E,
		0x72,
		0x7E,
		0x52,
		0x6E,
		0x5B,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x5F,
		0x7C,
		0x58,
		0x5E,
		0x7B,
		0x71,
		0x6F,
		0x74,
		0x4,
		0xE,
		0x74,
		0x30,
		0x55,
		0x54,
		0x5C,
		0x73,
		0x67,
		0x50,
		0x6D,
		0x78,
		0x1C,
		0x62,
		0x6A,
		0x52,
		0x66,
		0x5B,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
	};

	byte displayBuffer[4] = { 0, 0, 0, 0 };
}

namespace FX20
{
	enum Dot
	{
		NODOT = 0,
		DOT = 1
	};

	void init()
	{
		Wire.begin();
	}

	void setRaw(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
	{
		displayBuffer[0] = byte1;
		displayBuffer[1] = byte2;
		displayBuffer[2] = byte3;
		displayBuffer[3] = byte4;

		Wire.beginTransmission(ADDRESS);
		Wire.write(COMMAND_BYTE);
		Wire.write(SET_COMMAND);
		Wire.write(byte1);
		Wire.write(byte2);
		Wire.write(byte3);
		Wire.write(byte4);
		Wire.endTransmission();
	}

	void setRaw()
	{
		setRaw(displayBuffer[0], displayBuffer[1], displayBuffer[2], displayBuffer[3]);
	}

	void update()
	{
		setRaw();
	}

	void setByte(uint8_t index, uint8_t byte)
	{
		if (index >= 4) return;

		displayBuffer[index] = byte;

		setRaw(displayBuffer[0], displayBuffer[1], displayBuffer[2], displayBuffer[3]);
	}

	void setDigits(uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4, Dot dot1 = NODOT, Dot dot2 = NODOT, Dot dot3 = NODOT, Dot dot4 = NODOT)
	{
		num1 %= 10;
		num2 %= 10;
		num3 %= 10;
		num4 %= 10;
		
		setRaw(pgm_read_byte(numArray+num1) | (dot1 << 7), pgm_read_byte(numArray+num2) | (dot2 << 7), pgm_read_byte(numArray+num3) | (dot3 << 7), pgm_read_byte(numArray+num4) | (dot4 << 7));
	}

	void queueDigit(byte index, byte num, Dot dot = NODOT)
	{
		if (index >= 4) return;

		displayBuffer[index] = pgm_read_byte(numArray+num) | (dot << 7);
	}

	void setDigit(byte index, byte num, Dot dot = NODOT)
	{
		queueDigit(index, num, dot);

		setRaw();
	}

	void queueCharacter(byte index, char character, Dot dot = NODOT)
	{
		if (index >= 4) return;

		displayBuffer[index] = pgm_read_byte(charTable+character) | dot << 7;
	}

	void setCharacter(byte index, char character, Dot dot = NODOT)
	{
		queueCharacter(index, character, dot);

		setRaw();
	}

	void setString(byte index, const char* string)
	{
		for (int i = index; i < 4; i++)
		{
			if (!string[i])
			{
				update();
				return;
			}

			if (string[i] == '.') continue;

			queueCharacter(index, string[i], (string[i+1] == '.')? DOT : NODOT);
		}

		update();
	}

	void setFloat(float numberToDisplay, byte precision = 3, int index = 0, bool clear = true)
	{
		precision = max(min(precision, 3), 1);
		index = max(min(index, 3), 0);
		char buffer[4];
		memset(buffer, 0, 4);

		snprintf(buffer+index, 4-index, "%f.*", precision, numberToDisplay);

		for (int i = 0; i < 4; i++)
		{
			queueCharacter(i, buffer[i]);
		}

		update();
	}

	void setNumber(uint32_t numberToDisplay, int rightIndex = 3, bool clear = true)
	{
		if (clear) memset(displayBuffer, 0, 4);
		if (rightIndex >= 4) rightIndex = 3;

		if (numberToDisplay == 0) setDigit(rightIndex, 0);

		while (numberToDisplay && rightIndex >= 0)
		{
			queueDigit(rightIndex, numberToDisplay % 10);
			numberToDisplay /= 10;
			rightIndex--;
		}

		update();
	}

	void setRange(uint32_t numberToDisplay, int index, int width)
	{
		width = max(min(width, 4), 1);

		memset(displayBuffer+index, 0, width);

		int mod = 1;
		for (int i = 0; i < width; i++) mod *= 10;

		numberToDisplay %= mod; //truncate digits

		setNumber(numberToDisplay, index + width - 1, false);
	}

	void setBrightness(uint16_t brightness)
	{
		Wire.beginTransmission(ADDRESS);
		Wire.write(COMMAND_BYTE);
		Wire.write(BRIGHTNESS_COMMAND);
		Wire.write(brightness >> 8);
		Wire.write(brightness);
		Wire.endTransmission();
	}

	void clear()
	{
		setRaw(0, 0, 0, 0);
	}
};

#endif //_FX20