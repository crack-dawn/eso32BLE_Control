#include <Arduino.h>
#include "weather.h"
 
void setup()
{
    Serial.begin(115200);
    Serial.println("running \r\n");
    pinMode(2, OUTPUT);
    Serial2.begin(115200);
    Serial.println("running2 \r\n");
 
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        vTaskDelay(2000);
        Serial.println(".-..--.-.  \r\n");
    }
    Serial.println("wifi OK  \r\n");
}

void loop()
{
    // put your main code here, to run repeatedly:
    digitalWrite(2, LOW);
    delay(500);
    digitalWrite(2, HIGH);
    delay(500);
    if (Serial.available() > 0)
    {
        vTaskDelay(50);
        String str = Serial.readString();
        while (Serial.read() >= 0)
            ; //清空缓存区
        Serial.println(str);

        if (str[0] == '#' && str[1] == 't')
        {
            Serial.println("Get_Now_Time  \r\n");
            Get_Now_Time();
        }
        else if (str[0] == '#' && str[1] == 'w')
        {
            Serial.println("Get_Rencent_Weather  \r\n");
            Get_Rencent_Weather(API, CITY);
        }
        else if (str[0] == '#' && str[1] == 'b')
        {
            Serial.println("蓝牙");
        }
 
    }
}

// #include <HardwareSerial.h>

// #define SERIAL_BAUD 115200
// //      串口名 Arduino名 TX RX
// //     UART0 Serial0 pin1  pin3
// //     UART1 Serial1 pin10 pin9
// //     UART2 Serial2 pin17 pin16
