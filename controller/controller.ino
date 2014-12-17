#define YUN
//#define UNO

#include "Timer.h"
#include "TestSensor.h"
#include <PubSubClient.h>

#define SECOND 1000L
#define MINUTE 60 * SECOND
#define HOUR 60 * MINUTE

#ifdef YUN
  #include <SPI.h>
  #include "YunBoard.h"
  #include <YunClient.h>

  #define LOGDIR  "/mnt/sda1"
  #define LOGPATH "/mnt/sda1/"
  #define BOARD_TYPE YunBoard

  YunClient eClient;
#endif

#ifdef UNO
  #include <SdFat.h>
  #include "UnoBoard.h"
  #include <Wire.h>
  #include "RTClib.h"
  #include "Ethernet.h"

  #define LOGDIR ""
  #define LOGPATH ""
  #define BOARD_TYPE UnoBoard

  EthernetClient eClient;
#endif

#define TIMESTAMP_LENGTH 10                    // number of chars of a unix ts
#define LOGPATH_TIMESTAMP_INDEX sizeof(LOGDIR) - 1 // where to write the ts in the path
#define LOGPATH_LABEL_INDEX LOGPATH_TIMESTAMP_INDEX + TIMESTAMP_LENGTH // where to write '.c'
                                                                       // or '.s'
#define CLOSED_SUFFIX "C"
#define SENT_SUFFIX "S"
#define LOG_SUFFIX "L"
#define DOT "."

#define DOT_LENGTH sizeof(DOT) -1
#define LABEL_LENGTH sizeof(CLOSED_SUFFIX) - 1

#define FILEPATH_LENGTH LOGPATH_LABEL_INDEX + DOT_LENGTH + LABEL_LENGTH

#define BYTESUM_CHARLENGTH 6
#define CHECKSUM_LENGTH TIMESTAMP_LENGTH + BYTESUM_CHARLENGTH

#define DATAPOINT_MAX 2

//byte server[] = { 10, 10, 63, 221 };  // WIFI-specifics
byte server[] = { 192, 168, 0, 7 };  // WIFI-specifics

BOARD_TYPE board;

