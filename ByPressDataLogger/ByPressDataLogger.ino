#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ThreeWire.h>  
#include <RtcDS1302.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//RTC
#define D4 4
#define D5 5
#define D2 2
//*FSR sensors*/
#define noFSRs 3 // Number of FSRs connected
#define FSR1 34//36  //Analogue ports
#define FSR2 32//32 
#define FSR3 35//34 

int CardFlag = 0;
String dataMessage;
int testinc = 1;
int CSV_Header = 0;
int FirstRec = 0;
unsigned long Rec_Interval_Min = 1; 
unsigned long Rec_Interval_Ms = 0; 
unsigned long last_time = 0; 


ThreeWire myWire(D4,D5,D2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
BluetoothSerial BT;
String FileName = "";

float fsrVoltageArray[3];       // The analog reading converted                                                                
float fsrForceArray[3];         // The force in Newton
float fsrWeightInGramsArray[3]; // Weight converted to grams
float R_fsrArray[3];
float factor[3];
float degree[3];

int   pinArray[3]       = {FSR1, FSR2, FSR3};    // The pin ID for the
int avg_size = 20; // number of analog readings to average//three devices
float forceMaxArray[5]  = {1000.0, 200.0, 200.0}; // Maximum forces                                                        
float million = 1000000.0; // Unit for "1/micro
float conversionToKgrams = 1.0/9.80665;

long K       = 1000;
long R       = 1*K;    // R in K Ohm
long Vcc     = 3.3;    // 5V=5000mV, 3.3V = 3300 mV
float voltageMax = 0.98 * Vcc; // Maximum voltage set to 95% of Vcc. Set                               

void setup() {
 Rec_Interval_Ms = Rec_Interval_Min*60000;
 //Rec_Interval_Ms = 15000;
 Serial.begin(115200);
 BT.begin("Bypress"); //Bluetooth device name


    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);


    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }


 
 SD.begin();
 if(!SD.begin()){
    Serial.println("Failed to mount card");
    return;
  }else{
    Serial.println("card mounted");
  }
  uint8_t cardType = SD.cardType();
  Serial.print("\nCard Type: ");

  if(cardType == CARD_NONE){
    Serial.println("No card attached");
    CardFlag=0;
    return;
  }else{
    CardFlag =1;
    if(cardType == CARD_MMC){
          Serial.println("MMC");
        } else if(cardType == CARD_SD){
          Serial.println("SDSC");
        } else if(cardType == CARD_SDHC){
          Serial.println("SDHC");
        } else {
          Serial.println("UNKNOWN");
        }
    }

   

  String Strcompiled = GetFileName(now);
  FileName = "/"+Strcompiled+".csv";
  Serial.println(FileName);
  //File file = SD.open("/data.txt");
  File file = SD.open(FileName.c_str());
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, Strcompiled.c_str(), "TimeStamp FSR1press FSR2press FSR3press  [mmHg]\r\n");
  }else {
    Serial.println("File already exists");  
  }
  file.close();
  delay(1000);  
}

