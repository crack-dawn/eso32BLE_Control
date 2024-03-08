#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
// WiFi相关
const char *ssid = "233";
const char *password = "12345678";

//心知天气 相关
String API = "SJ3cQqGjCfr2Ej1oS";
String CITY = "luoyang";

// 定时器相关
u32_t cnt = 0;

void Get_Now_Time();                               // NTP 获取当前时间/日期
void Get_Today_Weather(String api, String city);   // https 获取当日天气
void Get_Rencent_Weather(String api, String city); // https 获取 今明后 三日天气

void IRAM_ATTR Tim0_Handle(void)
{
    ++cnt;
    if (cnt == 300) //五分钟一个循环
    {
        cnt = 0;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("串口初始化成功");

    hw_timer_t *tim0 = timerBegin(0, 8000, true);  //建立定时器  //80MHZ 进行 8000预分频后 每秒 计数10000次
    timerAlarmWrite(tim0, 10000 - 1, true);        //设置 自动重装载值 10000-1
    timerAttachInterrupt(tim0, Tim0_Handle, true); //绑定定时器
    timerAlarmEnable(tim0);
}

void loop()
{
    switch (cnt)
    {
    case 3:
        if (WiFi.status() != WL_CONNECTED)
        {
            WiFi.begin(ssid, password);
            vTaskDelay(100);
        }
        Serial.println("WiFi开启");
        ++cnt;
        break;

    case 8:
        Serial.println("现在的时间是");
        Get_Now_Time();
        Serial.println("今天的天气是");
        Get_Today_Weather(API, CITY);
        Serial.println("近三天的天气是");
        Get_Rencent_Weather(API, CITY);
        ++cnt;
        break;

    case 18:
        Serial.println("WiFi关闭");
        WiFi.disconnect(true); //关闭wifi ，降低功耗
        WiFi.mode(WIFI_OFF);
        ++cnt;

    default:
        break;
    }
}

void Get_Now_Time()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(100);
    }

    configTime(3600 * 7, 3600, "us.pool.ntp.org");

    static struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        if (!getLocalTime(&timeinfo))
            return;
    }

    Serial.println(&timeinfo, "#T-%Y-%m-%d-%w-%H:%M:%S"); //我需要的 时间 年月日 + 星期几 + 时分秒
    vTaskDelay(2);
}
void Get_Today_Weather(String api, String city)
{
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(100);
    }

    String url_xinzhi = "";
    url_xinzhi = "https://api.seniverse.com/v3/weather/now.json?key=";
    url_xinzhi += api;
    url_xinzhi += "&location=";
    url_xinzhi += city;
    url_xinzhi += "&language=en&unit=c";

    DynamicJsonDocument doc(2048); //分配内存,动态

    HTTPClient http;
    char data[100] = {"#w:"};

    http.begin(url_xinzhi);

    int httpGet = http.GET();

    if (httpGet > 0)
    {
        if (httpGet == HTTP_CODE_OK)
        {
            String json = http.getString();
            Serial.println(json);
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
        vTaskDelay(100);
    }

    String url_xinzhi = "";
    url_xinzhi = "https://api.seniverse.com/v3/weather/daily.json?key=";
    url_xinzhi += api;
    url_xinzhi += "&location=";
    url_xinzhi += city;
    url_xinzhi += "&language=en&unit=c";
    url_xinzhi += "&start=0&days=3";

    DynamicJsonDocument doc(2048); //分配内存,动态

    HTTPClient http;
    char data[100] = {"#W:"};

    http.begin(url_xinzhi);

    int httpGet = http.GET();

    if (httpGet > 0)
    {
        if (httpGet == HTTP_CODE_OK)
        {
            String json = http.getString();
            Serial.println(json);

            deserializeJson(doc, json);

            // JsonObject root = doc.as<JsonObject>();
            // JsonArray results = root["results"];
            // const char *a = results[0]["last_update"];
            // const char *b = results[0]["daily"][0]["text_day"];

            strcat(data, doc["results"][0]["daily"][0]["code_day"].as<const char *>());
            strcat(data, ":");
            strcat(data, doc["results"][0]["daily"][1]["code_day"].as<const char *>());
            strcat(data, ":");
            strcat(data, doc["results"][0]["daily"][2]["code_day"].as<const char *>());
            strcat(data, ":");

            strcat(data, doc["results"][0]["daily"][0]["code_night"].as<const char *>());
            strcat(data, ":");
            strcat(data, doc["results"][0]["daily"][1]["code_night"].as<const char *>());
            strcat(data, ":");
            strcat(data, doc["results"][0]["daily"][2]["code_night"].as<const char *>());
            strcat(data, "$");

            Serial.println(data);
        }
    }
    http.end();
}
