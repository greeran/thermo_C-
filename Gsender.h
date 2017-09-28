/* Gsender class helps send e-mails from Gmail account
*  using Arduino core for ESP8266 WiFi chip
*  by Boris Shobat
*  September 29 2016
*/
#ifndef G_SENDER
#define G_SENDER
#define GS_SERIAL_LOG_1         // Print to Serial only server responce
//#define GS_SERIAL_LOG_2       //  Print to Serial client commands and server responce
#include <WiFiClientSecure.h>

class Gsender
{
    protected:
        Gsender();
    private:
        const int SMTP_PORT = 465;//587;//465;
        const char* SMTP_SERVER = "smtp.mailgun.org";
        const char* EMAILBASE64_LOGIN = "cG9zdG1hc3RlckBzYW5kYm94NTQxNWE1YWUyZjk0NDRjNjg4MzE1NTBlYjE5NmNlOGIubWFpbGd1bi5vcmc=";
        const char* EMAILBASE64_PASSWORD = "UUFaV1NYMTI=";
        const char* FROM = "greeranjunk@gmail.com";
        const char* _error = nullptr;
        char* _subject = nullptr;
        String _serverResponce;
        static Gsender* _instance;
        bool AwaitSMTPResponse(WiFiClientSecure &client, const String &resp = "", uint16_t timeOut = 10000);

    public:
        static Gsender* Instance();
        Gsender* Subject(const char* subject);
        Gsender* Subject(const String &subject);
        bool Send(const String &to, const String &message);
        String getLastResponce();
        const char* getError();
};
#endif // G_SENDER
