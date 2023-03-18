#include <WiFi.h>

#define host 0

const char *ssid = "ESP32_wifi";
const char *password = "12345678";
const int port = 1234;
const int LED = D6;

struct imuData {
    int index;
    int ax;
    int ay;
    int az;
    int gx;
    int gy;
    int gz;
} imuData;

#if host == 1

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

int len = 0;

WiFiServer server;  // 声明服务器对象

WiFiClient clients[4];  // 创建4个对象
int clientNum = 0;      // 客户端计数

int flag;
void setup() {
    Serial.begin(115200);  // 启动串口通讯
    delay(3000);
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
    WiFi.mode(WIFI_AP);  // 设置为接入点模式
    Serial.printf("\r\nSet AP : %s , password : %s\r\n", ssid, password);
    WiFi.softAPConfig(local_IP, gateway, subnet);  // 配置接入点的IP，网关IP，子网掩码

    // WiFi.softAP(ssid, password,3,0);//启动校验式网络（需要输入账号密码的网络）,通道为3，最大连接数=4
    flag = WiFi.softAP(ssid, password);  // 监控状态变量result
    if (flag) {
        Serial.println("Open AP success");
    } else {
        Serial.println("Open AP failed");
    }

    server.begin(port);  // 服务器启动，端口号1234
    Serial.printf("Start TCP server on %d \r\n", port);
}

void loop() {
    WiFiClient client = server.available();  // 尝试建立客户对象
    if (client) {
        Serial.println("new connect");
        while (client.connected()) {
            if (client.available()) {
                len = 0;
                digitalWrite(LED, HIGH);
                while (client.available())  // 如果有可读数据
                {
                    *((char *)&imuData + len) = client.read();  // 读取一个字节
                    len++;
                    if (len >= sizeof(imuData))  // 长度超出
                    {
                        len = 0;
                        // Serial.printf("%d\r\n",imuData.index);
                        Serial.printf("recv: index %d,ax %d,ay %d, az %d, gx %d, gy %d, gz %d.\r\n", imuData.index, imuData.ax, imuData.ay, imuData.az, imuData.gx, imuData.gy, imuData.gz);  // 从串口打印
                    }
                }
                digitalWrite(LED, LOW);
            }
        }

        client.stop();  // 结束当前连接
        Serial.println("disconnect");
    }
}

#else

const char *ip = "192.168.4.1";

WiFiClient client;

uint8_t data[28];

int count = 0;

void setup() {
    Serial.begin(115200);
    delay(3000);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    Serial.print("\r\nconnect wifi ");

    imuData.index = 0;
    imuData.ax = 1;
    imuData.ay = 2;
    imuData.az = 3;
    imuData.gx = -1;
    imuData.gy = -2;
    imuData.gz = -3;
}

void loop() {
    WiFi.begin(ssid, password);            // 连接网络
    while (WiFi.status() != WL_CONNECTED)  // 等待网络连接成功
    {
        count++;
        delay(500);
        Serial.print(".");
        if (count >= 20) {
            // 重启
            Serial.println("to many try,restart.");
            delay(1000);
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.println("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());  // 打印模块IP

    Serial.println("connect server!");
    if (client.connect(ip, port))  // 尝试访问目标地址
    {
        Serial.println("connect success");
        // client.print("Hello world!");                    //向服务器发送数据
        while (client.connected() || client.available())  // 如果已连接或有收到的未读取的数据
        {
            digitalWrite(LED, HIGH);
            client.write((char *)&imuData, sizeof(imuData));
            Serial.println("send data");
            imuData.index++;
            digitalWrite(LED, LOW);
            delay(10);
                }
        Serial.println("disconnect");
        client.stop();  // 关闭客户端
    } else {
        Serial.println("connect error");
        client.stop();  // 关闭客户端
    }
    delay(3000);
}

#endif
