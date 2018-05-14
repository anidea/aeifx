#pragma once
#include "Arduino.h"

//#define DEBUG_RFID

enum class RFIDReadStyle : uint8_t
{
	EVENODD = 0,
	SEQUENTIAL = 1,
	IMMEDIATE = 2
};

struct FX200Configuration
{
	const RFIDReadStyle readStyle;
	const uint8_t pin;
	HardwareSerial* const fx200Serial;
	const uint16_t readTime;
	const uint16_t expireTime;

	FX200Configuration(RFIDReadStyle readStyle, uint8_t pin, HardwareSerial* fx200Serial, uint16_t readTime = 200, uint16_t expireTime = 2000)
	:	readStyle(readStyle),
		pin(pin),
		fx200Serial(fx200Serial),
		readTime(readTime),
		expireTime(expireTime) {}
};

class TagData;

template <int count>
class FX200
{
	TagData tagData[count];
	FX200Configuration& config;

public:
	FX200(FX200Configuration& config);

	void scan(byte index = 0);
	
	bool isSolved();
	bool hasNewData(int index = -1);
	bool learn();
	void init();

	String getTag(byte index);

private:
	void scanEvenOdd();
	void scanImmediate(byte index);
	void scanSequencial();
	void rfidGetVersion(byte index);
	void rfidGetImmediateRead(byte index);

	void rfidGetRead(byte index);
	void rfidSetTrigger(byte index, int ms);
	void rfidSetReadTime(byte index, int ms);
	void rfidSetExpireTime(byte index, int ms);

	void parseResponse(const char * string);

	void rfidSendCommand(byte index, const char * string);
	void RS485_SendMessage(char * pMessage);
	void RS485_GetMessage();	
};

class TagData
{
public:
	TagData()
	{
		memset(tagBuffer, 0, 32);
		strcpy(tagBuffer, "NONE");
		memset(solutionBuffer, 0, 32);
	}

	bool hasTag()
	{
		#ifdef DEBUG_RFID
		Serial.println(tagBuffer);
		#endif
		if (strcmp(tagBuffer, "NONE") == 0) return false;
		if (strcmp(tagBuffer, "EXPIRED") == 0) return false;
		return true;
	}

	String getTag()
	{
		newData = false;
		return String(tagBuffer);
	}

	bool hasNewData()
	{
		bool ret = newData;
		newData = false;
		return ret;
	}

	void setTag(const char* data)
	{
		if (strcmp(tagBuffer, data))
		{
			newData = true;
			strcpy(tagBuffer, data);
		}
	}

	bool isSolved()
	{
		return (!strcmp(tagBuffer, solutionBuffer));
	}

	void setSolvedTag()
	{
		if (hasTag())
		{
			strcpy(solutionBuffer, tagBuffer);
		}
	}
private:
	char tagBuffer[32];
	char solutionBuffer[32];
	bool newData = false;
};

template <int count>
bool FX200<count>::hasNewData(int index)
{
	if (index >= count) return false;

	if (index == -1)
	{
		for (int i = 0; i < count; i++)
		{
			if (tagData[i].hasNewData()) return true;
		}
		return false;
	}
	
	return tagData[index].hasNewData();
}
template <int count>
String FX200<count>::getTag(byte index)
{
	return tagData[index].getTag();
}

template <int count>
bool FX200<count>::isSolved()
{
	bool allSolved = true;

	for (int i = 0; i < count && allSolved; i++)
	{
		if (!tagData[i].isSolved()) allSolved = false;
	}

	return allSolved;
}

template <int count>
bool FX200<count>::learn()
{
	for (int i = 0; i < count; i++) if (!tagData[i].hasTag()) return false;
	for (int i = 0; i < count; i++) tagData[i].setSolvedTag();

	return true;
}

template <int count>
FX200<count>::FX200(FX200Configuration& config) : config(config)
{
	static_assert(count, "Must have at least one FX200 reader!");
	static_assert(count < 256, "Too many readers specified!");
}

template <int count>
void FX200<count>::init()
{
	config.fx200Serial->begin(115200);

	pinMode(config.pin, OUTPUT);
	digitalWrite(config.pin, LOW);

	delay(1000);

	for (int i = 0; i < count; i++)
	{
		#ifdef DEBUG_RFID
			rfidGetVersion(i);
			delay(config.readTime+50);
			RS485_GetMessage();
		#endif

		rfidSetReadTime(i, config.readTime);
		rfidSetExpireTime(i, config.expireTime);
	}
}

template <int count>
void FX200<count>::scan(byte index)
{
	static uint32_t lastTime = 0;
	uint32_t currentTime = millis();

	if (currentTime - lastTime > config.readTime || config.readStyle == RFIDReadStyle::IMMEDIATE)
	{
		lastTime = currentTime;
	}
	else
	{
		RS485_GetMessage();
		return;
	}

	switch (config.readStyle)
	{
	case RFIDReadStyle::EVENODD:
		scanEvenOdd();
		break;

	case RFIDReadStyle::SEQUENTIAL:
		scanSequencial();
		break;

	case RFIDReadStyle::IMMEDIATE:
		scanImmediate(index);
		break;
	}

	RS485_GetMessage();
}

