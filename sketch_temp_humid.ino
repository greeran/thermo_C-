#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include <Arduino.h>
extern "C" {
#include <user_interface.h>
}
#include <ArduinoJson.h>
#include "FS.h"
#include "Gsender.h"
#include <vector>

/* Set these to your desired credentials. */
const char *ssid = "ESPap";
const char *password = "thereisnospoon";
int dhttemp=0,dhthumi=0,minitemp=30,maxtemp=60,minihumd=20,maxhumd=40;
const char disabled[]="";//"disabled";
String sendemail="greeranjunk@gmail.com";
String wifi_ssid="Wifi_SSID";
String wifi_password="Wifi_Password";
static int srv_clt_flag = 0;
static const String WIFISSID="wifissid";
static const String  WIFIPASS="wifipassword";
static const String MINITMP="minitemp";
static const String MAXTMP="maxtemp";
static const String MINIHMD="minihumd";
static const String MAXHMD="maxhumd";
static const String SNDEMAIL="sendemail";
static const String RESTCNT="resetcounter";
static const String TEMPSUM="tempsum";
static const String HUMDSUM="humidsum";
static const String RESTMISS="resetmiss";
static const String apiKey = !!1thekey!!!

#define DHTPIN 2     // what digital pin we're connected to

#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

class file_config
{
  public:
  file_config()
  {
    
      ssid="SSID";
      passwd="SSID PASSWORD";
      mintmp="20";
      maxtmp="40";
      minhmd="30";
      maxhmd="50";
      sndemail="youremail@google.com";
      resetCnt=0;
      tempSum=0;
      humidSum=0;
      resetMiss=0;
  }
  void file_serielize(JsonObject &json)
  {
      clean();
      json[WIFISSID].printTo(ssid);
      json[WIFIPASS].printTo(passwd);
      json[MAXTMP].printTo(maxtmp);
      json[MINITMP].printTo(mintmp);
      json[MINIHMD].printTo(minhmd);
      json[MAXHMD].printTo(maxhmd);
      json[SNDEMAIL].printTo(sndemail);
      String a(resetCnt);
      String b(tempSum);
      String c(humidSum);
      String d(resetMiss);
      json[RESTCNT].printTo(a);
      json[TEMPSUM].printTo(b);
      json[HUMDSUM].printTo(c);
      json[RESTMISS].printTo(d);
      Serial.println("test save to jason1 sndmai:"+sndemail+" ssid:"+ssid);

  }
  
  void file_serielize(ESP8266WebServer &server)
  {
      
      ssid=server.arg(WIFISSID);
      passwd=server.arg(WIFIPASS);
      mintmp=server.arg(MINITMP);
      maxtmp=server.arg(MAXTMP);
      minhmd=server.arg(MINIHMD);
      maxhmd=server.arg(MAXHMD);
      sndemail=server.arg(SNDEMAIL);
      Serial.println("test save from srv sndmai:"+sndemail);
  }
  void file_deserilize(JsonObject &json)
  {
      json[WIFISSID]=ssid;
      json[WIFIPASS]=passwd;
      json[MAXTMP]=maxtmp;
      json[MINITMP]=mintmp;
      json[MINIHMD]=minhmd;
      json[MAXHMD]=maxhmd;
      json[SNDEMAIL]=sndemail;
      json[RESTCNT]= resetCnt;
      json[TEMPSUM]= tempSum;
      json[HUMDSUM]=humidSum;
      json[RESTMISS]= resetMiss;
      Serial.println("test to jason2 from param sndmail:"+sndemail);
  }

  String printConf()
  {
    return ssid+" "+passwd+" "+mintmp+" "+maxtmp+" "+minhmd+" "+maxhmd+" "+sndemail + " "+resetCnt+ " "+tempSum+" "+resetMiss;
  }
  String ssid;
  String passwd;
  String mintmp;
  String maxtmp;
  String minhmd;
  String maxhmd;
  String sndemail;
  unsigned int resetCnt;
  unsigned int tempSum;
  unsigned int humidSum;
  unsigned int resetMiss;

private:
  void clean()
  {
    ssid="";
    passwd="";
    mintmp="";
    maxtmp="";
    minhmd="";
    maxhmd="";
    sndemail="";
  }
  
};

ESP8266WebServer server(80);
file_config conffile;



bool saveConfig()
{
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject(); 
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return false;
    }
    Serial.println("Saving Values to File");
    conffile.file_serielize(server);
    Serial.println("Saving Values to json");
    conffile.file_deserilize(json);
    json.printTo(Serial);
    json.printTo(configFile);              
    configFile.close();
    return true;
}

