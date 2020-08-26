/****************************************************************
*                  Smartxapela 1.0 Firmwarea                    *
*                    CMPS10 sentsorea openHAB-en                *
*     Aitor, Aitor & Aitor by OteitzaLP eta Tknika              *
*     2017                                                      *
*****************************************************************

Smartxapela buruaren mugimenduak monitorizatu eta kontrolagailu batean 
beste edozein dispositibo kontrolatzeko prest jartzen dituen proiektua
da.

Creatives Commons CC
*/
#include <Wire.h>
#include <SoftwareSerial.h>
#include <SPI.h>

// MySensor parametroak
#define MY_DEBUG_
#define MY_NODE_ID 10
#define MY_RFM69_NEW_DRIVER
#define MY_RADIO_RFM69

#include <MySensors.h>

// MySensors sensor
#define sensorChild 1

#define ADDRESS 0x60        // Sentsorearen I2C helbidea

int minChangeValue = 1;

int bearingLastSendValue = -100;
int pitchLastSendValue = -100;
int rollLastSendValue = -100;

int marka_roll_ezker = 0;
int marka_roll_eskuin = 0;
int marka_pitch_aurrera = 0;
int marka_pitch_atzera = 0;

void presentation(){
  //MySensors
  sendSketchInfo("Smartxapela", "1.0");
  present(sensorChild, S_CUSTOM, "sentsorea") ;
}

MyMessage bearingMsg(sensorChild, V_VAR1);  
MyMessage pitchMsg(sensorChild, V_VAR2);  
MyMessage rollMsg(sensorChild, V_VAR3);  

MyMessage rollEvent(sensorChild, V_VAR4);  
MyMessage pitchEvent(sensorChild, V_VAR5); 

struct measurement {
  int pitch;           // Aurre inklinazioa
  int roll;            // Albo inklinazioa
  int bearing;         // Iparraldearekiko dugun desbideraketa
} m;

struct measurement readMeasurement() {
  struct measurement _measurement;
  
  Wire.beginTransmission(ADDRESS);     // I2C komunikazioa martxan jarri
  Wire.write(2);                       // Sentsorean irakurri nahi den erregistroa
  Wire.endTransmission();
  Wire.requestFrom(ADDRESS, 4);        // 4 Byte eskatu
  while(Wire.available() < 4){}        // 4 Byteak prest egon arte itxaron estén listos
  byte highByte = Wire.read();         // Sentsorearen 3 irakurketa egin eta gorde
  byte lowByte = Wire.read();          // highByte eta lowBytek desbideraketaren  Byte altua y baxua
             
  _measurement.pitch = Wire.read();              
  _measurement.roll = Wire.read();               
  _measurement.bearing = ((highByte<<8)+lowByte)/10;      // Iparraldearekiko dugun desbideraketa

  return _measurement;
}

void setup() { 
    Serial.begin(9600);
    Wire.begin();                  // I2C komunikazioa martxan jartzen du
}

void loop() {
  
  m = readMeasurement();    
  
  if (0 <= m.roll && m.roll <= 85) {
  } else if (171 <= m.roll && m.roll <= 255) {
    m.roll = m.roll - 256;
  } else {
    m.roll = 85;
  }
   
  if (0 <= m.pitch && m.pitch <= 85) {
    m.pitch = m.pitch;
  } else if (171 <= m.pitch && m.pitch <= 255) {
    m.pitch = m.pitch - 256;
  } else {
    m.pitch = 85;
  }


  // roll markak           
  if (marka_roll_ezker == 0 && m.roll < -20 && m.roll > -80) {
    marka_roll_ezker = 1;
    send(rollEvent.set("0"));
  }
    
  if (marka_roll_ezker !=0){ 
    if (m.roll > -10 ) {
      marka_roll_ezker = 0;
    }
  }
   
  if (marka_roll_eskuin == 0 && m.roll > 20 && m.roll < 80) {
    marka_roll_eskuin = 1;
    send(rollEvent.set("1"));
  }
   
  if (marka_roll_eskuin !=0) { 
    if (m.roll < 10 ) {
      marka_roll_eskuin = 0;
    }
  }

  // pitch markak
  if (marka_pitch_aurrera == 0 && m.pitch < -25 && m.pitch > -80) {
    marka_pitch_aurrera = 1;
    send(pitchEvent.set("0"));
  }
  
  if (marka_pitch_aurrera != 0) { 
    if (m.pitch > -10 ) {
      marka_pitch_aurrera = 0;
    }
  }

  if (marka_pitch_atzera == 0 && m.pitch > 10 && m.pitch < 80  ) {
    marka_pitch_atzera = 1;
    send(pitchEvent.set("1"));
  }  
  
  if (marka_pitch_atzera != 0){ 
    if (m.pitch < 10 ) {
      marka_pitch_atzera = 0;
    }
  }

  #ifdef MY_DEBUG
   Serial.print("Pitch: ");         // Balioak serie monitorean erakutsi
   Serial.println(m.pitch);
   Serial.print("Roll: ");
   Serial.println(m.roll);
   Serial.print("Bearing: ");
   Serial.println(m.bearing);
   Serial.println(" ");
   Serial.print("Roll Ezker Ebentua:  ");         
   Serial.println(marka_roll_ezker);
   Serial.print("Roll Eskuin Ebentua:  "); 
   Serial.println(marka_roll_eskuin);
   Serial.print("Pitch Aurrera Ebentua:  ");         
   Serial.println(marka_pitch_aurrera);
   Serial.print("Pitch Atzera Ebentua:  ");
   Serial.println(marka_pitch_atzera);
   Serial.println(" "); 
  #endif

  if (abs(m.bearing-bearingLastSendValue) > minChangeValue) { //Aldaketa minimoa izan arte ez bidali daturik
    send(bearingMsg.set(m.bearing)) ;                         //Desbideraketa kontroladorera bidali
    bearingLastSendValue = m.bearing;
  }

  if (abs(m.pitch-pitchLastSendValue) > minChangeValue) { //Aldaketa minimoa izan arte ez bidali daturik
    send(pitchMsg.set(m.pitch)) ;                         //Aurre Inklinazioa kontroladorera bidali
    pitchLastSendValue = m.pitch;
  }

  if (abs(m.roll-rollLastSendValue) > minChangeValue) {  //Aldaketa minimoa izan arte ez bidali daturik
    send(rollMsg.set(m.roll)) ;                         //Albo Inklinazioa kontroladorera bidali
    rollLastSendValue = m.roll;
  }

  //delay(50);
}
