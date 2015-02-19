#include "YunClient.h"
#include "SPI.h"
#include <PubSubClient.h>

#include "yunStorage.h"
#include "yunTimestamp.h"
#include "yunTransmitter.h"

#include "TestSensor.h"
#include "UltrasonicSR04.h"

#define REPORT_RESOLUTION 5 // when to check for sensors to report [s]
#define SEND_RESOLUTION 41 // when to check for sensors to report [s]

#define COLLECTOR 0x1
#define STORAGE 0x2

#define OPEN_DATAPOINT '('
#define CLOSE_DATAPOINT ")"
#define SEPARATOR ' '

#define MAX_VALUE_SIZE 10
#define TIMESTAMP_SIZE 10
#define MAX_DATAPOINT_SIZE MAX_VALUE_SIZE + TIMESTAMP_SIZE + 3



// the actural sensors and related info this sketch knows about
#define NUMBER_OF_SENSORS 3
// overview over used PINs
#define US_TRIG_PIN 13
#define US_ECHO_PIN 12
Sensor* sensors[NUMBER_OF_SENSORS] = { new UltrasonicSR04('a', 5,
                                                          US_TRIG_PIN,
                                                          US_ECHO_PIN),
                                       new TestSensor('c', 10) };


// uncomment to inspect for memory leaks (call this function where leaks suspected):
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void reportFreeRam() {
  Serial.println("Free Ram : " + String(freeRam()));
}

void setup() {
  Bridge.begin();
  FileSystem.begin();
  Serial.begin(9600);
}

void collectData(unsigned long seconds) {
  for (byte i=0;i<NUMBER_OF_SENSORS;i++) {
    Sensor* sensor = sensors[i];
    if (sensor->wantsToReport(seconds)) {
      char dataPoint[MAX_DATAPOINT_SIZE];
      getDataPoint(sensor, dataPoint);
      submitDataPoint(dataPoint);
    }
  }
}

// Writes one or more value(s) to a dataPoint of format
// "(<sensorName> <timestamp> <value> [<value> ..])"
void getDataPoint(Sensor* sensor, char* dataPoint) {
  char unixTimestamp[TIMESTAMP_LENGTH + 1];
  getTimestamp(unixTimestamp);
  sprintf(dataPoint, "%c%c%c%s%c", OPEN_DATAPOINT, sensor->name, SEPARATOR,
          unixTimestamp, SEPARATOR);
  char chArray[MAX_VALUE_SIZE];
  sensor->getData(chArray);
  strcat(dataPoint, chArray);
  strcat(dataPoint, CLOSE_DATAPOINT);
}

void submitDataPoint(char* dataPoint) {
  //Serial.println("[COLLECTOR:] Submitting datapoint: " + String(dataPoint));
  writeDataPoint(dataPoint);
}

// we need a global `previousValue` in order not to run collectData
// a million billion times while loop has found the right second
unsigned long previousValue = 0;
void loop() {
  unsigned long seconds = millis() / 1000;
  if (seconds != previousValue) {
    if (seconds % (REPORT_RESOLUTION) == 0) {
      collectData(seconds);
    }
    if (seconds % (SEND_RESOLUTION) == 0) {
      sendData();
    }
  }
  previousValue = seconds;
}