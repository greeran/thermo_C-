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
static const String apiKey = "61TUR14P6J0LX5OU";

#define DHTPIN 2     // what digital pin we're connected to

#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

class file_config
{
  public:
  file_config()
  {
    
      ssid="SSID";
      passwd="SSID_PASSWORD";
      mintmp="20";
      maxtmp="40";
      minhmd="30";
      maxhmd="50";
      sndemail="youremail@google.com";
      resetCnt=1;
      tempSum=0;
      humidSum=0;
      resetMiss=0;
  }
  void file_serielize(JsonObject &json)
  {

	  clean();
      json[WIFISSID].printTo(ssid);
      ssid.replace("\"","");
      json[WIFIPASS].printTo(passwd);
      passwd.replace("\"","");
      json[MAXTMP].printTo(maxtmp);
      maxtmp.replace("\"","");
      json[MINITMP].printTo(mintmp);
      mintmp.replace("\"","");
      json[MINIHMD].printTo(minhmd);
      minhmd.replace("\"","");
      json[MAXHMD].printTo(maxhmd);
      maxhmd.replace("\"","");
      json[SNDEMAIL].printTo(sndemail);
      sndemail.replace("\"","");
      String a(resetCnt);
      String b(tempSum);
      String c(humidSum);
      String d(resetMiss);
      json[RESTCNT].printTo(a);
      a.replace("\"","");
      resetCnt=a.toInt();
      json[TEMPSUM].printTo(b);
      b.replace("\"","");
      tempSum=b.toInt();
      json[HUMDSUM].printTo(c);
      c.replace("\"","");
      humidSum=c.toInt();
      json[RESTMISS].printTo(d);
      d.replace("\"","");
      resetMiss=d.toInt();

  }
  
