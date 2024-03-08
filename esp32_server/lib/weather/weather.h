#ifndef _weather_H_
#define _weather_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi���
extern const char ssid[20] ;
extern const char password[20] ;

//��֪���� ���
static String API = "SJ3cQqGjCfr2Ej1oS";
static String CITY = "yichang";

void Get_Now_Time() ; //NTP ��ȡ��ǰʱ��/����
void Get_Today_Weather(String api, String city) ;//https ��ȡ��������
void Get_Rencent_Weather(String api, String city) ;//https ��ȡ ������ ��������

#endif // !_weather_H_
 