// // 微信蓝牙小程序
// /*
//     Video: https://www.youtube.com/watch?v=oCMOYS71NIU
//     Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
//     Ported to Arduino ESP32 by Evandro Copercini
//    Create a BLE server that, once we receive a connection, will send periodic notifications.
//    创建一个BLE服务器，一旦我们收到连接，将会周期性发送通知0
//    T使用步骤：
//    1. 创建一个 BLE Server
//    2. 创建一个 BLE Service
//    3. 创建一个 BLE Characteristic
//    4. 创建一个 BLE Descriptor
//    5. 开始服务
//    6. 开始广播
// */
#include <common.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string.h>
#include <DHT.h>
#include "sg90.h"
#include <esp32-hal-timer.h>

//蓝牙部分
void SendBack(String s) ;
int my_strlen(unsigned char chr[]);
String chipId;
unsigned char txValue[20] = "hello-BLE";
BLEServer *pServer = NULL;                   // BLEServer指针 pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristic指针 pTxCharacteristic
bool deviceConnected = false;                //本次连接状态
bool oldDeviceConnected = false;             //上次连接状态

// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // service UUID服务特征码
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" //  接收特征码  RX
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //  发送特征码   TX


/*dht11部分*/
#define DHT11_data_pin 5
#define DHT_TYPE_MY DHT11
DHT Dht11 = DHT(DHT11_data_pin, DHT_TYPE_MY);
void DHT11_UPdate();
unsigned char  st[4];
/*定时器部分*/
// 定义中断函数：【中断应加载到IRAM中，且无返回值】
void IRAM_ATTR onTimer(){
    if (deviceConnected == true){
        DHT11_UPdate();
        Serial.write(st, 4);
    }
}
hw_timer_t *my_timer1 = timerBegin(0, uint16_t(8000000), true);
void time_Init_my(hw_timer_t *Timer)//,uint16_t psc, uint16_t val)
{
    // Timer = timerBegin(Timer, psc, true);
    //定时器的型号选用 预分频【主频：80MHz】 定时器上下计数【true？】
    timerAlarmWrite(Timer, 40, true);
    // 初始化完毕候，将定时器连接到中断： 定时器地址指针 中断处理函数 中断边沿触发类型
    timerAttachInterrupt(Timer, &onTimer, true);
    timerSetCountUp(Timer, true);
    // 定时： 操作的定时器 定时时长 数值是否重载【周期定时？】
    
    // 开始启动： 启动哪一个定时器？
    timerAlarmEnable(Timer);
}


class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        digitalWrite(2, 1);
        Serial.println(" 已经链接 ");
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        digitalWrite(2, 0);
        Serial.println(" 断开连接 ");
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue(); //接收信息

        if (rxValue.length() > 0)
        { 
            switch(rxValue[0])
            {  
                case ('@'): Sg90Ctrl(rxValue[1] - '0');  SendBack("yes");       break;
                case ('#'): digitalWrite(2,rxValue[1] - '0'); SendBack("ok");   break;
                default:                                                        break;
            }
        }
    }
};



void setup()
{
    time_Init_my(my_timer1);
    Dht11.begin();
    //串口初始化
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    Sg90Init();

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //获取ESP开发板唯一的MAC地址
    chipId.toUpperCase();                              //字符串全部转大写
    Serial.println(chipId.c_str()); //打印chipId信息  UDID

   
    delay(2002);
    
    // 创建一个 BLE 设备
    BLEDevice::init("BLE_test");
    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //设置回调
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // 创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY); //设置发信特征码
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE); //设置收信特征码
    pRxCharacteristic->setCallbacks(new MyCallbacks());                                                                               //设置回调
    pService->start();                  // 开始服务
    pServer->getAdvertising()->start(); // 开始广播
    Serial.println(" wait connect and notify... ");
}

String readString;

void loop()
{

    if (deviceConnected)
    {
        DHT11_UPdate();
        vTaskDelay(500); // 3ms 一帧 广播
        // pTxCharacteristic->setValue(txValue, my_strlen(txValue));
        // pTxCharacteristic->notify();
        // delay(20); // bluetooth stack will go into congestion, if too many packets are sent
    }
    while (Serial.available() > 0)
    {
        if (deviceConnected)
        {
            // vTaskDelay(500); // 3ms 一帧 广播
            // readString += Serial.read();
            // pTxCharacteristic->setValue(chipId.c_str()); //设备 UDID
            // //      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
            // pTxCharacteristic->notify(); //广播设备信息
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(510);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}







int my_strlen(unsigned char chr[])
{
    int i = 0 ;
    for(  i = 0 ; chr[i] != '\0'; ++i);
    return i>19? 19 : i ;
}

void SendBack(String s)
{
    pTxCharacteristic->setValue(s.c_str()); //发送设备 UDID
    pTxCharacteristic->notify();
}

void DHT11_UPdate()
{
     unsigned char  H = Dht11.readHumidity();
     unsigned char T = Dht11.readTemperature();
     st[0] = '*';
     st[1] = H;
     st[2] = T;
     st[3] = '0';

     pTxCharacteristic->setValue(st, 4); //发送设备 UDID
     pTxCharacteristic->notify();

}