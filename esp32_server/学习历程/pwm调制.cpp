#include <Arduino.h>
int calculatePWM(int degree)
{ // 0-180度
  // 20ms周期，高电平0.5-2.5ms，对应0-180度角度
    const float deadZone = 6.4; //对应0.5ms（0.5ms/(20ms/256）)
    const float max = 32;       //对应2.5ms
    if (degree < 0)
        degree = 0;
    if (degree > 180)
        degree = 180;
    return (int)(((max - deadZone) / 180) * degree + deadZone);
}

void pwm_init()
{
    // 通道， 频率， 分辨率
    ledcSetup(1, 50, 10);
    ledcSetup(2, 50, 8);
    //绑定引脚
    ledcAttachPin(2, 1);
    ledcAttachPin(4, 2);
}
void Sg90Ctrl(int x )
{
    ledcWrite(2, calculatePWM(x));
}
void pwm_set(int d)
{
    ledcWrite(1, d * 10);          // 输出PWM //首个参数为通道
    ledcWrite(2, calculatePWM(d)); // 输出PWM //首个参数为通道
}

void setup()
{
    Serial.begin(115200);
    Serial.println("running \r\n");
    pinMode(2, OUTPUT);

    pwm_init();
    Serial.println("running2 \r\n");
}


int d = 0;

void loop()
{
    if (Serial.available() > 0)
    {
        vTaskDelay(50);
        String str = Serial.readString();
        while (Serial.read() >= 0)
            ;
        Serial.println(str);
        d = str[0] - '0';
        ledcWrite(1, d * 10); // 输出PWM //首个参数为通道
    }
}
// #include <HardwareSerial.h>

// #define SERIAL_BAUD 115200
// //      串口名 Arduino名 TX RX
// //     UART0 Serial0 pin1  pin3
// //     UART1 Serial1 pin10 pin9
// //     UART2 Serial2 pin17 pin16
