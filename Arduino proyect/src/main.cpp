#include <Arduino.h>
#include <StarterKitNB.h>
#include <SparkFun_SHTC3.h>
#include <Wire.h>

#define PIN_VBAT WB_A0

#define VBAT_MV_PER_LSB (0.73242188F) 
#define VBAT_DIVIDER_COMP (1.73)      
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

SHTC3 mySHTC3;  
StarterKitNB sk;  
// NB
String apn = "m2m.entel.cl";
String user = "entelpcs";
String pw = "entelpcs";
String band = "B28 LTE";

// Thingsboard
String ClientIdTB = "grupo10";
String usernameTB = "aaaaa";
String passwordTB = "aaaaa";
String msg = "";
String resp = "";
String Operador = "";
String Banda = "";
String Red = "";

void errorDecoder(SHTC3_Status_TypeDef message)             
{
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.print("Nominal"); break;
    case SHTC3_Status_Error : Serial.print("Error"); break;
    case SHTC3_Status_CRC_Fail : Serial.print("CRC Fail"); break;
    default : Serial.print("Unknown return code"); break;
  }
}

void setup()
{
  sk.Setup();
  delay(500);
  sk.UserAPN(apn, user, pw);
  delay(500);
  Wire.begin();
  sk.Connect(apn, band);
  Serial.print("Beginning sensor. Result = ");            
  errorDecoder(mySHTC3.begin());                       
  Serial.println("\n\n");
}
void loop()
{
  if (!sk.ConnectionStatus()) // Si no hay conexion a NB
  {
    sk.Reconnect(apn, band);  // Se intenta reconecta
    delay(2000);
  }

  sk.ConnectBroker(ClientIdTB, usernameTB, passwordTB);  // Se conecta a ThingsBoard
  delay(2000);

  resp = sk.bg77_at((char *)"AT+COPS?", 500, false);
  Operador = resp.substring(resp.indexOf("\""), resp.indexOf("\"",resp.indexOf("\"")+1)+1);     
  delay(1000);

  resp = sk.bg77_at((char *)"AT+QNWINFO", 500, false);
  Red = resp.substring(resp.indexOf("\""), resp.indexOf("\"",resp.indexOf("\"")+1)+1);          
  Banda = resp.substring(resp.lastIndexOf("\"",resp.lastIndexOf("\"")-1), resp.lastIndexOf("\"")+1); 
  delay(1000);

  SHTC3_Status_TypeDef result = mySHTC3.update();             
  if(mySHTC3.lastStatus == SHTC3_Status_Nominal)             
  {
    msg = 
    "{\"humidity\":"+String(mySHTC3.toPercent())
    +",\"temperature\":"+String(mySHTC3.toDegC())
    +",\"battery\":"+String(round(analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB)/37)
    +",\"Operador\":"+Operador
    +",\"Banda de frecuencia\":"+Banda
    +",\"Red IoT\":"+Red+"}";
    Serial.println(msg);  
    sk.SendMessage(msg);
  }
  else
  {
    msg = 
    "{\"humidity\":\"ERROR\", \"temperature\":\"ERROR\",\"battery\":"
    +String(round(analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB)/37)
    +",\"Operador\":"+Operador
    +",\"Banda de frecuencia\":"+Banda
    +",\"Red IoT\":"+Red+"}";
    Serial.println(msg);
    sk.SendMessage(msg);
  }

  sk.DisconnectBroker();
  delay(500);                                                 // Tiempo de espera para cada entrega de datos


}


// Codigo ejemplo sensor de conteo de personas //
#define SENSOR_PIN  WB_IO6   // Attach AM312 sensor to Arduino Digital Pin WB_IO6

int gCurrentStatus = 0;         // variable for reading the pin current status
int gLastStatus = 0;            // variable for reading the pin last status

void setup()
{
   pinMode(SENSOR_PIN, INPUT);   // The Water Sensor is an Input
   pinMode(LED_GREEN, OUTPUT);  // The LED is an Output
   pinMode(LED_BLUE, OUTPUT);   // The LED is an Output
   Serial.begin(115200);
   time_t timeout = millis();
   while (!Serial)
   {
     if ((millis() - timeout) < 5000)
     {
       delay(100);
     }
     else
     {
       break;
     }
   }
   Serial.println("========================");
   Serial.println("    RAK12006 test");
   Serial.println("========================");
}

void loop() {

  gCurrentStatus = digitalRead(SENSOR_PIN);
  if(gLastStatus != gCurrentStatus)
  {
    if(gCurrentStatus == 0)
    {//0: detected   1: not detected
      Serial.println("IR detected ...");
     digitalWrite(LED_GREEN,HIGH);   //turn on
     digitalWrite(LED_BLUE,HIGH);
    }
    else
    {
      digitalWrite(LED_GREEN,LOW);
      digitalWrite(LED_BLUE,LOW);   // turn LED OF
    }
    gLastStatus = gCurrentStatus;
  }
  else
  {
    delay(100);
  }

}
