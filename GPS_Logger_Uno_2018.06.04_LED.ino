/******************************************************************************
  University of Wisconsin - Madison
  Joshua Harmon
  Agricultural Engineering
  
  Updated: 2016.03.24
  Update includes: allows for use with Uno and EM-506. Records date as year, 
  month, day and time as hour, minute, second.

  Updated: 2018.03.02
  Update: Trying to make data work with  GP3906-TLP GPS Module. Changed the 
  pins to match  GP3906-TLP GPS Module. Still working on errors.

  Updated: 2018.03.21
  Update changed baude rate and log rate to match spark fun example. Changed time
  match CST.

  Updated: 2018.04.06
  Updated changed the time from +5 hours to -5 hours. Accurate time based off of
  Greenwich Mean Time.
  
  This code uses Arduino Uno and GP3906-TLP GPS Module
  
  Note: The code requires a baude rate of 9600. In previous code versions the code
  used a baude rate of 4800. 
******************************************************************************/

/******************************************************************************
  This example uses SoftwareSerial to communicate with the GPS module on
  pins 8 and 9, then communicates over SPI to log that data to a uSD card.

  It uses the TinyGPS++ library to parse the NMEA strings sent by the GPS module,
  and prints interesting GPS information - comma separated - to a newly created
  file on the SD card.

  Resources:
  TinyGPS++ Library  - https://github.com/mikalhart/TinyGPSPlus/releases
  SD Library (Built-in)
  SoftwareSerial Library (Built-in)

  Development/hardware environment specifics:
  Arduino IDE 1.6.7
  GPS Logger Shield v2.0 - Make sure the UART switch is set to SW-UART
  Arduino Uno, RedBoard, Pro, Mega, etc.
******************************************************************************/

#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

#define ARDUINO_USD_CS 10 // uSD card CS pin (pin 10 on SparkFun GPS Logger Shield)CHANGED 3/2/18 wanted to match code on spark fun

/////////////////////////
// Log File Defintions //
/////////////////////////
// Keep in mind, the SD library has max file name lengths of 8.3 - 8 char prefix,
// and a 3 char suffix.
// Our log files are called "gpslogXX.csv, so "gpslog99.csv" is our max file.
#define LOG_FILE_PREFIX "gpslog" // Name of the log file.
#define MAX_LOG_FILES 100 // Number of log files that can be made
#define LOG_FILE_SUFFIX "csv" // Suffix of the log file
char logFileName[13]; // Char string to store the log file name
// Data to be logged:
#define LOG_COLUMN_COUNT 12
char * log_col_names[LOG_COLUMN_COUNT] = {
  "longitude", "latitude", "altitude", "speed", "course", "year", "month" , "day", "hour" , "minute" , "second", "satellites"
}; // log_col_names is printed at the top of the file.

//////////////////////
// Log Rate Control //
//////////////////////
#define LOG_RATE 5000 // Log every 5 second(s) CHANGED 3/31/18 - Not a major change
unsigned long lastLog = 0; // Global var to keep of last time we logged

/////////////////////////
// TinyGPS Definitions //
/////////////////////////
TinyGPSPlus tinyGPS; // tinyGPSPlus object to be used throughout
//  GP3906-TLP GPS Module: 9600, EM-506 Module: 4800
#define GPS_BAUD 9600 // GPS module's default baud rate - CHANGED 3/31/18

/////////////////////////////////
// GPS Serial Port Definitions //
/////////////////////////////////
// If you're using an Arduino Uno, Mega, RedBoard, or any board that uses the
// 0/1 UART for programming/Serial monitor-ing, use SoftwareSerial:
#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin CHANGED on 3/2/18
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin CHANGED on 3/2/18 
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial

// Set gpsPort to either ssGPS if using SoftwareSerial or Serial1 if using an
// Arduino with a dedicated hardware serial port
#define gpsPort ssGPS // Alternatively, use Serial1 on the Leonardo

// Define the serial monitor port. On the Uno, Mega, and Leonardo this is 'Serial'
//  on other boards this may be 'SerialUSB'
#define SerialMonitor Serial

// LED for visual confirmation of logged data
const int ledPin = A0;
int ledState = LOW;

void setup()
{
  SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);

  SerialMonitor.println("Setting up SD card.");
  // see if the card is present and can be initialized:
  if (!SD.begin(ARDUINO_USD_CS))
  {
    SerialMonitor.println("Error initializing SD card.");
  }
  updateFileName(); // Each time we start, create a new file, increment the number
  printHeader(); // Print a header at the top of the new file
  
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  if ((lastLog + LOG_RATE) <= millis())
  { // If it's been LOG_RATE milliseconds since the last log:
    if (tinyGPS.location.isUpdated()) // If the GPS data is valid
    {
      if (logGPSData()) // Log the GPS data
      {
        SerialMonitor.println("GPS logged."); // Print a debug message
        lastLog = millis(); // Update the lastLog variable
        
        if (ledState == LOW) {
          ledState = HIGH;
        }
        else {
          ledState = LOW;
        }
        digitalWrite(ledPin, ledState);
      }
      else // If we failed to log GPS
      { // Print an error, don't update lastLog
        SerialMonitor.println("Failed to log new GPS data.");
      }
    }
    else // If GPS data isn't valid
    {
      // Print a debug message. Maybe we don't have enough satellites yet.
      SerialMonitor.print("No GPS data. Sats: ");
      SerialMonitor.println(tinyGPS.satellites.value());
    }
  }

  // If we're not logging, continue to "feed" the tinyGPS object:
  while (gpsPort.available())
    tinyGPS.encode(gpsPort.read());
}

byte logGPSData()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile)
  { // Print longitude, latitude, altitude (in feet), speed (in mph), course
    // in (degrees), date, time, and number of satellites.
    logFile.print(tinyGPS.location.lng(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.location.lat(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.altitude.feet(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.speed.mph(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.course.deg(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.date.year());
    logFile.print(',');
    logFile.print(tinyGPS.date.month());
    logFile.print(',');
    logFile.print(tinyGPS.date.day());
    logFile.print(',');
    logFile.print(tinyGPS.time.hour()-5);     //Changed 4/06/18 to match CST
    //Can work in any time zone. CST is minus 5 hours off of Greenwich Mean Time.
    //Check local time based off of Greenwich Mean Time to change time zone.
    logFile.print(',');
    logFile.print(tinyGPS.time.minute());
    logFile.print(',');
    logFile.print(tinyGPS.time.second());
    logFile.print(',');
    logFile.print(tinyGPS.satellites.value());
    logFile.println();
    logFile.close();

    return 1; // Return success
  }

  return 0; // If we failed to open the file, return fail
}

// printHeader() - prints our eight column names to the top of our log file
void printHeader()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile) // If the log file opened, print our column names to the file
  {
    int i = 0;
    for (; i < LOG_COLUMN_COUNT; i++)
    {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // If it's anything but the last column
        logFile.print(','); // print a comma
      else // If it's the last column
        logFile.println(); // print a new line
    }
    logFile.close(); // close the file
  }
}

// updateFileName() - Looks through the log files already present on a card,
// and creates a new file with an incremented file index.
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // Clear logFileName string
    // Set logFileName to "gpslogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // If a file doesn't exist
    {
      break; // Break out of this loop. We found our index
    }
    else // Otherwise:
    {
      SerialMonitor.print(logFileName);
      SerialMonitor.println(" exists"); // Print a debug statement
    }
  }
  SerialMonitor.print("File name: ");
  SerialMonitor.println(logFileName); // Debug print the file name
}