// the actural sensors this sketch knows about
TestSensor<BOARD_TYPE> testSensorOne('1'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorTwo('2'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorThree('3'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorFour('4'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorFive('5'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorSix('6'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorSeven('7'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorEight('8'); // create TestSensor with it's UID
TestSensor<BOARD_TYPE> testSensorNine('9'); // create TestSensor with it's UID

PubSubClient client(server, 1883, callback, eClient);

char dataFilePath[FILEPATH_LENGTH + 1] = LOGPATH;
byte dataPointCounter = 0; // the number of dataPoints in one dataFile
char unixTimestamp[TIMESTAMP_LENGTH + 1];    // a unix timestamp

Timer t;                   // the timer starts processes at configured time
                           //   (process: e.g. 'get sensor reading')

// TODO: remove if unused!
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("CALLBACK!!! topic: " + String(topic));
  // handle message arrived
}

void setup() {
  board.begin();
  Serial.begin(9600);

  while(!Serial);

  createNewDataFile();

  // task scheduling..
  // (1) sensor data collection and logging
  t.every(5 * SECOND, writeDataForSensorOne);
  t.every(10 * SECOND, writeDataForSensorTwo);
  t.every(11 * SECOND, writeDataForSensorThree);
  t.every(12 * SECOND, writeDataForSensorFour);
  t.every(13 * SECOND, writeDataForSensorFive);
  t.every(14 * SECOND, writeDataForSensorSix);
  t.every(15 * SECOND, writeDataForSensorSeven);
  t.every(16 * SECOND, writeDataForSensorEight);
  t.every(17 * SECOND, writeDataForSensorNine);

  // (3) data transfer to server
  t.every(10 * SECOND, sendData);

}

void createNewDataFile(){
  board.getTimestamp(unixTimestamp);
  Serial.println("timeStamp " + String(unixTimestamp));
  byte i = 0;
  for (i; i<TIMESTAMP_LENGTH +1; i++) {
    if (i == 8) {
      dataFilePath[LOGPATH_TIMESTAMP_INDEX +i] = '.';
    } else if (i > 8) {
      dataFilePath[LOGPATH_TIMESTAMP_INDEX + i] = char(unixTimestamp[i-1]);
    } else {
      dataFilePath[LOGPATH_TIMESTAMP_INDEX + i] = char(unixTimestamp[i]);
    }
  }
  dataFilePath[i] = '\0';
  strcat(dataFilePath, LOG_SUFFIX);

  Serial.println("\ndataFilePath: " + String(dataFilePath));
  board.createFile(dataFilePath);

}

void writeDataForSensorOne() {
  if (checkCounter()) {
    testSensorOne.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorTwo() {
  if (checkCounter()) {
    testSensorTwo.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorThree() {
  if (checkCounter()) {
    testSensorThree.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorFour() {
  if (checkCounter()) {
    testSensorFour.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorFive() {
  if (checkCounter()) {
    testSensorFive.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorSix() {
  if (checkCounter()) {
    testSensorSix.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorSeven() {
  if (checkCounter()) {
    testSensorSeven.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorEight() {
  if (checkCounter()) {
    testSensorEight.collectData(dataFilePath, unixTimestamp, board);
  }
}
void writeDataForSensorNine() {
  if (checkCounter()) {
    testSensorNine.collectData(dataFilePath, unixTimestamp, board);
  }
}

bool checkCounter() {
  Serial.print("dp" + String(dataPointCounter) + "-");
  boolean bol;
  if (dataPointCounter < DATAPOINT_MAX) {
    dataPointCounter ++;
    bol = true;
  } else {
    relabelFile(dataFilePath, CLOSED_SUFFIX);
    // 2. create a new file with timestamped name
    createNewDataFile();
    bol = false;
    dataPointCounter = 0;
  }
  // in any case get a current timestamp to pass on to sensor(s)
  board.getTimestamp(unixTimestamp);
  return bol;
}

void loop () {
  t.update();
}

// 3. Data Transfer to server
void sendData() {
  char sendFilePath[FILEPATH_LENGTH + 1] = LOGDIR;
  Serial.println("Preparing to send data");
  while (board.nextPathInDir(sendFilePath, CLOSED_SUFFIX)) {
    Serial.println("..for file " + String(sendFilePath));
    unsigned int bufferLength = board.fileSize(sendFilePath);
    char sendBuffer[bufferLength + 1];

    char checksum[CHECKSUM_LENGTH + 1] = {'\0'};
    char startEnd[10];
    // checksumBytes: sum of all byte-values of the file
    unsigned long checksumBytes = board.readFile(sendFilePath, sendBuffer);

    buildChecksum(checksum, sendBuffer, bufferLength, checksumBytes);

    Serial.println("trying to send it..");
    if (client.connect("siteX", "punterX", "punterX")) {
      Serial.println("Got a connection!");
      if (client.publish(checksum, sendBuffer)) {
        Serial.println("should have sent the stuff by now..");
        relabelFile(sendFilePath, SENT_SUFFIX);
      }
    } else {
      Serial.println("Didn't get a connection!");
    }
  }
}

void buildChecksum(char* checksum, char* buffer, unsigned int bufferLength, unsigned long bytes) {
  sprintf(checksum, "%ld", bytes);
  for (byte i=strlen(checksum); i<CHECKSUM_LENGTH; i++) {
    if (i < BYTESUM_CHARLENGTH) {
      checksum[i] = ':';
    } else {
      checksum[i] = buffer[bufferLength - (CHECKSUM_LENGTH - i)];
    }
  }
  checksum[CHECKSUM_LENGTH] = '\0';
}

void relabelFile(char* oldName, char* label) {
  char newName[FILEPATH_LENGTH + 1];
  strcpy(newName, oldName);
  for (byte i=0; i<LABEL_LENGTH; i++) {
    newName[FILEPATH_LENGTH - 1 - i] = label[LABEL_LENGTH - 1 - i];
  }
  board.renameFile(oldName, newName);
  Serial.println("newName: " + String(newName));
}
