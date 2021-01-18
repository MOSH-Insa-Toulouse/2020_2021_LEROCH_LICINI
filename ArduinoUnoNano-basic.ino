/*
 * CHANGE ADDRESS!
 * Change the device address, network (session) key, and app (session) key to the values that are registered via the TTN dashboard.
 * The appropriate line is "myLora.initABP(XXX);" or "myLora.initOTAA(XXX);"
 * When using ABP, it is advised to enable "relax frame count".
 *
 * Connect the RN2xx3 as follows:
 * RN2xx3 -- Arduino
 * Uart TX -- 5
 * Uart RX -- 4
 * Reset -- 12
 * Vcc -- 3.3V
 * Gnd -- Gnd
 *
 * If you use an Arduino with a free hardware serial port, you can replace
 * the line "rn2xx3 myLora(mySerial);"
 * with     "rn2xx3 myLora(SerialX);"
 * where the parameter is the serial port the RN2xx3 is connected to.
 * Remember that the serial port should be initialised before calling initTTN().
 * For best performance the serial port should be set to 57600 baud, which is impossible with a software serial port.
 */
#include <rn2xx3.h>
#include <SoftwareSerial.h>
//#include "analogComp.h"

SoftwareSerial mySerial(4, 5); // RX, TX

#define pinLED 7
#define pinButton 8
#define pinGasAna A3
#define gasThreshold 300

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

bool bascule=false;

void setup()
{

  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinGasAna, INPUT);

  //Interruption capteur gaz
  //0=2
  attachInterrupt(0,interrupt_gaz,CHANGE);
  
  //pinMode(3, OUTPUT);
  //analogWrite(3, 100);
  //analogComparator.setOn();
  
  // Open serial communications and wait for port to open:
  Serial.begin(57600); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  Serial.println("Startup");

  initialize_radio();

  //transmit a startup message
  myLora.tx("TTN Mapper on TTN LL node");

  //set on comparateur interruption
  //analogComparator.waitComp(100);
  //analogComparator.enableInterrupt(interrupt_gaz, CHANGE);
  //analogComparator.disableInterrupt();
  
  delay(2000);
}

void interrupt_gaz()
{
  digitalWrite(2,LOW);
  Serial.println("interruption");
  bascule=not(bascule);
  if(bascule){
      myLora.tx("Alerte!");
      digitalWrite(pinLED, HIGH);
      Serial.println("alert");
  }else{
      myLora.tx("Fin alerte");
      digitalWrite(pinLED, LOW);
      Serial.println("Fin alert");      
  }
}

void initialize_radio()
{
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;

  /*
   * ABP: initABP(String addr, String AppSKey, String NwkSKey);
   * Paste the example code from the TTN console here:
   */
 // const char *devAddr = "02017201";
 // const char *nwkSKey = "AE17E567AECC8787F749A62F5541D522";
 // const char *appSKey = "8D7FFEF938589D95AAD928C2E2E7E48F";

  //join_result = myLora.initABP(devAddr, appSKey, nwkSKey);

  /*
   * OTAA: initOTAA(String AppEUI, String AppKey);
   * If you are using OTAA, paste the example code from the TTN console here:
   */
  const char *appEui = "70B3D57ED0038A88";
  const char *appKey = "FD967206B4D5416CCABCDB604DBF545A";

  join_result = myLora.initOTAA(appEui, appKey);


  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");
}

void loop()
{
  int analogSensor = analogRead(pinGasAna);

  Serial.println(analogSensor);
  if(analogSensor > gasThreshold){
    digitalWrite(2,HIGH);
    //digitalWrite(pinLED, HIGH);
    //Serial.println("alert");
    //myLora.tx("!"); //one byte, blocking function
  }else{
    digitalWrite(2, LOW);
    //digitalWrite(pinLED, LOW);
    //myLora.tx("l"); //one byte, blocking function
  }
    /*Serial.println("TXing");
    myLora.tx("!"); //one byte, blocking function
    String response = myLora.getRx();
    Serial.println(response);
    response = "";
    delay(5000);*/
    delay(1000);
}
