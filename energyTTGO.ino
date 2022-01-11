#include <WiFi.h>
#include <TFT_eSPI.h> 
//#include <WiFiUdp.h>
#include <vector> 
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "orb.h"
//#include "frame.h"

TFT_eSPI tft = TFT_eSPI(); 

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;  

unsigned long currTime=0;

#include "my_ssid.h"
//const char* ssid     = "xxxxxx"; //edit
//const char* password = "xxxxxx"; //edit

#define gray 0x39C7
#define dblue 0x01A9
#define purple 0xF14F
#define green 0x2D51

const String endpoint ="http://192.168.0.156/api/v1/data";
String payload="";

StaticJsonDocument<6000> doc;

//unsigned long refresh=120000;  // 120 seconds refreash rate
unsigned long refresh=5000;  // 5 seconds refreash rate

int deb=0;
int brightnes[5]={40,80,120,160,200};
int b=1;

double current=0;  // current tt1 reading
double last=0;     // previous tt1 reading

int fromtop=60;

//-------------------------------------------------------------------
// graph
int n=0;
int p[12]={0};
int readings[12]={0};
int minimal;
int maximal;
//-------------------------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  pinMode(35,INPUT_PULLUP);  // RIGHT BUTTON
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, brightnes[b]);
  
  WiFi.begin(ssid, password);
  tft.print("connecting");
  

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    tft.print(".");
   }

  tft.print("CONNECTED!!");
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  
  getData();
}

void loop() {

  // Get the timestamp from the library
  //now = timelib_get();
  //timeNow = now();
  // Convert to human readable format
  //timelib_break(now(), &tinfo);
    
  if(millis()>currTime+refresh)
  {
  getData();
  currTime=millis(); 
  }

  
  if(digitalRead(35)==0)
  {
    if(deb==0)
    {
      deb=1;
      b++;
      if(b==6) b=0;
      ledcWrite(pwmLedChannelTFT, brightnes[b]); 
    }
  }
  else 
  deb=0;
  
}



void getData()
{
    //tft.fillScreen(TFT_BLACK);
    //tft.fillRect(200,126,4,4,TFT_GREEN);
    //tft.fillRect(46,32,56,28,dblue);
    tft.fillRect(116,20,238,122,TFT_BLACK);
    tft.drawLine(118,22,118,122,TFT_WHITE);
    tft.drawLine(118,122,238,122,TFT_WHITE);

    tft.setFreeFont(&Orbitron_Medium_16);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);

    if ((WiFi.status() == WL_CONNECTED)) 
    { //Check the current connection status
      HTTPClient http;
      http.begin(endpoint); //Specify the URL
      int httpCode = http.GET();  //Make the request
      if (httpCode > 0) 
      { //Check for the returning code
        payload = http.getString();
        char inp[payload.length()];
        payload.toCharArray(inp,payload.length());
        deserializeJson(doc,inp);

        String tt1=doc["total_power_import_t1_kwh"];
        String tt2=doc["total_power_import_t2_kwh"];
        String ap=doc["active_power_w"];
        Serial.println(ap);

        tft.setTextColor(TFT_WHITE,TFT_BLACK);
        tft.drawString("T1 (kwh):",4,60+4,2);
        tft.drawString("T2 (kwh):",4,60+4+20+14,2);

        tft.setTextColor(green,TFT_BLACK);
        tft.drawString(ap ,4,10+4,6);
        tft.drawString(tt1,4,60+4+16);
        tft.drawString(tt2,4,60+4+20+14+16);

        if(n<12)
        {
          readings[n]=ap.toInt();
          n++;
        }
        else
        {
          for(int i=1;i<12;i++) readings[i-1]=readings[i];
          readings[11]=ap.toInt();         
        }

        minimal=readings[0];
        maximal=readings[0];

        for(int i=0;i<n;i++)
        {
          if(readings[i]<minimal)   minimal=readings[i]; 
          if(readings[i]>maximal)   maximal=readings[i]; 
        }

        int mx=maximal/2;
        int mi=minimal/2;

        for(int i=0;i<n;i++)
        {
          int re=readings[i]/2;
          p[i]=map(re,mi,mx,0,100);
        }

        if(n>=1)
        {
          for(int i=1;i<n;i++)
          {
            tft.drawLine(118+((i-1)*10),122-p[i-1],118+((i)*10),122-p[i],TFT_RED);
            tft.fillCircle(118+((i-1)*10),122-p[i-1],2,TFT_RED);
            tft.fillCircle(118+((i)*10),122-p[i],2,TFT_RED);
          }
        }
      }
    }
}
