#include "HX711.h"
#include "Adafruit_Fingerprint.h"
#include <SoftwareSerial.h>

// 引脚定义
uint8_t dataPin = 6;
uint8_t clockPin = 7;
SoftwareSerial mySerial(2, 3);  // 软串口，用于连接指纹模块
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
HX711 scale;

// 全局变量
uint8_t id;
volatile float weight;
bool enrolling = false;

void setup() {
  Serial.begin(115200);   // 与PC通信
  mySerial.begin(57600);  // 指纹模块通信
  scale.begin(dataPin, clockPin); // 初始化HX711
  
  Serial.println("Fingerprint and Weight Reading System");

  // 初始化指纹模块
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor not found.");
    while (1) { delay(1); }
  }
  
  scale.set_scale(127.15); // HX711 校准因子，需自行校准
  scale.tare(); // 将当前值设为0

  Serial.println("System ready.");
}

void loop() {
  // 监听串口命令
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // 清除多余的换行符

    if (command == "read_weight") {
      read_weight();
    } else if (command.startsWith("fp_enroll ")) {
      id = command.substring(10).toInt();
      enroll_fingerprint(id);
    } else if (command.startsWith("fp_delete ")) {
      id = command.substring(10).toInt();
      delete_fingerprint(id);
    } else if (command == "fp_detect") {
      detect_fingerprint();
    }
  }
}

void read_weight() {
  weight = scale.read_median(7);  // 读取重量
  Serial.print("Weight: ");
  Serial.print(weight, 2);
  Serial.println(" g");
}

void enroll_fingerprint(uint8_t id) {
  if (!enrolling) {
    enrolling = true;
    Serial.print("Enrolling ID #");
    Serial.println(id);
    if (getFingerprintEnroll(id) == FINGERPRINT_OK) {
      enrolling = false;
    }
  }
}

uint8_t getFingerprintEnroll(uint8_t id) {
  // 指纹注册逻辑，略，保持与之前相同
  // ...
  return FINGERPRINT_OK;  // 如果成功注册，返回OK
}

void delete_fingerprint(uint8_t id) {
  uint8_t p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint deleted!");
  } else {
    Serial.println("Failed to delete fingerprint.");
  }
}

void detect_fingerprint() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK) {
      p = finger.fingerSearch();
      if (p == FINGERPRINT_OK) {
        Serial.print("Fingerprint ID: ");
        Serial.println(finger.fingerID);
        read_weight();  // 检测到指纹后读取重量
      } else {
        Serial.println("Fingerprint not found.");
      }
    }
  }
}