bool loadConfig()
{
    if(SPIFFS.exists("/config.json"))
    {
      File configFile = SPIFFS.open("/config.json", "r");
      if (!configFile) {
        Serial.println("Failed to open config file for reading");
        return false;
      }
      size_t size=configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(),size);
      StaticJsonBuffer<256> tmpJsonBuffer;
      JsonObject& json = tmpJsonBuffer.parseObject(buf.get());
      if(!json.success())
      {
        Serial.println("Failed to parse config file");
        //TODO: go to default
        configFile.close();
        return -1; 
      }else
      {
        conffile.file_serielize(json);
      }
      configFile.close();
      configFile = SPIFFS.open("/config.json", "r");
      Serial.println("got file");
      while(configFile.available())
      {
        Serial.print(configFile.readString());
      }
      Serial.println("\nfile ended");
      configFile.close();
    } 
    return 0;
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  char temp[2048];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  if(3 < server.args())
  {    
      //save config
      saveConfig();
      // test
      srv_clt_flag=1;
      return;
  }else
  {
    loadConfig();
    Serial.println("after the load");
  }
  

  Serial.println("tste conf");
  Serial.print(conffile.printConf());
  Serial.println("\nafter test");
  snprintf ( temp, 2048,
"<html>\
  <head>\
    <meta/>\
    <title>Thermometer Setting</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Thermometer Setting</h1>\
    <p><h2>Temprature %d: Humidity %d </h2></p>\
    <p>Get Tempruture and Humidity <a href=\"?getTemp=Temp\"><button>Press</button></a>&nbsp;</p>\
    <p>Send To Thingspeak <a href=\"?sendThings=Thing\"><button>Press</button></a>&nbsp;</p>\
    <form>\
      <p>Wifi-SSID <input type=\"text\" %s name=\"wifissid\" value=%s>&nbsp;</p>\
      <p>Wifi-Password <input type=\"text\" %s name=\"wifipassword\" value=%s>&nbsp;</p>\
      <p>Min Temprature <input type=\"text\" %s name=\"minitemp\" value=%s>&nbsp;</p>\
      <p>Max Temprature <input type=\"text\" %s name=\"maxtemp\" value=%s>&nbsp;</p>\
      <p>Min Humidity <input type=\"text\" %s name=\"minihumd\" value=%s>&nbsp;</p>\
      <p>Max Humitidy <input type=\"text\" %s name=\"maxhumd\" value=%s>&nbsp;</p>\
      <p>notification Mail <input name=\"sendemail\" %s type=\"email\" value=%s \ 
      <input type=\"submit\" name=\"action\" value=\"Test Mail\"</p>\
      <p><input type=\"submit\" name=\"action\" %s value=\"Start Working\" </sp></a>\
    </form>&nbsp;</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",
  dhttemp,dhthumi,
  disabled,conffile.ssid.c_str(),
  disabled,conffile.passwd.c_str(),
  disabled,conffile.mintmp.c_str(),
  disabled,conffile.maxtmp.c_str(),
  disabled,conffile.minhmd.c_str(),
  disabled,conffile.maxhmd.c_str(),
  disabled,conffile.sndemail.c_str()
  );
	server.send(200, "text/html", temp);
  String message = "RAN TEST Message: ";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  Serial.print(message);
  Serial.println("finished test print");
}