void loop() {
    /*FSR sensors*/  
    int fsr1 = analogRead(FSR1);
//    Serial.print("FSR1:");
//    Serial.println(analogRead(FSR1));
     
    int fsr2 = analogRead(FSR2);
//    Serial.print("FSR2:");
//    Serial.println(analogRead(FSR2));
     
    int fsr3 = analogRead(FSR3);
//    Serial.print("FSR3:");
//    Serial.println(analogRead(FSR3));
    delay(1000);       

    for (int FSR = 0; FSR < noFSRs; FSR++) {   
      fsrVoltageArray[FSR] = 0.0; //Reset values upon entry
      fsrForceArray[FSR]   = 0.0;
      
      int fsrPin   = pinArray[FSR];     
      int fsrReading = analogRead(fsrPin); 
       
      fsrVoltageArray[FSR] = (float) map(fsrReading, 0, 4095, 0, 3300);
    } //End of loop over FSR's

    float FSR1voltage = fsrVoltageArray[0];
    float FSR2voltage = fsrVoltageArray[1];
    float FSR3voltage = fsrVoltageArray[2];
//    Serial.print("FSR1voltage"); Serial.println(FSR1voltage);
//    Serial.print("FSR2voltage "); Serial.println(FSR2voltage);
//    Serial.print("FSR3voltage "); Serial.println(FSR3voltage );
    
    for (int FSR = 0; FSR < noFSRs; FSR++) {   
      R_fsrArray[FSR] = 0.0; //Reset values upon entry

      int fsrPin   = pinArray[FSR];     
      int fsrReading = analogRead(fsrPin); 
    
      float sum_val = 0.0; // variable for storing sum used for averaging
      for (int ii=0;ii<avg_size;ii++){
          sum_val+=(fsrReading/4095.0)*3.3; 
          delay(10);
      }
      sum_val/=avg_size; // take average
      R_fsrArray[FSR] = (R/1000.0)*((Vcc/sum_val)-1.0); // calculate actual FSR resistance
    } 
   float R_FSR1 = R_fsrArray[0];
   float R_FSR2 = R_fsrArray[1];
   float R_FSR3 = R_fsrArray[2];
//   Serial.print(" R_FSR1="); Serial.println(R_FSR1);
//   Serial.print(" R_FSR2="); Serial.println(R_FSR2);
//   Serial.print(" R_FSR3="); Serial.println(R_FSR3 );
 
   for (int FSR = 0; FSR < noFSRs; FSR++) {  
      //convert to Pressure(mmHg) 
      factor[FSR]=log10(R_fsrArray[FSR])*(1.0/1.476);//0.215
      degree[FSR]=1.454-factor[FSR];//12.86
      fsrForceArray[FSR] =pow(10,degree[FSR]);
      delay(100);
   }  // End of loop over FSRs
   float FSR1press = fsrForceArray[0];
   float FSR2press = fsrForceArray[1];
   float FSR3press = fsrForceArray[2];
   Serial.print("FSR1press"); Serial.println(FSR1press);
   Serial.print("FSR2press"); Serial.println(FSR2press);
   Serial.print("FSR3press"); Serial.println(FSR3press);
   BT.print(FSR1press);
   BT.print(";");
   BT.print(FSR2press);
   BT.print(";");
   BT.println(FSR3press);


  // CSV Header
  if (CSV_Header == 0){
      dataMessage = "TimeStamp,P1,P2,P3\r\n";
      appendFile(SD, FileName.c_str(), dataMessage.c_str());
      CSV_Header = 1;
  }

  //Check Rec 1st record
  if (FirstRec ==0 ){
      //Start Record
      RtcDateTime now = Rtc.GetDateTime();
      String TimeStamp = GetTimeStamp(now);
      if (!now.IsValid()){
        Serial.println("RTC lost confidence in the DateTime!");
      }
      dataMessage = TimeStamp+","+String(FSR1press)+","+ String(FSR2press)+","+ String(FSR3press)+ "\r\n";
      Serial.println(dataMessage);
      appendFile(SD, FileName.c_str(), dataMessage.c_str());
      last_time = millis();  // Start Count interval
      FirstRec = 1;
  }else{
      // Record Data every record interval time
      if( millis() - last_time > Rec_Interval_Ms) {
         RtcDateTime now = Rtc.GetDateTime();
         String TimeStamp = GetTimeStamp(now);
        if (!now.IsValid()){
          // Common Causes:
          //    1) the battery on the device is low or even missing and the power line was disconnected
          Serial.println("RTC lost confidence in the DateTime!");
        }
        
       dataMessage = TimeStamp+","+String(FSR1press)+","+ String(FSR2press)+","+ String(FSR3press)+ "\r\n";
       Serial.println(dataMessage);
       appendFile(SD, FileName.c_str(), dataMessage.c_str());
       last_time = millis(); 
      }
  }

  
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
 // Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

String GetTimeStamp(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return (datestring);
}

String GetFileName(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u_%02u_%04u_%02u_%02u_%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return (datestring);
}
