 
#include "sg90.h"

#define sg90_data_pin 4
#define pwm_channel 2


int calculatePWM(int degree)
{                               // 0-180度
                                // 20ms周期，高电平0.5-2.5ms，对应0-180度角度
    const float deadZone = 6.4; //对应0.5ms（0.5ms/(20ms/256）)
    const float max = 32;       //对应2.5ms
    if (degree < 0)
        degree = 0;
    if (degree > 180)
        degree = 180;
    return (int)(((max - deadZone) / 180) * degree + deadZone);
}

void Sg90Init()
{
    pinMode(sg90_data_pin, OUTPUT);
    // 通道， 频率， 分辨率
    ledcSetup(pwm_channel, 50, 8);
    //绑定引脚
    ledcAttachPin(sg90_data_pin, pwm_channel);
}

void Sg90Ctrl(int x)
{
    int result = 0 ;
    switch (x)
    {
        case 1:
            result=calculatePWM(0);
            break;
        case 2:
            result = calculatePWM(50);
            break;
        case 3:
            result = calculatePWM(70);
            break;
        case 4:
            result = calculatePWM(90);
            break;
        case 5:
            result = calculatePWM(135);
            break;
        case 6:
            result = calculatePWM(180);
            break;
        default:
            break;
    }
    ledcWrite(pwm_channel, result);
}
