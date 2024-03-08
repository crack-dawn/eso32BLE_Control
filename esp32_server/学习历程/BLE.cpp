// // ΢������С����
// /*
//     Video: https://www.youtube.com/watch?v=oCMOYS71NIU
//     Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
//     Ported to Arduino ESP32 by Evandro Copercini
//    Create a BLE server that, once we receive a connection, will send periodic notifications.
//    ����һ��BLE��������һ�������յ����ӣ����������Է���֪ͨ0
//    Tʹ�ò��裺
//    1. ����һ�� BLE Server
//    2. ����һ�� BLE Service
//    3. ����һ�� BLE Characteristic
//    4. ����һ�� BLE Descriptor
//    5. ��ʼ����
//    6. ��ʼ�㲥
// */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <common.h>


String chipId;
char txValue[20] = "123456";
BLEServer *pServer = NULL;                   // BLEServerָ�� pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristicָ�� pTxCharacteristic
bool deviceConnected = false;                //��������״̬
bool oldDeviceConnected = false;             //�ϴ�����״̬

// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // UART service UUID  ������
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" // ����������
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //����������
// 96019fc6-1385-4934-b806-e8a155aa6068    UUID
// 76608736-5057-4838-9161-d664af4e4e3d     RX
// b3f42935-4864-4b93-a6a4-956e6ea20873     TX

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        digitalWrite(2, 1);
        Serial.println(" �Ѿ����� ");
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        digitalWrite(2, 0);
        Serial.println(" �Ͽ����� ");
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue(); //������Ϣ

        if (rxValue.length() > 0)
        { //�򴮿�����յ���ֵ
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
    //���ڳ�ʼ��
    Serial.begin(115200);

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //��ȡESP������Ψһ��MAC��ַ
    chipId.toUpperCase();                              //�ַ���ȫ��ת��д

    Serial.println(chipId.c_str()); //��ӡchipId��Ϣ  UDID

    pinMode(2, OUTPUT);

    delay(2002);

    // ����һ�� BLE �豸
    BLEDevice::init("UART Service");

    // ����һ�� BLE ����
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //���ûص�

    BLEService *pService = pServer->createService(SERVICE_UUID);

    // ����һ�� BLE ����
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY); //���÷���������
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE); //��������������
    pRxCharacteristic->setCallbacks(new MyCallbacks());                                                                               //���ûص�

    pService->start();                  // ��ʼ����
    pServer->getAdvertising()->start(); // ��ʼ�㲥
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
            delay(3); // 3ms һ֡����
            readString += Serial.read();
            pTxCharacteristic->setValue(chipId.c_str()); //�����豸 UDID
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