
#include "weather.h"


const char ssid[20] = "233";
const char password[20] = "12345678";

void Get_Now_Time()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(2000);
    }
    configTime(3600 * 7, 3600, "us.pool.ntp.org");

    static struct tm timeinfo;
    
    if (!getLocalTime(&timeinfo))
    {
        if (!getLocalTime(&timeinfo))
            return;
    }
    static u8_t data[25];

    data[0] = '#';
    data[1] = 'T';

    data[2] = timeinfo.tm_year;
    data[3] = timeinfo.tm_mon+1;
    data[4] = timeinfo.tm_mday;

    data[5] = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;

    data[6] = timeinfo.tm_hour;
    data[7] = timeinfo.tm_min;
    data[8] = timeinfo.tm_sec;

    data[9] = '\r';
    data[10] ='\n';

    for(u_char i=3; i<=8; ++i)
    {
        data[i] +='0';
    }
    Serial.write(data, 11);

    //Serial.println(&timeinfo, "#T-%Y-%m-%d-%w-%H:%M:%S"); //我需要的 时间 年月日 + 星期几 + 时分秒
    vTaskDelay(30);
}

void Get_Today_Weather(String api, String city)
{
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(2000);
    }
    String url_xinzhi = "";
    url_xinzhi = "https://api.seniverse.com/v3/weather/now.json?key=";
    url_xinzhi += api;
    url_xinzhi += "&location=";
    url_xinzhi += city;
    url_xinzhi += "&language=en&unit=c";

    DynamicJsonDocument doc(1536); //分配内存,动态

    static HTTPClient http;
    static int httpGet;

    char data[50] = {"#w:"};

    http.begin(url_xinzhi);

    httpGet = http.GET();

    if (httpGet > 0)
    {
        if (httpGet == HTTP_CODE_OK)
        {
            String json = http.getString();

            deserializeJson(doc, json);

            strcat(data, doc["results"][0]["now"]["code"].as<const char *>());
            strcat(data, ":");
            strcat(data, doc["results"][0]["now"]["temperature"].as<const char *>());
            strcat(data, "$");

            Serial.println(data); //我需要的天气数据
        }
    }
    http.end();
}

void Get_Rencent_Weather(String api, String city)
{

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(2000);
    }

    String url_xinzhi = "";
    url_xinzhi = "https://api.seniverse.com/v3/weather/daily.json?key=";
    url_xinzhi += api;
    url_xinzhi += "&location=";
    url_xinzhi += city;
    url_xinzhi += "&language=en&unit=c";
    url_xinzhi += "&start=0&days=3";

    DynamicJsonDocument doc(2048); //分配内存,动态

    static HTTPClient http;
    static int httpGet;

    http.begin(url_xinzhi);
    httpGet = http.GET();

    if (httpGet > 0)
    {
        if (httpGet == HTTP_CODE_OK)
        {
            String json = http.getString();

            deserializeJson(doc, json);

            JsonObject root = doc.as<JsonObject>();
            JsonArray results = root["results"];

            static u8_t data[25];

            data[0] = '#';
            data[1] = 'W';
            data[8] = '\r';
            data[9] = '\n';
 
            for (u8_t i = 0; i < 3; ++i)
            {

                data[i + 2] = atoi(results[0]["daily"][i]["code_day"])+'0';

                data[i + 2 + 3] = atoi(results[0]["daily"][i]["code_night"])+'0';
            }
           
            Serial.write(data, 10); // 以 二进制/ HEX 码发送   8bit
        }
    }
    http.end();
}