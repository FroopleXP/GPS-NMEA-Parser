/*
 *   GPS NMEA PARSER FOR BLUETOOTH GPS RECEIVER 
 *   WRITTEN BY: Connor Edwards
 *   DATE: 6/7/16
 *   (C) Noval Studios, 2016 
 */

// Including the SPI and SD libraries
#include <SPI.h>
#include <SD.h>

////////////////////////////////
//  GPS CONFIGURATION
////////////////////////////////
// Variable for incoming data
String gps_stream_data;

// Mem for Lat, Long and UTC time
float latitude; 
int lat_sign_index;
int lat_index;

float longitude;
int long_sign_index; 
int long_index;

int gps_fix;
int gps_fix_index;

// Declaring start point of command
int start_index;

// Counter for SD Card
int amount_of_files = 0;
String new_filename = "gps_data_";
String file_extension = ".txt";
String filename;
char* file_char;
File root;

// Characters for NMEA
char nmea_code[] = {
  'G', 'G', 'A'
};

////////////////////////////////
//  SD CONFIGURATION
////////////////////////////////
const int chipSelect = 4;

void setup() {

  // Starting Serial @ 9600 Baud
  Serial.begin(9600);

  Serial.println("Starting SD...");
  
  // Starting SD Card
  if (!SD.begin(chipSelect)) { //  Failed to start SD card
    Serial.println("Failed to initialise SD, is it plugged in?");
    return;
  }

  // Counting the amount of files
  root = SD.open("/");
  bool count_finished = false;

  // Looping through and getting the amount of files on SD card
  while (!count_finished) {
    File entry = root.openNextFile();
    if (!entry) {
      count_finished = true;
    } else if (!entry.isDirectory()) {
      amount_of_files++;
    }
  }

  Serial.println(amount_of_files);
  
  // Creating the file name
  new_filename += String(amount_of_files);
  filename += new_filename;
  filename += file_extension;
  
  filename.toCharArray(file_char, filename.length());
  
  Serial.println(file_char);
  
}

void loop() {

  // Checking for data
  if (Serial.available() > 0) {
    
    // Getting the GPS data stream
    gps_stream_data = Serial.readStringUntil('\n');

    Serial.println(gps_stream_data);

    // Check that it is a command
    if (gps_stream_data.charAt(0) == '$') {
      
      // Command is valid, check that it is GPGGA data
      if (gps_stream_data.charAt(3) == nmea_code[0] and 
          gps_stream_data.charAt(4) == nmea_code[1] and 
          gps_stream_data.charAt(5) == nmea_code[2]) {
            
          // Get the start index
          start_index = gps_stream_data.indexOf(',');

          // Getting the Latitude index
          lat_index = gps_stream_data.indexOf(',', start_index + 1);
          // Getting the Latitude sign index
          lat_sign_index = gps_stream_data.indexOf(',', lat_index + 1);
          
          // Getting the Longitude index
          long_index = gps_stream_data.indexOf(',', lat_sign_index + 1);
          // Getting the Sign of Longitude index
          long_sign_index = gps_stream_data.indexOf(',', long_index + 1);

          // Getting the satellite fix quality index
          gps_fix_index = gps_stream_data.indexOf(',', long_sign_index + 1);
          
          // Getting the Satellite fix quality
          gps_fix = gps_stream_data.substring(gps_fix_index + 1, gps_fix_index + 2).toInt();
  
          // Checking the Satellite fix quality
          if (gps_fix == 1) {

            Serial.println(new_filename);
             
            // Starting the SD for Writing GPS data to
            File gps_data = SD.open("gps_data.txt", FILE_WRITE);
            
            // Getting the Latitude and sign
            String lat_sign = gps_stream_data.substring(lat_sign_index + 1, long_index); // Getting the Sign
            
            float lat = gps_stream_data.substring(lat_index + 1, lat_sign_index).toFloat();
            float lat_deg = float(int(lat / 100));
            float lat_deg_dec = (lat - (lat_deg * 100)) / 60;
            latitude = lat_deg + lat_deg_dec;

            Serial.println(latitude, 4);
            
            // Checking the sign
            if (lat_sign == "S") {
              latitude = latitude * (-1);
            }
            
            // Getting the Longitude and sign 
            String _long_sign = gps_stream_data.substring(long_sign_index + 1, gps_fix_index); // Getting the Sign
            
            float _long = gps_stream_data.substring(long_index + 1, long_sign_index).toFloat();
            float _long_deg = float(int(_long / 100));
            float _long_deg_dec = (_long - (_long_deg * 100)) / 60;
            longitude = _long_deg + _long_deg_dec;

            Serial.println(longitude, 4);
            
            // Checking the sign
            if (_long_sign == "W") {
              longitude = longitude * (-1);
            }

            // Write the Coordinates to SD
            if (gps_data) {
              gps_data.print(longitude, 4);
              gps_data.print(",");
              gps_data.print(latitude, 4);
              gps_data.print(",");
              gps_data.print("0"); // Mock Distance above see level
              gps_data.print(" ");
            }

            // Closing the file
            gps_data.close();
            
          }
      }
           
    } 
              
  }

  
  
}