  void file_serielize(ESP8266WebServer &server)
  {
	  Serial.println("test save from web (b4 clean) sndmai:"+sndemail+" ssid:"+ssid + " passwd:"+passwd);
      clean();
      ssid=server.arg(WIFISSID);
      Serial.println("TEST b4 replace the ssid "+ssid);
      ssid.replace("\"","");
      passwd=server.arg(WIFIPASS);
      passwd.replace("\"","");
      mintmp=server.arg(MINITMP);
      mintmp.replace("\"","");
      maxtmp=server.arg(MAXTMP);
      maxtmp.replace("\"","");
      minhmd=server.arg(MINIHMD);
      minhmd.replace("\"","");
      maxhmd=server.arg(MAXHMD);
      maxhmd.replace("\"","");
      sndemail=server.arg(SNDEMAIL);
      sndemail.replace("\"","");
      Serial.println("test save from web (after clean) sndmai:"+sndemail+" ssid:"+ssid+ " passwd:"+passwd);
  }
  void file_deserilize(JsonObject &json)
  {

      json[WIFISSID]=ssid;
      Serial.print("RAN TEST !! ssid: ");
      Serial.print(ssid);
      Serial.print(" jason: ");
      json[WIFIPASS].printTo(Serial);
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
    resetCnt=0;
    tempSum=0;
    humidSum=0;
    resetMiss=0;
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
    Serial.println("TEST print after saving to jason");
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
      StaticJsonBuffer<512> tmpJsonBuffer;
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
      Serial.println("Test read file on loading");
      while(configFile.available())
      {
        Serial.print(configFile.readString());
      }
      Serial.println(" ");
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
  if(5 < server.args())
  {    
      //save config
      saveConfig();
      srv_clt_flag=1;
      return;
  }else
  {
    loadConfig();
  }
  
  Serial.print(conffile.printConf());
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
/*  String message = "RAN TEST Message: ";
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
*/
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

	Serial.print("Testing Connection (reset reason)");
	Serial.println(ESP.getResetInfoPtr()->reason);
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
	readTmpAndHmd();
}

int sendMessageToGMail(const String subject,const String msg)
{
	Serial.print("sending msg to email: ");
	String tmpSndMail(conffile.sndemail);
	tmpSndMail.replace("\"","");
	Serial.println(tmpSndMail);
  
	int retVal=0;
	Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
	if(gsender->Subject(subject)->Send(tmpSndMail, msg)) {
	  Serial.print("Message sent: ");
	  Serial.print(msg);
	  Serial.println(" ");
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

const static unsigned int CYCLE_NUM=5;
int startWork()
{
	conffile.resetCnt++;
	Serial.println("TEST RAN start work");
	Serial.println(conffile.printConf());
	if(0!=readTmpAndHmd())
	{
		conffile.resetMiss++;
		//send mail dht failed
		sendMessageToGMail("Your sensor failed","Sensor failed to get temperature and humidity");
		conffile.resetCnt--;
		conffile.resetMiss++;
	}else
	{
		/*String tmpoutput="B4 tmpSum:";
		tmpoutput+=conffile.tempSum;
		tmpoutput+=" hmdSum:";
		tmpoutput+=conffile.humidSum;
		tmpoutput+=" dhttmp:";
		tmpoutput+=dhttemp;
		tmpoutput+=" dhthmi:";
		tmpoutput+=dhthumi;
		Serial.println(tmpoutput);*/
		conffile.tempSum+=dhttemp;
		conffile.humidSum+=dhthumi;
		/*tmpoutput="after tmpSum:";
		tmpoutput+=conffile.tempSum;
		tmpoutput+=" hmdSum:";
		tmpoutput+=conffile.humidSum;
		tmpoutput+=" dhttmp:";
		tmpoutput=+dhttemp;
		tmpoutput=+" dhthmi:";
		tmpoutput=+dhthumi;*/

		// TEST TEST TEST

		int confmintmp,confminhmd,confmaxhmd,confmaxtmp;
		String tmp = conffile.mintmp;
		tmp.replace("\"","");
		confmintmp=tmp.toInt();
		tmp = conffile.minhmd;
		tmp.replace("\"","");
		confminhmd=tmp.toInt();
		tmp = conffile.maxtmp;
		tmp.replace("\"","");
		confmaxtmp=tmp.toInt();
		tmp = conffile.maxhmd;
		tmp.replace("\"","");
		confmaxhmd=tmp.toInt();


		if((dhttemp < confmintmp) ||
				(dhthumi < confminhmd) ||
				(dhttemp > confmaxtmp) ||
				(dhthumi > confmaxhmd) )
		{
			String tmpou=("InOut of range "+conffile.minhmd +" "
					+conffile.mintmp +" "
					+conffile.maxhmd +" "
					+conffile.maxtmp +" ");
			Serial.println(tmpou);
			// not in range
			// send mail not in range
		  String rangeFail="Range failed temperature=";
		  rangeFail+=dhttemp;
		  rangeFail+=" humidity";
		  rangeFail+=dhthumi;
		  sendMessageToGMail("Sensor out of range",rangeFail);
		}
		//conffile.tempSum+=dhttemp;
		//conffile.humidSum+=dhthumi;
		sendThinkSpeak();
	}

	if((CYCLE_NUM-1) == conffile.resetCnt)
	//if(1)
	{
		//send update
		// reset all for next 24 hours
		unsigned int avgTemp=conffile.tempSum/conffile.resetCnt;
		unsigned int avgHumd=conffile.humidSum/conffile.resetCnt;

		String strSummery="Temperature and humidity summery\nTemperature average ";
		strSummery+=avgTemp;
		strSummery+="\nHumid average ";
		strSummery+=avgHumd;
		strSummery+="\nmissed ";
		strSummery+=conffile.resetMiss;

		sendMessageToGMail("Sensor day summery",strSummery);
		conffile.resetCnt = 1;
		conffile.resetMiss=0;
		conffile.tempSum = 0;
		conffile.humidSum = 0;
	}
	//save pramas to file
	StaticJsonBuffer<512> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
	  Serial.println("Failed to open config file for writing");
	  return false;
	}
	Serial.println("save config before deepsleep");
	Serial.println(conffile.printConf());
	conffile.file_deserilize(json);
	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
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
	for(int i=0;i<5;i++)
	{
		delay(2000);
		float h = (int)dht.readHumidity();
		// Read temperature as Celsius (the default)
		float t = (int)dht.readTemperature();
		if (isnan(h) && isnan(t) ) {
			Serial.println("Failed 1 to read from DHT sensor!");
			continue;
		}
		if ((0>h) && (0>t))
		{
			Serial.println("Failed 2 to read from DHT sensor!");
			continue;
		}
		if ((200<h) && (200<t))
		{
			Serial.println("Failed 3 to read from DHT sensor!");
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
