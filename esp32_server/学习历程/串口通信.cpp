#include <HardwareSerial.h>

#define SERIAL_BAUD 115200

HardwareSerial Serial1(1); //��������1

int distance = 0;

//      ������ Arduino�� TX RX
//     UART0 Serial0 pin1  pin3
//     UART1 Serial1 pin10 pin9
//     UART2 Serial2 pin17 pin16

void setup()
{
    //��ʼ������0
    Serial.begin(SERIAL_BAUD);
    //��ʼ������1
    Serial1.begin(SERIAL_BAUD, SERIAL_8N1, 12, 13);
    //��ʼ������2
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
 