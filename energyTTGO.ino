#include <WiFi.h>
#include <TFT_eSPI.h> 
#include <vector> 
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "orb.h"

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

#define ACTIVE_POWER  1
#define POWER_PER_DAY 2
int application = ACTIVE_POWER;

StaticJsonDocument<6000> doc;


//unsigned long refresh=120000;  // 120 seconds refreash rate
unsigned long refresh=5000;  // 5 seconds refreash rate

int deb00=0;  // LEFT BUTTON PUSHED
int deb35=0;  // RIGHT BUTTON PUSHED

int brightnes[5]={40,80,120,160,200};
int b=1;

double current=0;  // current tt1 reading
double last=0;     // previous tt1 reading

String tt1OLD="888.888";
String tt2OLD="888.888";
String apOLD="8888";

//-------------------------------------------------------------------
// graph
int n=0;
int p[12]={0};
int readings[12]={0};
int minimal;
int maximal;
//-------------------------------------------------------------------
void setup() {
  pinMode( 0,INPUT_PULLUP);  // LEFT BUTTON
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
  
  getDataFromP1();
}

// ---------------------------------------------------------------------------
void loop() {

  
  if(millis()>currTime+refresh)
  {
    if(application==ACTIVE_POWER)
    {
      //Serial.println("..loop(ACTIVE_POWER)");
      getDataFromP1();
    }
    else
    {
      if(application==POWER_PER_DAY)
      {
        Serial.println("..loop(POWER_PER_DAY)");
        getDataFromInfluxDB();
      }
    }
    currTime=millis(); 
  }

  if(digitalRead(0)==0)
  {
    if(deb00==0)
    {
      Serial.println("..push LEFT BUTTON");
      deb00=1;
      if(application==ACTIVE_POWER)
      {
        application=POWER_PER_DAY;
        tft.fillScreen(TFT_BLACK);  // 135x240
        //tft.fillRect(5,5,230,130,TFT_GREEN);
        //tft.fillRect(5,5,238,130,dblue);
        tft.drawLine(20,122,235,122,TFT_WHITE);        
        Serial.println("....application=POWER_PER_DAY");
      }
      else
      {
        if(application==POWER_PER_DAY)
        {
          application=ACTIVE_POWER;
          tft.fillScreen(TFT_BLACK);  // 135x240
          Serial.println("....application=ACTIVE_POWER");
        }
      }
    }
  }
  else
  {
    deb00=0;
  }


    
  if(digitalRead(35)==0)
  {
    if(deb35==0)
    {
      Serial.println("..push RIGHT BUTTON");
      deb35=1;
      b++;
      if(b==6) b=0;
      ledcWrite(pwmLedChannelTFT, brightnes[b]); 
    }
  }
  else 
  {
    deb35=0;
  }
}

// ---------------------------------------------------------------------------
void displayActivePower(String tt1, String tt2, String ap)
{

        tft.setTextColor(TFT_WHITE,TFT_BLACK);
        tft.drawString("T1 (kwh):",4,60+4,2);
        tft.drawString("T2 (kwh):",4,60+4+20+14,2);

        // Remove old data
        tft.setTextColor(TFT_BLACK,TFT_BLACK);
        tft.drawString(apOLD ,4,10+4,6);
        tft.drawString(tt1OLD,4,60+4+16);
        tft.drawString(tt2OLD,4,60+4+20+14+16);

        // Display new data
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

        tt1OLD = tt1;
        tt2OLD = tt1;
        apOLD  = ap;
}

// ---------------------------------------------------------------------------
void getDataFromP1()
{
    const String endpoint ="http://192.168.0.156/api/v1/data";
    String payload="";

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
      if (httpCode > 0)    //Check for the returning code
      { 
        payload = http.getString();
        char inp[payload.length()];
        payload.toCharArray(inp,payload.length());
        deserializeJson(doc,inp);

        String tt1String=doc["total_power_import_t1_kwh"];
        String tt2String=doc["total_power_import_t2_kwh"];
        String apString=doc["active_power_w"];
        //Serial.println(apString);
        displayActivePower(tt1String,tt2String,apString);
      }
    }
}

