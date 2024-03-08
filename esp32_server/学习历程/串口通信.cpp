#include <HardwareSerial.h>

#define SERIAL_BAUD 115200

HardwareSerial Serial1(1); //声明串口1

int distance = 0;

//      串口名 Arduino名 TX RX
//     UART0 Serial0 pin1  pin3
//     UART1 Serial1 pin10 pin9
//     UART2 Serial2 pin17 pin16

void setup()
{
    //初始化串口0
    Serial.begin(SERIAL_BAUD);
    //初始化串口1
    Serial1.begin(SERIAL_BAUD, SERIAL_8N1, 12, 13);
    //初始化串口2
    Serial2.begin(SERIAL_BAUD);


    
}

void loop()
{
    while (Serial2.available() > 0)
    {
        uint8_t byteFromSerial = Serial2.read();
        Serial2.print(byteFromSerial);
    }

}
 