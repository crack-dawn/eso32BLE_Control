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

//��������
void SendBack(String s) ;
int my_strlen(unsigned char chr[]);
String chipId;
unsigned char txValue[20] = "hello-BLE";
BLEServer *pServer = NULL;                   // BLEServerָ�� pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristicָ�� pTxCharacteristic
bool deviceConnected = false;                //��������״̬
bool oldDeviceConnected = false;             //�ϴ�����״̬

// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // service UUID����������
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" //  ����������  RX
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //  ����������   TX


/*dht11����*/
#define DHT11_data_pin 5
#define DHT_TYPE_MY DHT11
DHT Dht11 = DHT(DHT11_data_pin, DHT_TYPE_MY);
void DHT11_UPdate();
unsigned char  st[4];
/*��ʱ������*/
// �����жϺ��������ж�Ӧ���ص�IRAM�У����޷���ֵ��
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
    //��ʱ�����ͺ�ѡ�� Ԥ��Ƶ����Ƶ��80MHz�� ��ʱ�����¼�����true����
    timerAlarmWrite(Timer, 40, true);
    // ��ʼ����Ϻ򣬽���ʱ�����ӵ��жϣ� ��ʱ����ַָ�� �жϴ����� �жϱ��ش�������
    timerAttachInterrupt(Timer, &onTimer, true);
    timerSetCountUp(Timer, true);
    // ��ʱ�� �����Ķ�ʱ�� ��ʱʱ�� ��ֵ�Ƿ����ء����ڶ�ʱ����
    
    // ��ʼ������ ������һ����ʱ����
    timerAlarmEnable(Timer);
}


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
    //���ڳ�ʼ��
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    Sg90Init();

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //��ȡESP������Ψһ��MAC��ַ
    chipId.toUpperCase();                              //�ַ���ȫ��ת��д
    Serial.println(chipId.c_str()); //��ӡchipId��Ϣ  UDID

   
    delay(2002);
    
    // ����һ�� BLE �豸
    BLEDevice::init("BLE_test");
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
        DHT11_UPdate();
        vTaskDelay(500); // 3ms һ֡ �㲥
        // pTxCharacteristic->setValue(txValue, my_strlen(txValue));
        // pTxCharacteristic->notify();
        // delay(20); // bluetooth stack will go into congestion, if too many packets are sent
    }
    while (Serial.available() > 0)
    {
        if (deviceConnected)
        {
            // vTaskDelay(500); // 3ms һ֡ �㲥
            // readString += Serial.read();
            // pTxCharacteristic->setValue(chipId.c_str()); //�豸 UDID
            // //      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
            // pTxCharacteristic->notify(); //�㲥�豸��Ϣ
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
    pTxCharacteristic->setValue(s.c_str()); //�����豸 UDID
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

     pTxCharacteristic->setValue(st, 4); //�����豸 UDID
     pTxCharacteristic->notify();

}