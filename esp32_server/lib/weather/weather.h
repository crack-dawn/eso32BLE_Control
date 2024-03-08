#ifndef _weather_H_
#define _weather_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi相关
extern const char ssid[20] ;
extern const char password[20] ;

//心知天气 相关
static String API = "SJ3cQqGjCfr2Ej1oS";
static String CITY = "yichang";

void Get_Now_Time() ; //NTP 获取当前时间/日期
void Get_Today_Weather(String api, String city) ;//https 获取当日天气
void Get_Rencent_Weather(String api, String city) ;//https 获取 今明后 三日天气

#endif // !_weather_H_
 