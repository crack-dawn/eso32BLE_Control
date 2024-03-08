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
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <common.h>


String chipId;
char txValue[20] = "123456";
BLEServer *pServer = NULL;                   // BLEServer指针 pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristic指针 pTxCharacteristic
bool deviceConnected = false;                //本次连接状态
bool oldDeviceConnected = false;             //上次连接状态

// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // UART service UUID  服务码
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" // 接收特征码
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //发送特征码
// 96019fc6-1385-4934-b806-e8a155aa6068    UUID
// 76608736-5057-4838-9161-d664af4e4e3d     RX
// b3f42935-4864-4b93-a6a4-956e6ea20873     TX

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
        { //向串口输出收到的值
            Serial.println("-----------------------");
            Serial.print("RX: ");
            for (int i = 0; i < rxValue.length(); i++)
                Serial.print(rxValue[i]);
            Serial.println();
            Serial.println(chipId.c_str());

            if (rxValue == "getid")
            {
                Serial.println(chipId.c_str());
            }
            else if (rxValue == "on")
            {
                Serial.println("open");
                digitalWrite(2, 1);
            }
            else if (rxValue == "off")
            {
                Serial.println("close");
                digitalWrite(2, 0);
            }
            Serial.println("-----------------------|");
        }
    }
};

void setup()
{
    //串口初始化
    Serial.begin(115200);

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //获取ESP开发板唯一的MAC地址
    chipId.toUpperCase();                              //字符串全部转大写

    Serial.println(chipId.c_str()); //打印chipId信息  UDID

    pinMode(2, OUTPUT);

    delay(2002);

    // 创建一个 BLE 设备
    BLEDevice::init("UART Service");

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
        //        pTxCharacteristic->setValue(&txValue, 1);
        //        pTxCharacteristic->notify();
        //        txValue++;
        //    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
    while (Serial.available() > 0)
    {
        if (deviceConnected)
        {
            delay(3); // 3ms 一帧数据
            readString += Serial.read();
            pTxCharacteristic->setValue(chipId.c_str()); //发送设备 UDID
            //      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
            pTxCharacteristic->notify();
            Serial.println(chipId);
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
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