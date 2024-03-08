#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <common.h>

#include <WiFi.h>
#include "time.h"

#include "soc/rtc_wdt.h" //���ÿ��Ź���

/*
//ESP32���Ź����� ��Ҫ������ #include "soc/rtc_wdt.h" //���ÿ��Ź���
  rtc_wdt_protect_off();     //���Ź�д�����ر� �رպ����ι��
  //rtc_wdt_protect_on();    //���Ź�д������ �򿪺���ι��
  //rtc_wdt_disable();       //���ÿ��Ź�
  rtc_wdt_enable();          //���ÿ��Ź�

  rtc_wdt_set_time(RTC_WDT_STAGE0, 7000);     // ���ÿ��Ź���ʱ 7000ms.
 //���������������������� ֧��ԭ�� �����޻��ߴ���ͣ�����֧�������ǽ����Ķ���
��������������������������������
��Ȩ����������ΪCSDN����������2����ԭ�����£���ѭCC 4.0 BY-SA��ȨЭ�飬ת���븽��ԭ�ĳ������Ӽ���������
ԭ�����ӣ�https://blog.csdn.net/jianfengbeyond/article/details/123979350
*/

/*WiFi ������*/
const char *ssid = "233";
const char *password = "12345678";

const char *ntpServer = "us.pool.ntp.org";
const long gmtOffset_sec = 3600 * 7;
const int daylightOffset_sec = 3600;

/*WiFi����*/
void printLocalTime()
{
    static struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        // return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

#define USE_MULTCORE 1
/*BLE ������*/
String chipId;
String readString;
char txValue[20] = "123456";
BLEServer *pServer = NULL;                   // BLEServerָ�� pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristicָ�� pTxCharacteristic
bool deviceConnected = false;                //��������״̬
bool oldDeviceConnected = false;             //�ϴ�����״̬

/*BLE ����*/
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
            Serial.println("----------");
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
            Serial.println("-----------|");
        }
    }
};

// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // UART service UUID  ������
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" // ����������
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //����������

/*freeRTos ��̬�������� */
static TaskHandle_t xTaskSetup_Handle = NULL;
static TaskHandle_t xTaskBLE_Handle = NULL;
static TaskHandle_t xTaskWiFi_Handle = NULL;

static void xTaskSetup(void *pvparameters);
static void xTaskBLE(void *pvparameters);
static void xTaskWiFi(void *pvparameters);

static void xTaskBLE(void *pvparameters)
{

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //��ȡESP������Ψһ��MAC��ַ
    chipId.toUpperCase();                              //�ַ���ȫ��ת��д
    Serial.println(chipId.c_str());                    //��ӡchipId��Ϣ  UDID

    vTaskDelay(3);

    pinMode(2, OUTPUT);

    vTaskDelay(2);

    // ����һ�� BLE �豸
    BLEDevice::init("BLE_APP");

    // ����һ�� BLE ����
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //���ûص�

    BLEService *pService = pServer->createService(SERVICE_UUID);

    //����һ�� BLE ����
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY); //���÷���������
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE); //��������������
    pRxCharacteristic->setCallbacks(new MyCallbacks());                                                                               //���ûص�
    Serial.println(" 1.����������������� ");

    pService->start();                  // ��ʼ����
    pServer->getAdvertising()->start(); // ��ʼ�㲥

    Serial.println(" 2.�����㲥���� ");
    Serial.println(" 3.1�����㲥�� ");

    while (1)
    {

        if (deviceConnected)
        {
            // pTxCharacteristic->setValue(txValue);
            // pTxCharacteristic->notify();

            vTaskDelay(200); // bluetooth stack will go into congestion, if too many packets are sent
        }
        while (Serial.available() > 0)
        {
            if (deviceConnected)
            {
                vTaskDelay(3); // 3ms һ֡ �㲥
                readString += Serial.read();
                pTxCharacteristic->setValue(chipId.c_str()); //�豸 UDID
                //      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
                pTxCharacteristic->notify(); //�㲥�豸��Ϣ
            }
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected)
        {

            Serial.println(" 3.2�������¹㲥 ");
            vTaskDelay(20);              // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected)
        {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }
    }
}
static void xTaskWiFi(void *pvparameters)
{

    // connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        Serial.print(".");
    }

    Serial.println("# WiFi�Ѿ�����");
    vTaskDelay(1000);

    
    // init and get the time
    Serial.println("# WiFiУʱ");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("# WiFiУʱ���");
    printLocalTime();

    // disconnect WiFi as it's no longer needed
    vTaskDelay(1000);
    Serial.println("# WiFi�Ͽ�");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    while (1)
    {
        Serial.println("# WiFiѭ����");

        Serial.println("WiFi");
        vTaskDelay(1000);
        printLocalTime();
    }
}

static void xTaskSetup(void *pvparameters)
{

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    BaseType_t xReturn = pdPASS;

    taskENTER_CRITICAL(&mux);

#if (USE_MULTCORE == 0)
    xReturn = xTaskCreate(xTaskBLE,
                          "xTaskBLE",
                          4096,
                          NULL,
                          2,
                          (TaskHandle_t *)&xTaskBLE_Handle);

    if (pdPASS == xReturn)
    {
        Serial.println("����һ ����setup");
    }
    xReturn = xTaskCreate(xTaskWiFi,
                          "xTaskWiFi",
                          4096,
                          NULL,
                          1,
                          (TaskHandle_t *)&xTaskWiFi_Handle);

    if (pdPASS == xReturn)
    {
        Serial.println("����� WiFi setup");
    }
#else

    xReturn = xTaskCreatePinnedToCore(xTaskBLE,
                                      "xTaskBLE",
                                      2048,
                                      NULL,
                                      1,
                                      (TaskHandle_t *)&xTaskBLE_Handle,
                                      0);

    if (pdPASS == xReturn)
    {
        Serial.println("����һ ����setup");
    }

    xReturn = xTaskCreatePinnedToCore(xTaskWiFi,
                                      "xTaskWiFi",
                                      2048,
                                      NULL,
                                      1,
                                      (TaskHandle_t *)&xTaskWiFi_Handle,
                                      1);

    if (pdPASS == xReturn)
    {
        Serial.println("����� WiFi setup");
    }
#endif

    vTaskDelete(xTaskSetup_Handle); //ɾ�� ��������

    portEXIT_CRITICAL(&mux);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("���ڳ�ʼ���ɹ�");
}

void loop()
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(xTaskSetup,
                          "xTaskSetup",
                          1024,
                          NULL,
                          1,
                          (TaskHandle_t *)&xTaskSetup_Handle);

    if (pdPASS == xReturn)
    {
        Serial.println("��ʼ���� ׼������������������ BLE �� WiFi---------");
    }

    vTaskStartScheduler(); /*��������������� ������ ���߳�����ʼ���� ����������ʼ���У�*/

    while (1)
    {
    }
}