template <int count>
void FX200<count>::scanEvenOdd()
{
	static bool SCAN = true;
	static byte index = 0;

	if (SCAN)
	{
		for (int i = 0; i < count; i++)
		{
			if (i % 2)
			{
				rfidSetTrigger(i, config.readTime);
			}
			else
			{
				rfidSetTrigger(i, 0);
			}
		}

		SCAN = false;
	}

	if (!SCAN)
	{
		for (int i = index; i < count; i += 2)
		{
			rfidGetRead(i);
		}
	}

	if (++index == 2 || index == count)
	{
		index = 0;
		SCAN = true;
	}
}

template <int count>
void FX200<count>::scanImmediate(byte index)
{
	rfidGetImmediateRead(index);
}

template <int count>
void FX200<count>::scanSequencial()
{
	static bool SCAN = true;
	static byte index = 0;
	
	if (SCAN)
	{
		for (int i = 0; i < count; i++)
		{
			rfidSetTrigger(i, config.readTime * i);
		}

		SCAN = false;
	}

	if (!SCAN)
	{
		rfidGetRead(index);
	}

	if (++index == count)
	{
		index = 0;
		SCAN = true;
	}
}

template <int count>
void FX200<count>::rfidGetVersion(byte index)
{
	rfidSendCommand(index, "VERSION");
	RS485_GetMessage();
}

template <int count>
void FX200<count>::rfidGetImmediateRead(byte index)
{
	rfidSendCommand(index, "REQRFID");
	RS485_GetMessage();
}

template <int count>
void FX200<count>::rfidGetRead(byte index)
{
	rfidSendCommand(index, "READRFID");
	RS485_GetMessage();
}

template <int count>
void FX200<count>::rfidSetTrigger(byte index, int ms)
{
	static char trigger_string[8];
	sprintf(trigger_string, "TRG%04d", ms);

	rfidSendCommand(index, trigger_string);
	RS485_GetMessage();
}

template <int count>
void FX200<count>::rfidSetReadTime(byte index, int ms)
{
	static char read_string[9];
	sprintf(read_string, "TIME%04d", ms);

	rfidSendCommand(index, read_string);
	RS485_GetMessage();
}

template <int count>
void FX200<count>::rfidSetExpireTime(byte index, int ms)
{
	static char expire_string[11];
	sprintf(expire_string, "EXPIRE%04d", ms);

	rfidSendCommand(index, expire_string);
	RS485_GetMessage();
}

template <int count>
void FX200<count>::parseResponse(const char * string)
{
	int uSrc = 0, uDest = 0;
	char cMsg[32];
	memset(cMsg, 0, 32);
	sscanf(string, "%d,%d,%s", &uSrc, &uDest, cMsg);

	if (uSrc >= count) return;

	#ifdef DEBUG_RFID
		Serial.println(string);
	#endif

	if (cMsg[0] == ':')
	{
		tagData[uSrc].setTag(cMsg+1);

		#ifdef DEBUG_RFID
			Serial.print(uSrc);
			Serial.print(" Tag Stored: ");
			Serial.println(tagData[uSrc].getTag());
		#endif
	}
}

template <int count>
void FX200<count>::rfidSendCommand(byte index, const char * string)
{
	static char tx_buffer[48];
	memset(tx_buffer, 0, 48);
	sprintf(tx_buffer, "###255,%d,%s###", index, string);
	RS485_SendMessage(tx_buffer);
}

template <int count>
void FX200<count>::RS485_SendMessage(char *pMessage)
{
	byte pos = 0;
	digitalWrite(config.pin, HIGH);
	delay(10);

	config.fx200Serial->write(pMessage);
	config.fx200Serial->flush();
	delay(2);

	digitalWrite(config.pin, LOW);

	delay(50);
}

template <int count>
void FX200<count>::RS485_GetMessage()
{
	static size_t pos = 0;
	static char rx_buffer[64];

	auto consumeCommand = [&](const char* response, size_t length)
	{
		static char buffer[32];

		strncpy(buffer, response, length);
		buffer[length] = 0;

		parseResponse(buffer);
	};

	while (config.fx200Serial->available())
	{
		rx_buffer[pos++] = (char)config.fx200Serial->read();

		char* startToken = (char*)memmem(rx_buffer, 64, "###", 3);
		char* endToken = (char*)memmem(rx_buffer, 64, "!!!", 3);

		if ((endToken && endToken < startToken) || (!startToken && endToken)) // If data is fubar, trash
		{
			#ifdef DEBUG_RFID
			Serial.println("Trashed");
			#endif

			memset(rx_buffer, 0, 64);
			pos = 0;
		}

		if (startToken && endToken)
		{
			startToken += 3;
			size_t msg_length = endToken - startToken;

			consumeCommand(startToken, msg_length);

			memset(rx_buffer, 0, 64);
			pos = 0;
		}
	}
}