// ---------------------------------------------------------------------------
void getDataFromInfluxDB()
{
    float daykW = 0.0;
    float nightkW = 0.0;
    float totalkW = 0.0;
    
    String endpoint ="http://192.168.0.160:8086/query?db=energydb&q=select+max(t1)+from+log+where+time++%3E=+now()-5d+group+by+time(24h)";
    String payload="";

    if ((WiFi.status() == WL_CONNECTED)) 
    { //Check the current connection status
      //Serial.println("..getDataFromInfluxDB(WL_CONNECTED)");
      HTTPClient http;
      http.begin(endpoint); //Specify the URL
      int httpCode = http.GET();  //Make the request
      if (httpCode > 0)    //Check for the returning code
      { 
        payload = http.getString();
        char inp[payload.length()];
        payload.toCharArray(inp,payload.length());
        deserializeJson(doc,inp);

        String tt1String00=doc["results"][0]["series"][0]["values"][0][1];
        String tt1String01=doc["results"][0]["series"][0]["values"][1][1];
        String tt1String02=doc["results"][0]["series"][0]["values"][2][1];
        String tt1String03=doc["results"][0]["series"][0]["values"][3][1];
        String tt1String04=doc["results"][0]["series"][0]["values"][4][1];  // YESTERDAY
        String tt1String05=doc["results"][0]["series"][0]["values"][5][1];  // TODAY
        //Serial.print("....TODAY DAY: ");
        //Serial.println(tt1String05);

        float day04 = tt1String04.toFloat();
        float day05 = tt1String05.toFloat();

        daykW = day05 - day04; // DELTA TODAY
        //Serial.print("....DELTA DAY: ");
        //Serial.println(daykW);
        
        //tft.setFreeFont(&Orbitron_Medium_16);
        //tft.setTextColor(TFT_WHITE,TFT_BLACK);
        //tft.drawString("T1 (kwh):",4,60+4,2);
        //tft.setTextColor(green,TFT_BLACK);
        //tft.drawString(String(tt1day05) ,4,10+4);
      }
    }

    endpoint ="http://192.168.0.160:8086/query?db=energydb&q=select+max(t2)+from+log+where+time++%3E=+now()-5d+group+by+time(24h)";
    payload="";

    if ((WiFi.status() == WL_CONNECTED)) 
    { //Check the current connection status
      //Serial.println("..getDataFromInfluxDB(WL_CONNECTED)");
      HTTPClient http;
      http.begin(endpoint); //Specify the URL
      int httpCode = http.GET();  //Make the request
      if (httpCode > 0)    //Check for the returning code
      { 
        payload = http.getString();
        char inp[payload.length()];
        payload.toCharArray(inp,payload.length());
        deserializeJson(doc,inp);

        String tt1String00=doc["results"][0]["series"][0]["values"][0][1];
        String tt1String01=doc["results"][0]["series"][0]["values"][1][1];
        String tt1String02=doc["results"][0]["series"][0]["values"][2][1];
        String tt1String03=doc["results"][0]["series"][0]["values"][3][1];
        String tt1String04=doc["results"][0]["series"][0]["values"][4][1];  // YESTERDAY
        String tt1String05=doc["results"][0]["series"][0]["values"][5][1];  // TODAY
        //Serial.print("....TODAY NIGHT: ");
        //Serial.println(tt1String05);

        float day04 = tt1String04.toFloat();
        float day05 = tt1String05.toFloat();

        nightkW = day05 - day04; // DELTA TODAY
        //Serial.print("....DELTA NIGHT: ");
        //Serial.println(nightkW);
        
        //tft.setFreeFont(&Orbitron_Medium_16);
        //tft.setTextColor(TFT_WHITE,TFT_BLACK);
        //tft.drawString("T1 (kwh):",4,60+4,2);
        //tft.setTextColor(green,TFT_BLACK);

        //tft.drawString(String(tt1day05) ,4,10+4);
      }
    }

    totalkW = daykW + nightkW;
    Serial.print("..DELTA TOTAL: ");
    Serial.println(totalkW);

    tft.setFreeFont(&Orbitron_Medium_16);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawString("T1+T2 (kwh) TODAY:",4,60+4,2);
    tft.setTextColor(green,TFT_BLACK);
    tft.drawString(String(totalkW) ,4,10+4,6);
    
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
