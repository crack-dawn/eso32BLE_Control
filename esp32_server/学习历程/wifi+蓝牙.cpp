#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <common.h>

#include <WiFi.h>
#include "time.h"

#include "soc/rtc_wdt.h" //设置看门狗用

/*
//ESP32看门狗设置 需要先引入 #include "soc/rtc_wdt.h" //设置看门狗用
  rtc_wdt_protect_off();     //看门狗写保护关闭 关闭后可以喂狗
  //rtc_wdt_protect_on();    //看门狗写保护打开 打开后不能喂狗
  //rtc_wdt_disable();       //禁用看门狗
  rtc_wdt_enable();          //启用看门狗

  rtc_wdt_set_time(RTC_WDT_STAGE0, 7000);     // 设置看门狗超时 7000ms.
 //如果解决了您的问题与困扰 支持原创 给个赞或者打个赏，您的支持是我们进步的动力
————————————————
版权声明：本文为CSDN博主「锋仔2」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/jianfengbeyond/article/details/123979350
*/

/*WiFi 变量区*/
const char *ssid = "233";
const char *password = "12345678";

const char *ntpServer = "us.pool.ntp.org";
const long gmtOffset_sec = 3600 * 7;
const int daylightOffset_sec = 3600;

/*WiFi函数*/
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
/*BLE 变量区*/
String chipId;
String readString;
char txValue[20] = "123456";
BLEServer *pServer = NULL;                   // BLEServer指针 pServer
BLECharacteristic *pTxCharacteristic = NULL; // BLECharacteristic指针 pTxCharacteristic
bool deviceConnected = false;                //本次连接状态
bool oldDeviceConnected = false;             //上次连接状态

/*BLE 函数*/
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
#define SERVICE_UUID "96019fa1-1385-4934-b806-e8a155aa6068"           // UART service UUID  服务码
#define CHARACTERISTIC_UUID_RX "76608fa2-5057-4838-9161-d664af4e4e3d" // 接收特征码
#define CHARACTERISTIC_UUID_TX "b3f42fa3-4864-4b93-a6a4-956e6ea20873" //发送特征码

/*freeRTos 动态创建任务 */
static TaskHandle_t xTaskSetup_Handle = NULL;
static TaskHandle_t xTaskBLE_Handle = NULL;
static TaskHandle_t xTaskWiFi_Handle = NULL;

static void xTaskSetup(void *pvparameters);
static void xTaskBLE(void *pvparameters);
static void xTaskWiFi(void *pvparameters);

static void xTaskBLE(void *pvparameters)
{

    chipId = String((uint32_t)ESP.getEfuseMac(), HEX); //获取ESP开发板唯一的MAC地址
    chipId.toUpperCase();                              //字符串全部转大写
    Serial.println(chipId.c_str());                    //打印chipId信息  UDID

    vTaskDelay(3);

    pinMode(2, OUTPUT);

    vTaskDelay(2);

    // 创建一个 BLE 设备
    BLEDevice::init("BLE_APP");

    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //设置回调

    BLEService *pService = pServer->createService(SERVICE_UUID);

    //创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY); //设置发信特征码
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE); //设置收信特征码
    pRxCharacteristic->setCallbacks(new MyCallbacks());                                                                               //设置回调
    Serial.println(" 1.蓝牙服务器创建完成 ");

    pService->start();                  // 开始服务
    pServer->getAdvertising()->start(); // 开始广播

    Serial.println(" 2.蓝牙广播开启 ");
    Serial.println(" 3.1蓝牙广播中 ");

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
                vTaskDelay(3); // 3ms 一帧 广播
                readString += Serial.read();
                pTxCharacteristic->setValue(chipId.c_str()); //设备 UDID
                //      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
                pTxCharacteristic->notify(); //广播设备信息
            }
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected)
        {

            Serial.println(" 3.2蓝牙重新广播 ");
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

    Serial.println("# WiFi已经链接");
    vTaskDelay(1000);

    
    // init and get the time
    Serial.println("# WiFi校时");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("# WiFi校时完成");
    printLocalTime();

    // disconnect WiFi as it's no longer needed
    vTaskDelay(1000);
    Serial.println("# WiFi断开");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    while (1)
    {
        Serial.println("# WiFi循环中");

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
        Serial.println("任务一 蓝牙setup");
    }
    xReturn = xTaskCreate(xTaskWiFi,
                          "xTaskWiFi",
                          4096,
                          NULL,
                          1,
                          (TaskHandle_t *)&xTaskWiFi_Handle);

    if (pdPASS == xReturn)
    {
        Serial.println("任务二 WiFi setup");
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
        Serial.println("任务一 蓝牙setup");
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
        Serial.println("任务二 WiFi setup");
    }
#endif

    vTaskDelete(xTaskSetup_Handle); //删除 启动任务

    portEXIT_CRITICAL(&mux);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("串口初始化成功");
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
        Serial.println("起始任务 准备启动，创建子任务 BLE 和 WiFi---------");
    }

    vTaskStartScheduler(); /*启动任务调度器， 创建的 多线程任务开始运行 从启动任务开始运行，*/

    while (1)
    {
    }
}

