#include "fx200.h"
#include "fx450.h"

// Number of readers
const int numberOfReaders = 1;

// Our FX200 configuration
FX200Configuration config
{
  /*  Read Style:*/ RFIDReadStyle::SEQUENTIAL, // EVENODD, SEQUENTIAL, or IMMEDIATE (used with user controlled timings)
  /*   RS485 Pin:*/ RS485_ENABLE, // This pin is defined in fx300.h and fx450.h
  /*  RS485 Uart:*/ &Serial5, // This is the UART we want to use
  /*   Read Time:*/ 400, // This is how long a reader will attempt to find a tag and also our interval
  /* Expire Time:*/ 2000 // This is how long a reader will keep tag information before expiring
};

FX200<numberOfReaders> fx200(config);

void setup()
{
  Serial.begin(115200);

  // Initialize our object
  fx200.init();
}

void loop()
{
  if (hallLearnCommand()) learn(); // Learn the tags when a magnet is detected
  
  fx200.scan(); // Command the RFID network to scan, should be called in a loop with minimal blocking

  if (fx200.hasNewData()) // Do we have new data?
  {
    // Iterate over all the readers and see what they have
    for (int i = 0; i < numberOfReaders; i++)
    {
      Serial.print(i); // Print the index of the reader
      Serial.print(": ");
      Serial.println(fx200.getTag(i)); // Print the tag data
    }
  }

  // If FX200 tags are learned, and they match, solve the game
  if (fx200.isSolved())
  {
    Serial.println("Solved");
  }
}

void learn()
{
  Serial.println("Learning...");
  while (!hallLearnCommand()) {}
  
  bool learned = fx200.learn();
  Serial.println(learned ? "Learning successful!" : "Learning failed!");
}

bool hallLearnCommand()
{
  static const int HALL_NORTH_THRESH = (.4 * 0x3FF);
  static const int HALL_SOUTH_THRESH = (.6 * 0x3FF);

  static bool releaseNorth;
  static bool learning = false;
  static bool released = true;

  int read = analogRead(HALL);

  bool inNorth = read < HALL_NORTH_THRESH;
  bool inSouth = read > HALL_SOUTH_THRESH;

  if (!inNorth && !inSouth) released = true;

  if (!released) return false;

  if (learning)
  {
    if (inNorth && releaseNorth)
    {
      released = false;
      learning = false;
      return true;
    }
    else if (inSouth && !releaseNorth)
    {
      released = false;
      learning = false;
      return true;
    }
  }
  else
  {
    if (inNorth)
    {
      releaseNorth = false;
      learning = true;
      released = false;

      return true;
    }
    else if (inSouth)
    {
      releaseNorth = true;
      learning = true;
      released = false;

      return true;
    }
  }

  return false;
}