int connectToClient()
{
  int isConnected=0;
  WiFi.begin();
  for(int i=0;i<20;i++)
  {
    if(WiFi.status() == WL_CONNECTED)
    {
      isConnected=1;
      break;
    }
    delay(500);
    Serial.print(".");
  }
  if(1==isConnected)
  {
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  return isConnected;
}

void connectToAccessPoint()
{
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.println(ESP.getResetReason());
	Serial.println("Mounting FS");
	if (!SPIFFS.begin()) {
		Serial.println("Failed to mount file system");
		return;
	}
	dht.begin();
	Serial.println("Dht module started");

	Serial.println("Testing Connection");
	if(REASON_EXT_SYS_RST == ESP.getResetInfoPtr()->reason)
	{
		Serial.println("Extenal Reset so disconnect wifi");
		WiFi.disconnect(true);
		connectToAccessPoint();
		srv_clt_flag=0;
	}
	else
	{
	// try to connect to Wifi as client
		if(0==connectToClient())
		{
		  //failed start as AP
			Serial.println("Failed to connect as client. Starting Access Point");
			connectToAccessPoint();
			srv_clt_flag=0;
		}else
		{
			loadConfig();
			srv_clt_flag=2;
		}
	}
}

int sendMessageToGMail(String subject,String msg)
{
	Serial.print("sending msg to email: ");
	String tmpSndMail=conffile.sndemail;
  tmpSndMail.replace("\"","");
	Serial.println(conffile.sndemail);
  Serial.print("tmpsend mail:");
  Serial.println(tmpSndMail);
  
	int retVal=0;
	Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
	if(gsender->Subject(subject)->Send(tmpSndMail, msg)) {
	  Serial.println("Message sent.");
	} else {
	  retVal=-1;
	  Serial.print("Error sending message: ");
	  Serial.println(gsender->getError());
	}
	return retVal;
}

int sendThinkSpeak()
{
	WiFiClient client;
	if (client.connect("api.thingspeak.com",80)) {  //   "184.106.153.149" or api.thingspeak.com
	    	String postStr = apiKey;
			postStr +="&field1=";
			postStr += String(dhttemp);
			postStr +="&field2=";
			postStr += String(dhthumi);
			postStr += "\r\n\r\n";
			client.print("POST /update HTTP/1.1\n");
			client.print("Host: api.thingspeak.com\n");
			client.print("Connection: close\n");
			client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
			client.print("Content-Type: application/x-www-form-urlencoded\n");
			client.print("Content-Length: ");
			client.print(postStr.length());
			client.print("\n\n");
			client.print(postStr);
			return 0;
	}
	return -1;
}

const static unsigned int CYCLE_NUM=24;
int startWork()
{
	conffile.resetCnt++;
	if(0!=readTmpAndHmd())
	{
		conffile.resetMiss++;
		//send mail dht failed
		sendMessageToGMail("Your sensor failed","Sensor failed to get temperature and humidity");
		conffile.resetCnt--;
		conffile.resetMiss++;
	}else
	{
		conffile.tempSum+=dhttemp;
		conffile.humidSum+=dhthumi;
		if((dhttemp < conffile.mintmp.toInt()) ||
				(dhthumi < conffile.minhmd.toInt()) ||
				(dhttemp > conffile.maxtmp.toInt()) ||
				(dhttemp > conffile.maxhmd.toInt()) )
		{
			// not in range
			// send mail not in range
		  String rangeFail="Range failed temperature=";
		  rangeFail+=dhttemp;
		  rangeFail+=" humidity";
		  rangeFail+=dhthumi;
		  sendMessageToGMail("Sensor out of range",rangeFail);
		}
		conffile.tempSum+=dhttemp;
		conffile.humidSum+=dhthumi;
		sendThinkSpeak();
	}

	//if((CYCLE_NUM-1) >= conffile.resetCnt)
	if(1)
	{
		//send update
		// reset all for next 24 hours
		unsigned int avgTemp=conffile.tempSum/conffile.resetCnt;
		unsigned int avgHumd=conffile.humidSum/conffile.resetCnt;

		String strSummery("Temperature and humidity summery");
		strSummery+="\nTemperature average "+avgTemp;
		strSummery+="\nHumid average "+avgHumd;
		strSummery+="\nmissed "+conffile.resetMiss;

		sendMessageToGMail("Sensor day summery",strSummery);
		conffile.resetCnt = 0;
		conffile.resetMiss=0;
		conffile.tempSum = 0;
		conffile.humidSum = 0;
	}


  Serial.println("go to deepsleep");
  ESP.deepSleep(20 * 1000000);
  srv_clt_flag=3;
  return 0;  
}

void test_connection()
{
  WiFi.begin("Redmi", "********");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void switchFromAPtoClt()
{
  //dissconnect server
  server.stop();
  //dissconnect as access point
  Serial.println("Disconnecting AP");
  WiFi.softAPdisconnect(true);
  Serial.print("Connection to Station Mode ");
  Serial.print(conffile.ssid.c_str());
  Serial.print(" ");
  Serial.println(conffile.passwd.c_str());
  WiFi.begin(conffile.ssid.c_str(), conffile.passwd.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

int readTmpAndHmd()
{
	int retVal=-1;
	for(int i=0;i<3;i++)
	{
		delay(2000);
		float h = (int)dht.readHumidity();
		// Read temperature as Celsius (the default)
		float t = (int)dht.readTemperature();
		if (isnan(h) && isnan(t) && (0>h) && (0>t)) {
			Serial.println("Failed to read from DHT sensor!");
			continue;
		}
		retVal=0;
		dhttemp = (int)t;
		dhthumi = (int)h;
		Serial.print("DHT sensor Success temp: ");
		Serial.print(t);
		Serial.print(" humd: ");
		Serial.println(h);
		break;
	}
	return retVal;

}

void loop() {

  switch (srv_clt_flag)
  {
    case 0: 
      readTmpAndHmd();
      server.handleClient();
      break;
    case 1:
      switchFromAPtoClt();
      srv_clt_flag=2;
      break;
    case 2:

      Serial.println("start working");
      srv_clt_flag=startWork();
      Serial.print("Returned from work with ");
      Serial.println(srv_clt_flag);
      break;
    default: 
      Serial.println("in default");
      break;
  }
}
