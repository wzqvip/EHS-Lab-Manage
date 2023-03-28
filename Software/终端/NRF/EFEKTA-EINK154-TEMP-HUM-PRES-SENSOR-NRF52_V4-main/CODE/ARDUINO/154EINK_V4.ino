// ###################          Mini weather station with electronic ink display 1.54 Inch | nRF52           ############### //
//                                                                                                                          //
//        @filename   :   EFEKTA_THEINK154NEW.ino                                                                            //
//        @brief en   :   Wireless, battery-operated temperature, humidity, pressure sensor (bme280)                        //
//                        with electronic ink display(GDEH029A1 DALIAN GOOD DISPLAY CO., LTD.). Works on nRF52.             //
//        @brief ru   :   Беcпроводной, батарейный датчик температуры, влажности и давления (bme280)                        //
//                        с дисплеем на электронных чернилах(GDEH029A1 DALIAN GOOD DISPLAY CO., LTD.). Работает на nRF52.   //
//        @author     :   Andrew Lamchenko aka Berk                                                                         //
//                                                                                                                          //
//        Copyright (C) EFEKTALAB 2020                                                                                      //
//        Copyright (c) Arduino LLC.  All right reserved.                                                                   //
//        Copyright (c) Arduino Srl.  All right reserved.                                                                   //
//        Copyright (c) Sensnology AB. All right reserved.                                                                  //
//        Copyright (C) Waveshare. All right reserved.                                                                      //
//                                                                                                                          //
// ######################################################################################################################## //

#define LANG_EN
#define MY_RESET_REASON_TEXT
//#define MY_DEBUG
#define DCPOWER
//#define BME280
//#define MY_PASSIVE_NODE
//#define MY_NODE_ID 14
//#define MY_NRF5_ESB_MODE (NRF5_1MBPS)
#define MY_NRF5_ESB_MODE (NRF5_250KBPS)
#ifndef MY_DEBUG
#define MY_DISABLED_SERIAL
#endif
#define SN "EFEKTA mWS 154"
#define SV "0.21"
#define MY_RADIO_NRF5_ESB
#define MY_NRF5_ESB_PA_LEVEL (0x8UL)
#define ESPECIALLY


bool error_bosch;
bool error_sensor;
bool updateink1;
bool updateink2;
bool updateink3;
bool updateink4;
bool updateinkclear;
bool change;
bool check;
bool tch;
bool hch;
bool bch;
bool pch;
bool fch;
bool configMode;
bool button_flag;
bool nosleep;
bool flag_update_transport_param;
bool flag_sendRoute_parent;
bool flag_no_present;
bool flag_nogateway_mode;
bool flag_find_parent_process;
bool Ack_FP;

uint8_t cpNom;
uint8_t cpCount;
uint8_t timeSend = 1;
uint8_t battSend;
uint8_t battery;
uint8_t old_battery;
uint8_t err_delivery_beat;

byte weekdayPrint;

const float tempThreshold = 0.5;
const float humThreshold = 2.0;
const float pressThreshold = 1.0;
float temperatureSend;
float humiditySend;
float pressureSend;
int temperatureInt;
int humidityInt;
int pressureInt;
float old_temperature;
float old_humidity;
float old_pressure;
float batteryVoltageF;
int16_t linkq;
int16_t old_linkq;

int16_t myid;
int16_t mypar;
int16_t old_mypar = -1;

const uint16_t minuteT = 60000;
uint16_t BATT_TIME;
uint16_t BATT_COUNT;
uint16_t batteryVoltage;

uint32_t configMillis;
uint32_t previousMillis;
uint32_t SLEEP_TIME;
const uint32_t shortWait = 20;
uint32_t stopTimer;
uint32_t startTimer;
uint32_t PRECISION_TIME_WDT;
uint32_t PRECISION_TIME;
const uint32_t SLEEP_TIME_WDT = 10000;
uint32_t sleepTimeCount;

bool opposite_colorPrint  = false;
bool colorPrint = true;

#include "epaper154_V2.h"
#include "epaperpaint.h"
#include "imagedata.h"
unsigned char image_temp[5200];
Paint paith(image_temp, 0, 0);
Epd epd;


int16_t mtwr;
#define MY_TRANSPORT_WAIT_READY_MS (mtwr)
#include <MySensors.h>
#define TEMP_CHILD_ID 0
#define HUM_CHILD_ID 1
#define BARO_CHILD_ID 2
#define FORECAST_CHILD_ID 3
#define SIGNAL_Q_ID 100
#define BATTERY_VOLTAGE_ID 101
#define SET_BATT_SEND_ID 103
#define MY_SEND_RESET_REASON 105
#define SET_COLOR_ID 106
MyMessage msgTemp(TEMP_CHILD_ID, V_TEMP);
MyMessage msgHum(HUM_CHILD_ID, V_HUM);
MyMessage msgPres(BARO_CHILD_ID, V_PRESSURE);
MyMessage forecastMsg(FORECAST_CHILD_ID, V_VAR1);
MyMessage sqMsg(SIGNAL_Q_ID, V_VAR1);
MyMessage bvMsg(BATTERY_VOLTAGE_ID, V_VAR1);
MyMessage setBattSendMsg(SET_BATT_SEND_ID, V_VAR1);
MyMessage sendMsg(MY_SEND_RESET_REASON, V_VAR1);
MyMessage setColor(SET_COLOR_ID, V_VAR1);


// ##############################################################################################################
// #                                                 FORECAST                                                   #
// ##############################################################################################################
#define CONVERSION_FACTOR (1.0/10.0)         // used by forecast algorithm to convert from Pa to kPa, by dividing hPa by 10.
const char *weather[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };
enum FORECAST
{
  STABLE = 0,                     // "Stable Weather Pattern"
  SUNNY = 1,                      // "Slowly rising Good Weather", "Clear/Sunny "
  CLOUDY = 2,                     // "Slowly falling L-Pressure ", "Cloudy/Rain "
  UNSTABLE = 3,                   // "Quickly rising H-Press",     "Not Stable"
  THUNDERSTORM = 4,               // "Quickly falling L-Press",    "Thunderstorm"
  UNKNOWN = 5                     // "Unknown (More Time needed)
};
int16_t forecast;
int16_t  old_forecast = -1;            // Stores the previous forecast, so it can be compared with a new forecast.
const int LAST_SAMPLES_COUNT = 5;
float lastPressureSamples[LAST_SAMPLES_COUNT];
int minuteCount = 0;              // Helps the forecst algorithm keep time.
bool firstRound = true;           // Helps the forecast algorithm recognise if the sensor has just been powered up.
float pressureAvg;                // Average value is used in forecast algorithm.
float pressureAvg2;               // Average after 2 hours is used as reference value for the next iteration.
float dP_dt;                      // Pressure delta over time

// ##############################################################################################################
// #                          unscheduled update, global semiconductor crisis dedicated                         #
// ##############################################################################################################
#ifdef BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bosch;
#else
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bosch;
#include "Adafruit_HTU21DF.h"
Adafruit_HTU21DF sensor = Adafruit_HTU21DF();
#endif
#define SEALEVELPRESSURE_HPA (1013.25)



// ##############################################################################################################
// #                                                 INTERRUPT                                                  #
// ##############################################################################################################
uint32_t PIN_BUTTON_MASK;
volatile byte buttIntStatus = 0;
#define APP_GPIOTE_MAX_USERS 1
extern "C" {
#include "app_gpiote.h"
#include "nrf_gpio.h"
}
static app_gpiote_user_id_t m_gpiote_user_id;


void preHwInit() {
  pinMode(PIN_BUTTON, INPUT);
}



void before()
{
  //########################################## CONFIG MCU ###############################################

#ifdef DCPOWER
  NRF_POWER->DCDCEN = 1;
#endif

  NRF_SAADC ->ENABLE = 0;
  NRF_PWM0  ->ENABLE = 0;
  NRF_TWIM0 ->ENABLE = 0;
  NRF_TWIS0 ->ENABLE = 0;


  NRF_NFCT->TASKS_DISABLE = 1;
  NRF_NVMC->CONFIG = 1;
  NRF_UICR->NFCPINS = 0;
  NRF_NVMC->CONFIG = 0;
  NRF_PWM1  ->ENABLE = 0;
  NRF_PWM2  ->ENABLE = 0;
  NRF_TWIM1 ->ENABLE = 0;
  NRF_TWIS1 ->ENABLE = 0;

  NRF_RADIO->TXPOWER = 0x4UL;

#ifndef MY_DEBUG
  NRF_UART0->ENABLE = 0;
#endif


  //########################################## INIT HAPPY ##############################################

  happy_init();

  //########################################## CONFIG PROG ###############################################


  battSend = loadState(103);
  if (battSend > 24) {
    battSend = 5;
    saveState(103, battSend);
  }
  if (battSend <= 0) {
    battSend = 5;
    saveState(103, battSend);
  }
  //battSend = 1; // для теста, 1 час

  if (loadState(106) > 1) {
    saveState(106, 0);
  }
  colorChange(loadState(106));
  //colorChange(false); // для теста, true - белый шрифт, черный фон или false - четный шрифт, белый фон

  timeConf();

  //########################################## EINK INIT ###############################################
  epd.Init();
  epd.Clear2();
  paith.SetWidth(200);
  paith.SetHeight(200);
  paith.SetRotate(ROTATE_0);
  paith.Clear(colorPrint);
  DrawImageWH(&paith, 40, 30, LOGO, 116, 140, opposite_colorPrint);
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
  hwSleep(2000);
#ifdef ESPECIALLY
  epd.Clear4();
  epd.Clear4();
  paith.Clear(colorPrint);
  DrawImageWH(&paith, 0, 0, ESPECIALLY_LOGO, 200, 200, opposite_colorPrint);
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
  hwSleep(7000);
#endif
  epd.Clear4();
  epd.Clear4();
  paith.Clear(colorPrint);
#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, REGEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, REG, 180, 82, opposite_colorPrint);
#endif
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
  epd.Sleep();

  wdt_init();
}

void presentation()
{
  check = sendSketchInfo(SN, SV);
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = sendSketchInfo(SN, SV);
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(TEMP_CHILD_ID, S_TEMP, "Temperature");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(TEMP_CHILD_ID, S_TEMP, "Temperature");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(HUM_CHILD_ID, S_HUM, "Humidity");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(HUM_CHILD_ID, S_HUM, "Humidity");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(BARO_CHILD_ID, S_BARO, "Pressure");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(BARO_CHILD_ID, S_BARO, "Pressure");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(FORECAST_CHILD_ID, S_CUSTOM, "Forecast");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(FORECAST_CHILD_ID, S_CUSTOM, "Forecast");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(SIGNAL_Q_ID, S_CUSTOM, "SIGNAL %");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(SIGNAL_Q_ID, S_CUSTOM, "SIGNAL %");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(BATTERY_VOLTAGE_ID, S_CUSTOM, "BATTERY VOLTAGE");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(BATTERY_VOLTAGE_ID, S_CUSTOM, "BATTERY VOLTAGE");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(SET_BATT_SEND_ID, S_CUSTOM, "BATT SEND INTERTVAL | H");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(SET_BATT_SEND_ID, S_CUSTOM, "BATT SEND INTERTVAL | H");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(MY_SEND_RESET_REASON, S_CUSTOM, "RESTART REASON");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(MY_SEND_RESET_REASON, S_CUSTOM, "RESTART REASON");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  check = present(SET_COLOR_ID, S_CUSTOM, "COLOR W/B");
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = present(SET_COLOR_ID, S_CUSTOM, "COLOR W/B");
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }
  wait(shortWait * 2);
  sendConfig();
  wait(shortWait);
}



void setup() {
  config_Happy_node();

  if (flag_nogateway_mode == false) {
    epd.Reset();
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, REGAEN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, REGA, 180, 82, opposite_colorPrint);
#endif
    epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
    epd.SendCommand(0x22);
    epd.SendData(0xFF);
    epd.SendCommand(0x20);
    epd.WaitUntilIdle();
    epd.Sleep();
    sendResetReason();
  } else {

    epd.Reset();
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, REGA2EN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, REGA2, 180, 82, opposite_colorPrint);
#endif
    epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
    epd.SendCommand(0x22);
    epd.SendData(0xFF);
    epd.SendCommand(0x20);
    epd.WaitUntilIdle();
    epd.Sleep();

  }

  transportDisable();

  interrupt_Init();

  sleepTimeCount = SLEEP_TIME;

  SensorInit();

  readBatt();
}


void loop() {
  if (flag_update_transport_param == true) {
    update_Happy_transport();
  }
  if (flag_sendRoute_parent == true) {
    present_only_parent();
  }
  if (isTransportReady() == true) {
    if (flag_find_parent_process == true) {
      find_parent_process();
    }
    if (flag_nogateway_mode == false) {
      if (configMode == false) {
        if (buttIntStatus == PIN_BUTTON) {
          if (digitalRead(PIN_BUTTON) == LOW && button_flag == false) {
            button_flag = true;
            previousMillis = millis();
          }
          if (digitalRead(PIN_BUTTON) == LOW && button_flag == true) {
            if ((millis() - previousMillis > 0) && (millis() - previousMillis <= 4500)) {
              if (updateink1 == false) {
                einkZeropush();
                updateink1 = true;
              }
            }
            if ((millis() - previousMillis > 4500) && (millis() - previousMillis <= 5500)) {
              if (updateinkclear == false) {
                clearOne();
                updateinkclear = true;
              }
            }
            if ((millis() - previousMillis > 5500) && (millis() - previousMillis <= 8500)) {
              if (updateink2 == false) {
                einkOnepush();
                updateink2 = true;
                updateinkclear = false;
              }
            }
            if ((millis() - previousMillis > 8500) && (millis() - previousMillis <= 9500)) {
              if (updateinkclear == false) {
                clearOne();
                updateinkclear = true;
              }
            }
            if ((millis() - previousMillis > 9500) && (millis() - previousMillis <= 12500)) {
              if (updateink3 == false) {
                einkOnePluspush();
                updateink3 = true;
                updateinkclear = false;
              }
            }
            if (millis() - previousMillis > 12500) {
              if (updateinkclear == false) {
                clearOne();
                updateinkclear = true;
              }
            }
            wdt_nrfReset();
          }
          if (digitalRead(PIN_BUTTON) == HIGH && button_flag == true) {

            if ((millis() - previousMillis <= 4500) && (button_flag == true))
            {
              einkZeroend();
              reseteinkset();
              configMode = true;
              button_flag = false;
              buttIntStatus = 0;
              transportReInitialise();
              wait(shortWait);
              NRF5_ESB_startListening();
              wait(shortWait);
              configMillis = millis();
            }
            if ((millis() - previousMillis > 5500 && millis() - previousMillis <= 8500) && button_flag == true)
            {
              einkOneend();
              reseteinkset();
              button_flag = false;
              buttIntStatus = 0;
              transportReInitialise();
              wait(shortWait);
              presentation();
              wait(shortWait);
              transportDisable();
              wait(shortWait * 10);
              change = true;
              BATT_COUNT = BATT_TIME;
              sleepTimeCount = SLEEP_TIME;
            }
            if ((millis() - previousMillis > 9500) && (millis() - previousMillis <= 12500) && (button_flag == true))
            {
              einkOnePlusend();
              new_device();
            }
            if ((((millis() - previousMillis > 4500) && (millis() - previousMillis <= 5500)) || ((millis() - previousMillis > 8500) && (millis() - previousMillis <= 9500)) || (millis() - previousMillis > 12500) ) && (button_flag == true) )
            {
              wdt_nrfReset();
              change = true;
              sleepTimeCount = SLEEP_TIME;
              reseteinkset();
              button_flag = false;
              buttIntStatus = 0;
            }
          }
          wdt_nrfReset();
        } else {
          wdt_nrfReset();
          sleepTimeCount++;
          if (sleepTimeCount >= SLEEP_TIME) {
            sleepTimeCount = 0;
            readData();
            if (change == true) {
              transportReInitialise();
              wait(shortWait);
              sendData();
              transportDisable();
              wait(shortWait);
              DisplayUpdate();
              wait(shortWait);
              change = false;
            }
          }
          nosleep = false;
        }
      } else {
        if (millis() - configMillis > 20000) {
          configMode = false;
          button_flag = false;
          buttIntStatus = 0;
          transportDisable();
          wait(shortWait * 2);
          change = true;
          sleepTimeCount = SLEEP_TIME;
        }
        wdt_nrfReset();
      }
    } else {
      if (buttIntStatus == PIN_BUTTON) {
        if (digitalRead(PIN_BUTTON) == LOW && button_flag == false) {
          button_flag = true;
          previousMillis = millis();
        }
        if (digitalRead(PIN_BUTTON) == LOW && button_flag == true) {
          if ((millis() - previousMillis > 0) && (millis() - previousMillis <= 4500)) {
            if (updateink1 == false) {
              einkZeropush();
              updateink1 = true;
            }
          }
          if ((millis() - previousMillis > 4500) && (millis() - previousMillis <= 5500)) {
            if (updateinkclear == false) {
              clearOne();
              updateinkclear = true;
            }
          }
          if ((millis() - previousMillis > 5500) && (millis() - previousMillis <= 8500)) {
            if (updateink2 == false) {
              einkOnePluspush();
              updateink2 = true;
              updateinkclear = false;
            }
          }
          if (millis() - previousMillis > 8500) {
            if (updateinkclear == false) {
              clearOne();
              updateinkclear = true;
            }
          }
          wdt_nrfReset();
        }
        if (digitalRead(PIN_BUTTON) == HIGH && button_flag == true) {
          if (millis() - previousMillis <= 4500 && button_flag == true)
          {
            wdt_nrfReset();
            einkZeroend();
            reseteinkset();
            button_flag = false;
            buttIntStatus = 0;
            transportReInitialise();
            check_parent();
            cpCount = 0;
            change = true;
            BATT_COUNT = BATT_TIME;
            sleepTimeCount = SLEEP_TIME;
          }
          if ((millis() - previousMillis > 5500) && (millis() - previousMillis <= 8500) && (button_flag == true))
          {
            einkOnePlusend();
            new_device();
          }
          if ( ( ( millis() - previousMillis > 4500 && millis() - previousMillis <= 5500 ) || ( millis() - previousMillis > 8500)) && button_flag == true)
          {
            wdt_nrfReset();
            change = true;
            sleepTimeCount = SLEEP_TIME;
            reseteinkset();
            button_flag = false;
            buttIntStatus = 0;
          }
        }
        wdt_nrfReset();
      } else {
        sleepTimeCount++;
        if (sleepTimeCount >= SLEEP_TIME) {
          sleepTimeCount = 0;
          cpCount++;
          if (cpCount >= cpNom) {
            transportReInitialise();
            check_parent();
            cpCount = 0;
          }
          readData();
          if (change == true) {
            change = false;
            if (flag_nogateway_mode == false) {
              transportReInitialise();
            }
            wait(shortWait);
            sendData();
            if (flag_nogateway_mode == false) {
              transportDisable();
            }
            wait(shortWait);
            DisplayUpdate();
            wait(shortWait);
          }
          sleepTimeCount = 0;
        }
        if (cpCount < cpNom) {
          nosleep = false;
        }
      }
    }
  }
  if (_transportSM.failureCounter > 0)
  {
    _transportConfig.parentNodeId = loadState(201);
    _transportConfig.nodeId = myid;
    _transportConfig.distanceGW = loadState(202);
    mypar = _transportConfig.parentNodeId;
    nosleep = false;
    err_delivery_beat = 6;
    happy_node_mode();
    gateway_fail();
  }

  if (nosleep == false) {
    wdt_nrfReset();
    transportDisable();

    uint32_t periodTimer;
    uint32_t quotientTimer;
    stopTimer = millis();
    if (stopTimer < startTimer) {
      periodTimer = (4294967295 - startTimer) + stopTimer;
    } else {
      periodTimer = stopTimer - startTimer;
    }
    if (periodTimer >= SLEEP_TIME_WDT) {
      quotientTimer = periodTimer / SLEEP_TIME_WDT;
      if (quotientTimer == 0) {
        PRECISION_TIME_WDT = periodTimer - SLEEP_TIME_WDT;
        sleepTimeCount++;
      } else {
        PRECISION_TIME_WDT = periodTimer - SLEEP_TIME_WDT * quotientTimer;
        sleepTimeCount = sleepTimeCount + quotientTimer;
      }
    } else {
      PRECISION_TIME_WDT = SLEEP_TIME_WDT - periodTimer;
    }
    hwSleep(PRECISION_TIME_WDT);
    startTimer = millis();
    nosleep = true;
  }
}



// ##############################################################################################################
// #                                                 BME 280 INIT                                               #
// ##############################################################################################################

// #####################################################

void SensorInit() {
#ifdef BME280
  if (! bosch.begin(&Wire)) {
    error_bosch = true;
    CORE_DEBUG(PSTR("MyS: BME280 INITIALIZATION ERROR\n"));
  }
  bosch.setSampling(Adafruit_BME280::MODE_FORCED, // operating mode
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF); // filtering
  wait(500);
#else
  if (!bosch.begin()) {
    error_bosch = true;
    CORE_DEBUG(PSTR("MyS: BMP280 INITIALIZATION ERROR\n"));
  }
  bosch.setSampling(Adafruit_BMP280::MODE_FORCED, // operating mode
                    Adafruit_BMP280::SAMPLING_NONE, // temperature
                    Adafruit_BMP280::SAMPLING_X1, // pressure
                    Adafruit_BMP280::FILTER_OFF); // filtering
  wait(500);

  if (!sensor.begin()) {
    error_sensor = true;
    CORE_DEBUG(PSTR("MyS: HTU21D INITIALIZATION ERROR\n"));
  }
#endif
}



// ##############################################################################################################
// #                                                 SENSOR READ                                                #
// ##############################################################################################################

void readData() {

#ifdef BME280
  if (error_bosch == false) {
    bosch.takeForcedMeasurement();
    wait(shortWait);
    temperatureSend = bosch.readTemperature();
    pressureSend = bosch.readPressure() / 100.0F;
    humiditySend = bosch.readHumidity();
    forecast = sample(pressureSend);
    wait(shortWait);
  } else {
    temperatureSend = -1.0;
    pressureSend = -1.0;
    humiditySend = -1.0;
    forecast = 10;
  }
#else
  if (error_bosch == false) {
    wait(shortWait);
    bosch.takeForcedMeasurement();
    pressureSend = bosch.readPressure() / 100.0F;
    forecast = sample(pressureSend);
  } else {
    pressureSend = -1.0;
    forecast = 10;
  }
  if (error_sensor == false) {
    wait(shortWait);
    temperatureSend = sensor.readTemperature();
    humiditySend = sensor.readHumidity();
    wait(shortWait);
  } else {
    temperatureSend = -1.0;
    humiditySend = -1.0;
    forecast = 11;
  }
#endif


  if ((int)humiditySend < 0) {
    humiditySend = 0.0;
  }
  if ((int)humiditySend > 99) {
    humiditySend = 99.0;
  }

  if ((int)temperatureSend < 0) {
    temperatureSend = 0.0;
  }
  if ((int)temperatureSend > 99) {
    temperatureSend = 99.9;
  }
  if ((int)pressureSend < 300) {
    pressureSend = 0.0;
  }
  if ((int)pressureSend > 1100) {
    pressureSend = 1100.0;
  }

  if ((error_bosch == false) && (error_sensor == false)) {
    if (forecast != old_forecast) {
      change = true;
      fch = true;
      if ((old_forecast != 5) && (forecast == 0)) {
        forecast = old_forecast;
        change = false;
        fch = false;
      }
      if ((old_forecast != 5) && (forecast != 0)) {
        old_forecast = forecast;
      }
      if (old_forecast == 5) {
        old_forecast = forecast;
      }
    }
  }

#ifndef LANG_EN
  pressureSend = pressureSend * 0.75006375541921;
#endif

  if (abs(temperatureSend - old_temperature) >= tempThreshold) {
    old_temperature = temperatureSend;
    change = true;
    tch = true;
  }
  if (abs(pressureSend - old_pressure) >= pressThreshold) {
    old_pressure = pressureSend;
    change = true;
    pch = true;
  }
  if (abs(humiditySend - old_humidity) >= humThreshold) {
    old_humidity = humiditySend;
    change = true;
    hch = true;
  }

  BATT_COUNT++;
  CORE_DEBUG(PSTR("BATT_COUNT: %d\n"), BATT_COUNT);
  if (BATT_COUNT >= BATT_TIME) {
    CORE_DEBUG(PSTR("BATT_COUNT == BATT_TIME: %d\n"), BATT_COUNT);
    change = true;
  }
  wdt_nrfReset();
}

void readBatt() {
  wait(30);
  batteryVoltage = hwCPUVoltage();
  battery = battery_level_in_percent(batteryVoltage);
  batteryVoltageF = (float)batteryVoltage / 1000.00;
  CORE_DEBUG(PSTR("battery voltage: %d\n"), batteryVoltage);
  CORE_DEBUG(PSTR("battery percentage: %d\n"), battery);
  if (BATT_TIME != 0) {
    bch = true;
  }
}



// ##############################################################################################################
// #                                                 SENSOR SEND                                                #
// ##############################################################################################################

void sendData() {
  if (flag_nogateway_mode == false) {
    if (tch == true) {
      check = send(msgTemp.setDestination(0).set(temperatureSend, 2));
      if (check == false) {
        _transportSM.failedUplinkTransmissions = 0;
        wait(shortWait * 4);
        check = send(msgTemp.setDestination(0).set(temperatureSend, 2));
        if (check == false) {
          _transportSM.failedUplinkTransmissions = 0;
        }
      }
      tch = false;
      checkSend();
    }
    if (hch == true) {
      check = send(msgHum.setDestination(0).set(humiditySend, 2));
      if (check == false) {
        _transportSM.failedUplinkTransmissions = 0;
        wait(shortWait * 4);
        check = send(msgHum.setDestination(0).set(humiditySend, 2));
        if (check == false) {
          wait(shortWait * 8);
          _transportSM.failedUplinkTransmissions = 0;
        }
      }
      hch = false;
      checkSend();
    }
    if (pch == true) {
      check = send(msgPres.setDestination(0).set(pressureSend, 2));
      if (check == false) {
        _transportSM.failedUplinkTransmissions = 0;
        wait(shortWait * 4);
        check = send(msgPres.setDestination(0).set(pressureSend, 2));
        if (check == false) {
          wait(shortWait * 8);
          _transportSM.failedUplinkTransmissions = 0;
        }
      }
      pch = false;
      checkSend();
    }
    if (fch == true) {
      check = send(forecastMsg.set(forecast));
      if (check == false) {
        _transportSM.failedUplinkTransmissions = 0;
        wait(50);
        check = send(forecastMsg.set(forecast));
        wait(50);
      }
      fch = false;
    }
    if (BATT_COUNT >= BATT_TIME) {
      CORE_DEBUG(PSTR("BATT_COUNT == BATT_TIME: %d\n"), BATT_COUNT);
      wait(5);
      readBatt();
      BATT_COUNT = 0;
    }
    if (bch == true) {
      if (BATT_TIME != 0) {
        batLevSend();
      }
      bch = false;
    }
  } else {
    if (BATT_COUNT >= BATT_TIME) {
      CORE_DEBUG(PSTR("BATT_COUNT == BATT_TIME: %d\n"), BATT_COUNT);
      wait(5);
      readBatt();
      BATT_COUNT = 0;
    }
    tch = false;
    hch = false;
    bch = false;
    pch = false;
    fch = false;
  }
  wdt_nrfReset();
}


void checkSend() {
  if (check == true) {
    err_delivery_beat = 0;
    if (flag_nogateway_mode == true) {
      flag_nogateway_mode = false;
      CORE_DEBUG(PSTR("MyS: Flag_nogateway_mode = FALSE\n"));
      CORE_DEBUG(PSTR("MyS: NORMAL GATEWAY MODE\n"));
      err_delivery_beat = 0;
    }
    CORE_DEBUG(PSTR("MyS: SEND BATTERY LEVEL\n"));
    CORE_DEBUG(PSTR("MyS: BATTERY LEVEL %: %d\n"), battery);
  } else {
    _transportSM.failedUplinkTransmissions = 0;
    if (err_delivery_beat < 6) {
      err_delivery_beat++;
    }
    if (err_delivery_beat == 5) {
      if (flag_nogateway_mode == false) {
        happy_node_mode();
        gateway_fail();
        CORE_DEBUG(PSTR("MyS: LOST GATEWAY MODE\n"));
      }
    }
  }
}


void batLevSend() {
  if (battery > 100) {
    battery = 100;
  }
  check = sendBatteryLevel(battery, 1);
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    check = sendBatteryLevel(battery, 1);
  } else {
    wait(1500, C_INTERNAL, I_BATTERY_LEVEL);
  }
  checkSend();
  wdt_nrfReset();
  lqSend();

  check = send(bvMsg.set(batteryVoltageF, 2));
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(100);
    check = send(bvMsg.set(batteryVoltageF, 2));
    _transportSM.failedUplinkTransmissions = 0;
  } else {
    CORE_DEBUG(PSTR("MyS: SEND BATTERY VOLTAGE\n"));
  }
}


void lqSend() {
  linkq = transportGetReceivingRSSI();
  linkq = map(linkq, -85, -40, 0, 100);
  if (linkq < 0) {
    linkq = 0;
  }
  if (linkq > 100) {
    linkq = 100;
  }

  check = send(sqMsg.set(linkq));
  if (!check) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait * 2);
    check = send(sqMsg.set(linkq));
    _transportSM.failedUplinkTransmissions = 0;
  } else {
    CORE_DEBUG(PSTR("MyS: SEND LINK QUALITY\n"));
    CORE_DEBUG(PSTR("MyS: LINK QUALITY %: %d\n"), linkq);
  }
}


static __INLINE uint8_t battery_level_in_percent(const uint16_t mvolts)
{
  uint8_t battery_level;

  if (mvolts >= 3000)
  {
    battery_level = 100;
  }
  else if (mvolts > 2900)
  {
    battery_level = 100 - ((3000 - mvolts) * 20) / 100;
  }
  else if (mvolts > 2750)
  {
    battery_level = 80 - ((2900 - mvolts) * 30) / 150;
  }
  else if (mvolts > 2550)
  {
    battery_level = 50 - ((2750 - mvolts) * 40) / 200;
  }
  else if (mvolts > 2250)
  {
    battery_level = 10 - ((2550 - mvolts) * 10) / 300;
  }
  else
  {
    battery_level = 0;
  }
  return battery_level;
}


void sendConfig() {

  check = send(setBattSendMsg.set(battSend));
  if (check == false) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = send(setBattSendMsg.set(battSend));
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }

  bool inverse = loadState(106);
  check = send(setColor.set(inverse));
  if (check == false) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = send(setColor.set(inverse));
    wait(shortWait * 2);
    _transportSM.failedUplinkTransmissions = 0;
  }
}


void sendResetReason() {
  String reason;
#ifdef MY_RESET_REASON_TEXT
  if (NRF_POWER->RESETREAS == 0) reason = "POWER_ON";
  else {
    if (NRF_POWER->RESETREAS & (1UL << 0)) reason += "PIN_RESET ";
    if (NRF_POWER->RESETREAS & (1UL << 1)) reason += "WDT ";
    if (NRF_POWER->RESETREAS & (1UL << 2)) reason += "SOFT_RESET ";
    if (NRF_POWER->RESETREAS & (1UL << 3)) reason += "LOCKUP";
    if (NRF_POWER->RESETREAS & (1UL << 16)) reason += "WAKEUP_GPIO ";
    if (NRF_POWER->RESETREAS & (1UL << 17)) reason += "LPCOMP ";
    if (NRF_POWER->RESETREAS & (1UL << 17)) reason += "WAKEUP_DEBUG";
  }
#else
  reason = NRF_POWER->RESETREAS;
#endif

  check = send(sendMsg.set(reason.c_str()));
  if (check == false) {
    _transportSM.failedUplinkTransmissions = 0;
    wait(shortWait);
    check = send(sendMsg.set(reason.c_str()));
    if (check == false) {
      wait(shortWait * 3);
      _transportSM.failedUplinkTransmissions = 0;
      check = send(sendMsg.set(reason.c_str()));
      wait(shortWait);
    }
  }
  if (check) NRF_POWER->RESETREAS = (0xFFFFFFFF);
}



void timeConf() {

  SLEEP_TIME = (timeSend * minuteT / SLEEP_TIME_WDT);

  BATT_TIME = (battSend * 60 / timeSend);

  cpNom = (120 / timeSend);

  CORE_DEBUG(PSTR("SLEEP_TIME: %d\n"), SLEEP_TIME);
}



void receive(const MyMessage & message)
{
  if (configMode == true) {

    if (message.sensor == SET_BATT_SEND_ID) {
      if (message.type == V_VAR1) {
        battSend = message.getByte();
        if (battSend > 24) {
          battSend = 24;
        }
        if (battSend <= 0) {
          battSend = 1;
        }
        saveState(103, battSend);
        wait(shortWait);
        send(setBattSendMsg.set(battSend));
        wait(shortWait);
        transportDisable();
        wait(shortWait);
        reportBattInk();
        configMode = false;
        change = true;
        timeConf();
        sleepTimeCount = SLEEP_TIME;
      }
    }

    if (message.sensor == SET_COLOR_ID) {
      if (message.type == V_VAR1) {
        bool colorPrintTemp = message.getBool();
        colorChange(colorPrintTemp);
        wait(shortWait);
        send(setColor.set(colorPrintTemp));
        wait(shortWait);
        transportDisable();
        wait(shortWait);
        configMode = false;
        change = true;
        sleepTimeCount = SLEEP_TIME;
      }
    }
  }
}



void interrupt_Init() {
  nrf_gpio_cfg_input(PIN_BUTTON, NRF_GPIO_PIN_NOPULL);
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
  PIN_BUTTON_MASK = 1 << PIN_BUTTON;
  app_gpiote_user_register(&m_gpiote_user_id, PIN_BUTTON_MASK, PIN_BUTTON_MASK, gpiote_event_handler);
  app_gpiote_user_enable(m_gpiote_user_id);
  buttIntStatus = 0;
}

void gpiote_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
  MY_HW_RTC->CC[0] = (MY_HW_RTC->COUNTER + 2);

  if (PIN_BUTTON_MASK & event_pins_high_to_low) {
    if (buttIntStatus == 0) {
      buttIntStatus = PIN_BUTTON;
    }
  }
}


static __INLINE void wdt_init(void)
{
  NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
  NRF_WDT->CRV = 35 * 32768;
  NRF_WDT->RREN |= WDT_RREN_RR0_Msk;
  NRF_WDT->TASKS_START = 1;
}


static __INLINE void wdt_nrfReset() {
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}


void new_device() {
  hwWriteConfig(EEPROM_NODE_ID_ADDRESS, 255);
  saveState(200, 255);
  hwReboot();
}



// ##############################################################################################################
// #                                                 E-PAPER DISP                                               #
// ##############################################################################################################

void DrawImageWH(Paint * paint, int x, int y, const unsigned char* imgData, int Width, int Height, int colored)
{
  int i, j;
  const unsigned char* prt = imgData;
  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width; i++) {
      if (pgm_read_byte(prt) & (0x80 >> (i % 8))) {
        paint->DrawPixel(x + i, y + j, colored);
      }
      if (i % 8 == 7) {
        prt++;
      }
    }
    if (Width % 8 != 0) {
      prt++;
    }
  }
}


void reseteinkset() {
  updateink1 = false;
  updateink2 = false;
  updateink3 = false;
  updateink4 = false;
  updateinkclear = false;
}


void clearOne() {
  if (colorPrint == true) {
    epd.Clear4();
  } else {
    epd.Clear4();
  }
}


void einkZeropush() {
  epd.Reset();

  if (colorPrint == true) {
    epd.Clear4();
  } else {
    epd.Clear4();
  }

  paith.Clear(colorPrint);

  if (flag_nogateway_mode == false) {
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, CONFEN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, CONF, 180, 82, opposite_colorPrint);
#endif
  } else {
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, FINDEN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, FIND, 180, 82, opposite_colorPrint);
#endif
  }
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}


void einkZeroend() {
  paith.Clear(colorPrint);
  if (flag_nogateway_mode == false) {
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, CONFAEN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, CONFA, 180, 82, opposite_colorPrint);
#endif
  } else {
#ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, FINDAEN, 180, 82, opposite_colorPrint);
#else
    DrawImageWH(&paith, 10, 50, FINDA, 180, 82, opposite_colorPrint);
#endif
  }
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}


void einkOnepush() {
  paith.Clear(colorPrint);

#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, PRESENTEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, PRESENT, 180, 82, opposite_colorPrint);
#endif
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}


void einkOneend() {
  paith.Clear(colorPrint);

#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, PRESENTAEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, PRESENTA, 180, 82, opposite_colorPrint);
#endif
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}


void einkOnePluspush() {
  paith.Clear(colorPrint);

#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, RESETEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, RESET, 180, 82, opposite_colorPrint);
#endif
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}


void einkOnePlusend() {
  paith.Clear(colorPrint);

#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, RESETAEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, RESETA, 180, 82, opposite_colorPrint);
#endif
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
}

/*
  void reportTimeInk() {
    paith.Clear(colorPrint);

  #ifdef LANG_EN
    DrawImageWH(&paith, 10, 50, NEWSENSINTEN, 180, 82, opposite_colorPrint);
  #else
    DrawImageWH(&paith, 10, 50, NEWSENSINT, 180, 82, opposite_colorPrint);
  #endif

    if (timeSend >= 10) {
      byte one_t = timeSend / 10;
      byte two_t = timeSend % 10;
      switch (one_t) {
        case 1:
          DrawImageWH(&paith, 76, 138, S1, 24, 37, opposite_colorPrint);
          break;
        case 2:
          DrawImageWH(&paith, 76, 138, S2, 24, 37, opposite_colorPrint);
          break;
        case 3:
          DrawImageWH(&paith, 76, 138, S3, 24, 37, opposite_colorPrint);
          break;
        case 4:
          DrawImageWH(&paith, 76, 138, S4, 24, 37, opposite_colorPrint);
          break;
        case 5:
          DrawImageWH(&paith, 76, 138, S5, 24, 37, opposite_colorPrint);
          break;
        case 6:
          DrawImageWH(&paith, 76, 138, S6, 24, 37, opposite_colorPrint);
          break;
        case 7:
          DrawImageWH(&paith, 76, 138, S7, 24, 37, opposite_colorPrint);
          break;
        case 8:
          DrawImageWH(&paith, 76, 138, S8, 24, 37, opposite_colorPrint);
          break;
        case 9:
          DrawImageWH(&paith, 76, 138, S9, 24, 37, opposite_colorPrint);
          break;
      }

      switch (two_t) {
        case 0:
          DrawImageWH(&paith, 100, 138, S0, 24, 37, opposite_colorPrint);
          break;
        case 1:
          DrawImageWH(&paith, 100, 138, S1, 24, 37, opposite_colorPrint);
          break;
        case 2:
          DrawImageWH(&paith, 100, 138, S2, 24, 37, opposite_colorPrint);
          break;
        case 3:
          DrawImageWH(&paith, 100, 138, S3, 24, 37, opposite_colorPrint);
          break;
        case 4:
          DrawImageWH(&paith, 100, 138, S4, 24, 37, opposite_colorPrint);
          break;
        case 5:
          DrawImageWH(&paith, 100, 138, S5, 24, 37, opposite_colorPrint);
          break;
        case 6:
          DrawImageWH(&paith, 100, 138, S6, 24, 37, opposite_colorPrint);
          break;
        case 7:
          DrawImageWH(&paith, 100, 138, S7, 24, 37, opposite_colorPrint);
          break;
        case 8:
          DrawImageWH(&paith, 100, 138, S8, 24, 37, opposite_colorPrint);
          break;
        case 9:
          DrawImageWH(&paith, 100, 138, S9, 24, 37, opposite_colorPrint);
          break;
      }
    } else {
      switch (timeSend) {
        case 0:
          DrawImageWH(&paith, 88, 138, S0, 24, 37, opposite_colorPrint);
          break;
        case 1:
          DrawImageWH(&paith, 88, 138, S1, 24, 37, opposite_colorPrint);
          break;
        case 2:
          DrawImageWH(&paith, 88, 138, S2, 24, 37, opposite_colorPrint);
          break;
        case 3:
          DrawImageWH(&paith, 88, 138, S3, 24, 37, opposite_colorPrint);
          break;
        case 4:
          DrawImageWH(&paith, 88, 138, S4, 24, 37, opposite_colorPrint);
          break;
        case 5:
          DrawImageWH(&paith, 88, 138, S5, 24, 37, opposite_colorPrint);
          break;
        case 6:
          DrawImageWH(&paith, 88, 138, S6, 24, 37, opposite_colorPrint);
          break;
        case 7:
          DrawImageWH(&paith, 88, 138, S7, 24, 37, opposite_colorPrint);
          break;
        case 8:
          DrawImageWH(&paith, 88, 138, S8, 24, 37, opposite_colorPrint);
          break;
        case 9:
          DrawImageWH(&paith, 88, 138, S9, 24, 37, opposite_colorPrint);
          break;
      }
    }
    epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
    epd.SendCommand(0x22);
    epd.SendData(0xFF);
    epd.SendCommand(0x20);
    epd.WaitUntilIdle();
    wait(2000);
  }
*/

void reportBattInk() {
  paith.Clear(colorPrint);

#ifdef LANG_EN
  DrawImageWH(&paith, 10, 50, NEWBATTINTEN, 180, 82, opposite_colorPrint);
#else
  DrawImageWH(&paith, 10, 50, NEWBATTINT, 180, 82, opposite_colorPrint);
#endif

  if (battSend >= 10) {
    byte one_t = battSend / 10;
    byte two_t = battSend % 10;
    switch (one_t) {
      case 1:
        DrawImageWH(&paith, 76, 138, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 76, 138, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 76, 138, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 76, 138, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 76, 138, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 76, 138, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 76, 138, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 76, 138, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 76, 138, S9, 24, 37, opposite_colorPrint);
        break;
    }

    switch (two_t) {
      case 0:
        DrawImageWH(&paith, 100, 138, S0, 24, 37, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 100, 138, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 100, 138, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 100, 138, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 100, 138, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 100, 138, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 100, 138, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 100, 138, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 100, 138, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 100, 138, S9, 24, 37, opposite_colorPrint);
        break;
    }
  } else {
    switch (battSend) {
      case 0:
        DrawImageWH(&paith, 88, 138, S0, 24, 37, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 88, 138, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 88, 138, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 88, 138, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 88, 138, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 88, 138, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 88, 138, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 88, 138, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 88, 138, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 88, 138, S9, 24, 37, opposite_colorPrint);
        break;
    }
  }
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();

  wait(2000);
}


void DisplayUpdate() {
  epd.Reset();
  DrawTable();
  //displayDMY();
  DrawTemperature(temperatureSend);
  DrawPressHum(humiditySend, pressureSend);
  DrawIcons(battery, linkq, forecast);
  epd.DisplayWindows2(paith.GetImage(), 0, 0, 200, 200);
  epd.SendCommand(0x22);
  epd.SendData(0xFF);
  epd.SendCommand(0x20);
  epd.WaitUntilIdle();
  epd.Sleep();
}


void DrawTable() {
  epd.Clear4();
  paith.SetWidth(200);
  paith.SetHeight(200);
  paith.SetRotate(ROTATE_0);
  paith.Clear(colorPrint);
  paith.DrawHorizontalLine(0, 175 , 45, opposite_colorPrint);
  paith.DrawHorizontalLine(0, 176 , 45, opposite_colorPrint);
  paith.DrawHorizontalLine(155, 175 , 45, opposite_colorPrint);
  paith.DrawHorizontalLine(155, 176 , 45, opposite_colorPrint);
  paith.DrawLine(45, 175 , 50, 200, opposite_colorPrint);
  paith.DrawLine(44, 176 , 49, 200, opposite_colorPrint);
  paith.DrawLine(155, 175 , 150, 200, opposite_colorPrint);
  paith.DrawLine(156, 176 , 151, 200, opposite_colorPrint);
#ifdef LANG_EN
  DrawImageWH(&paith, 0, 7, TEN, 200, 10, opposite_colorPrint);
  DrawImageWH(&paith, 0, 118, PHEN, 200, 10, opposite_colorPrint);
#else
  DrawImageWH(&paith, 0, 7, T, 200, 10, opposite_colorPrint);
  DrawImageWH(&paith, 0, 118, PH, 200, 10, opposite_colorPrint);
#endif
}



void DrawPressHum(float hum, float pres) {

  int pressure_temp = round(pres);
  //pressure_temp = 1234 ; // checking the output
  int humidity_temp = round(hum);
  byte one_p;
  byte two_p;
  byte three_p;
  byte four_p;

#ifdef LANG_EN
  if (pressure_temp < 1000) {
    one_p = pressure_temp / 100;
    two_p = pressure_temp % 100 / 10;
    three_p = pressure_temp % 10;
  } else {
    one_p = pressure_temp / 1000;
    two_p = pressure_temp % 1000 / 100;
    three_p = pressure_temp % 100 / 10;
    four_p = pressure_temp % 10;
  }
#else
  one_p = pressure_temp / 100;
  two_p = pressure_temp % 100 / 10;
  three_p = pressure_temp % 10;
#endif

  byte one_h = humidity_temp / 10;
  byte two_h = humidity_temp % 10;

#ifdef LANG_EN
  DrawImageWH(&paith, 0, 131, HPER, 24, 37, opposite_colorPrint);
  if (pressure_temp < 1000) {
    DrawImageWH(&paith, 96, 131, PRESSEN, 24, 37, opposite_colorPrint);
  } else {
    DrawImageWH(&paith, 72, 131, PRESSEN, 24, 37, opposite_colorPrint);
  }
#else
  DrawImageWH(&paith, 0, 131, HPER, 24, 37, opposite_colorPrint);
  if (pressure_temp < 1000) {
    DrawImageWH(&paith, 96, 131, PRESS, 24, 37, opposite_colorPrint);
  } else {
    DrawImageWH(&paith, 72, 131, PRESS, 24, 37, opposite_colorPrint);
  }
#endif

  switch (one_p) {
    case 1:
      DrawImageWH(&paith, 168, 130, S1, 24, 37, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 168, 130, S2, 24, 37, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 168, 130, S3, 24, 37, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 168, 130, S4, 24, 37, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 168, 130, S5, 24, 37, opposite_colorPrint);
      break;
    case 6:
      DrawImageWH(&paith, 168, 130, S6, 24, 37, opposite_colorPrint);
      break;
    case 7:
      DrawImageWH(&paith, 168, 130, S7, 24, 37, opposite_colorPrint);
      break;
    case 8:
      DrawImageWH(&paith, 168, 130, S8, 24, 37, opposite_colorPrint);
      break;
    case 9:
      DrawImageWH(&paith, 168, 130, S9, 24, 37, opposite_colorPrint);
      break;
  }

  switch (two_p) {
    case 0:
      DrawImageWH(&paith, 144, 130, S0, 24, 37, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 144, 130, S1, 24, 37, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 144, 130, S2, 24, 37, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 144, 130, S3, 24, 37, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 144, 130, S4, 24, 37, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 144, 130, S5, 24, 37, opposite_colorPrint);
      break;
    case 6:
      DrawImageWH(&paith, 144, 130, S6, 24, 37, opposite_colorPrint);
      break;
    case 7:
      DrawImageWH(&paith, 144, 130, S7, 24, 37, opposite_colorPrint);
      break;
    case 8:
      DrawImageWH(&paith, 144, 130, S8, 24, 37, opposite_colorPrint);
      break;
    case 9:
      DrawImageWH(&paith, 144, 130, S9, 24, 37, opposite_colorPrint);
      break;
  }

  switch (three_p) {
    case 0:
      DrawImageWH(&paith, 120, 130, S0, 24, 37, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 120, 130, S1, 24, 37, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 120, 130, S2, 24, 37, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 120, 130, S3, 24, 37, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 120, 130, S4, 24, 37, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 120, 130, S5, 24, 37, opposite_colorPrint);
      break;
    case 6:
      DrawImageWH(&paith, 120, 130, S6, 24, 37, opposite_colorPrint);
      break;
    case 7:
      DrawImageWH(&paith, 120, 130, S7, 24, 37, opposite_colorPrint);
      break;
    case 8:
      DrawImageWH(&paith, 120, 130, S8, 24, 37, opposite_colorPrint);
      break;
    case 9:
      DrawImageWH(&paith, 120, 130, S9, 24, 37, opposite_colorPrint);
      break;
  }

  if (pressure_temp >= 1000) {
    switch (three_p) {
      case 0:
        DrawImageWH(&paith, 96, 130, S0, 24, 37, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 96, 130, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 96, 130, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 96, 130, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 96, 130, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 96, 130, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 96, 130, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 96, 130, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 96, 130, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 96, 130, S9, 24, 37, opposite_colorPrint);
        break;
    }
  }


  switch (one_h) {
    case 0:
      DrawImageWH(&paith, 48, 130, S0, 24, 37, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 48, 130, S1, 24, 37, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 48, 130, S2, 24, 37, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 48, 130, S3, 24, 37, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 48, 130, S4, 24, 37, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 48, 130, S5, 24, 37, opposite_colorPrint);
      break;
    case 6:
      DrawImageWH(&paith, 48, 130, S6, 24, 37, opposite_colorPrint);
      break;
    case 7:
      DrawImageWH(&paith, 48, 130, S7, 24, 37, opposite_colorPrint);
      break;
    case 8:
      DrawImageWH(&paith, 48, 130, S8, 24, 37, opposite_colorPrint);
      break;
    case 9:
      DrawImageWH(&paith, 48, 130, S9, 24, 37, opposite_colorPrint);
      break;
  }

  switch (two_h) {
    case 0:
      DrawImageWH(&paith, 24, 130, S0, 24, 37, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 24, 130, S1, 24, 37, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 24, 130, S2, 24, 37, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 24, 130, S3, 24, 37, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 24, 130, S4, 24, 37, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 24, 130, S5, 24, 37, opposite_colorPrint);
      break;
    case 6:
      DrawImageWH(&paith, 24, 130, S6, 24, 37, opposite_colorPrint);
      break;
    case 7:
      DrawImageWH(&paith, 24, 130, S7, 24, 37, opposite_colorPrint);
      break;
    case 8:
      DrawImageWH(&paith, 24, 130, S8, 24, 37, opposite_colorPrint);
      break;
    case 9:
      DrawImageWH(&paith, 24, 130, S9, 24, 37, opposite_colorPrint);
      break;
  }
}


void DrawIcons(uint8_t batt, int16_t lq, int16_t fc) {

  if (batt == 0) {
    DrawImageWH(&paith, 160, 178, IMAGE_B0, 32, 16, opposite_colorPrint);
  }
  if (batt > 0 && batt < 5) {
    DrawImageWH(&paith, 160, 178, IMAGE_B5, 32, 16, opposite_colorPrint);
  }
  if (batt >= 5 && batt < 10) {
    DrawImageWH(&paith, 160, 178, IMAGE_B10, 32, 16, opposite_colorPrint);
  }
  if (batt >= 10 && batt < 15) {
    DrawImageWH(&paith, 160, 178, IMAGE_B15, 32, 16, opposite_colorPrint);
  }
  if (batt >= 15 && batt <= 20) {
    DrawImageWH(&paith, 160, 178, IMAGE_B20, 32, 16, opposite_colorPrint);
  }
  if (batt >= 20 && batt <= 25) {
    DrawImageWH(&paith, 160, 178, IMAGE_B25, 32, 16, opposite_colorPrint);
  }
  if (batt >= 25 && batt <= 30) {
    DrawImageWH(&paith, 160, 178, IMAGE_B30, 32, 16, opposite_colorPrint);
  }
  if (batt >= 30 && batt <= 35) {
    DrawImageWH(&paith, 160, 178, IMAGE_B35, 32, 16, opposite_colorPrint);
  }
  if (batt >= 35 && batt <= 40) {
    DrawImageWH(&paith, 160, 178, IMAGE_B40, 32, 16, opposite_colorPrint);
  }
  if (batt >= 40 && batt <= 45) {
    DrawImageWH(&paith, 160, 178, IMAGE_B45, 32, 16, opposite_colorPrint);
  }
  if (batt >= 45 && batt <= 50) {
    DrawImageWH(&paith, 160, 178, IMAGE_B50, 32, 16, opposite_colorPrint);
  }
  if (batt >= 50 && batt <= 55) {
    DrawImageWH(&paith, 160, 178, IMAGE_B55, 32, 16, opposite_colorPrint);
  }
  if (batt >= 55 && batt <= 60) {
    DrawImageWH(&paith, 160, 178, IMAGE_B60, 32, 16, opposite_colorPrint);
  }
  if (batt >= 60 && batt <= 65) {
    DrawImageWH(&paith, 160, 178, IMAGE_B65, 32, 16, opposite_colorPrint);
  }
  if (batt >= 65 && batt <= 70) {
    DrawImageWH(&paith, 160, 178, IMAGE_B70, 32, 16, opposite_colorPrint);
  }
  if (batt >= 70 && batt <= 75) {
    DrawImageWH(&paith, 160, 178, IMAGE_B75, 32, 16, opposite_colorPrint);
  }
  if (batt >= 75 && batt <= 80) {
    DrawImageWH(&paith, 160, 178, IMAGE_B80, 32, 16, opposite_colorPrint);
  }
  if (batt >= 80 && batt <= 85) {
    DrawImageWH(&paith, 160, 178, IMAGE_B85, 32, 16, opposite_colorPrint);
  }
  if (batt >= 85 && batt <= 90) {
    DrawImageWH(&paith, 160, 178, IMAGE_B90, 32, 16, opposite_colorPrint);
  }
  if (batt >= 90 && batt <= 95) {
    DrawImageWH(&paith, 160, 178, IMAGE_B95, 32, 16, opposite_colorPrint);
  }
  if (batt >= 95 && batt <= 100) {
    DrawImageWH(&paith, 160, 178, IMAGE_B100, 32, 16, opposite_colorPrint);
  }


  if (lq == 0) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN0, 24, 16, opposite_colorPrint);
  }
  if (lq > 0 && lq < 15) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN15, 24, 16, opposite_colorPrint);
  }
  if (lq > 15 && lq < 30) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN30, 24, 16, opposite_colorPrint);
  }
  if (lq >= 30 && lq < 45) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN45, 24, 16, opposite_colorPrint);
  }
  if (lq >= 45 && lq < 60) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN60, 24, 16, opposite_colorPrint);
  }
  if (lq >= 60 && lq <= 80) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN80, 24, 16, opposite_colorPrint);
  }
  if (lq >= 80 && lq <= 100) {
    DrawImageWH(&paith, 16, 178, IMAGE_KN100, 24, 16, opposite_colorPrint);
  }


#ifdef LANG_EN
  switch (fc) {
    case 0:
      DrawImageWH(&paith, 56, 171, IMAGE_FC0EN, 88, 24, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 56, 171, IMAGE_FC1EN, 88, 24, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 56, 171, IMAGE_FC2EN, 88, 24, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 56, 171, IMAGE_FC3EN, 88, 24, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 56, 171, IMAGE_FC4EN, 88, 24, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 56, 171, IMAGE_FC5EN, 88, 24, opposite_colorPrint);
      break;
    case 10:
      DrawImageWH(&paith, 56, 171, IMAGE_FC10EN, 88, 24, opposite_colorPrint);
      break;
    case 11:
      DrawImageWH(&paith, 56, 171, IMAGE_FC11EN, 88, 24, opposite_colorPrint);
      break;
  }
#else
  switch (fc) {
    case 0:
      DrawImageWH(&paith, 56, 171, IMAGE_FC0, 88, 24, opposite_colorPrint);
      break;
    case 1:
      DrawImageWH(&paith, 56, 171, IMAGE_FC1, 88, 24, opposite_colorPrint);
      break;
    case 2:
      DrawImageWH(&paith, 56, 171, IMAGE_FC2, 88, 24, opposite_colorPrint);
      break;
    case 3:
      DrawImageWH(&paith, 56, 171, IMAGE_FC3, 88, 24, opposite_colorPrint);
      break;
    case 4:
      DrawImageWH(&paith, 56, 171, IMAGE_FC4, 88, 24, opposite_colorPrint);
      break;
    case 5:
      DrawImageWH(&paith, 56, 171, IMAGE_FC5, 88, 24, opposite_colorPrint);
      break;
    case 10:
      DrawImageWH(&paith, 56, 171, IMAGE_FC5, 88, 24, opposite_colorPrint);
      break;
    case 11:
      DrawImageWH(&paith, 56, 171, IMAGE_FC5, 88, 24, opposite_colorPrint);
      break;
  }
#endif
}

/*
  void displayDMY() {
  int yearPrint = year();
  if (yearPrint == 1970) {
    DrawImageWH(&paith, 53, 4, LTM, 96, 14, opposite_colorPrint);
  } else {
  #ifdef LANG_EN
    byte year_one = yearPrint / 1000;
    byte year_two = yearPrint % 1000 / 100;
    byte year_three = yearPrint % 100 / 10;
    byte year_four = yearPrint % 10;

    switch (year_one) {
      case 0:
        DrawImageWH(&paith, 60, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 60, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 60, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 60, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 60, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 60, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 60, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 60, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 60, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 60, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_two) {
      case 0:
        DrawImageWH(&paith, 49, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 49, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 49, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 49, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 49, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 49, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 49, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 49, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 49, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 49, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_three) {
      case 0:
        DrawImageWH(&paith, 38, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 38, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 38, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 38, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 38, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 38, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 38, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 38, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 38, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 38, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_four) {
      case 0:
        DrawImageWH(&paith, 27, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 27, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 27, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 27, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 27, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 27, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 27, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 27, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 27, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 27, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    int monthPrint = month();
    byte month_one;
    byte month_two;
    if (monthPrint > 9) {
      month_one = monthPrint / 10;
      month_two = monthPrint % 10;
    } else {
      month_one = 0;
      month_two = monthPrint;
    }
    switch (month_two) {
      case 0:
        DrawImageWH(&paith, 101, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 101, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 101, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 101, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 101, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 101, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 101, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 101, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 101, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 101, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (month_one) {
      case 0:
        DrawImageWH(&paith, 112, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 112, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 112, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 112, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 112, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 112, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 112, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 112, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 112, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 112, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    DrawImageWH(&paith, 71, 4, DATEPOINT, 4, 13, opposite_colorPrint);

    int dayPrint = day();
    byte day_one;
    byte day_two;
    if (dayPrint > 9) {
      day_one = dayPrint / 10;
      day_two = dayPrint % 10;
    } else {
      day_one = 0;
      day_two = dayPrint;
    }

    switch (day_one) {
      case 0:
        DrawImageWH(&paith, 86, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 86, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 86, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 86, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 86, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 86, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 86, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 86, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 86, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 86, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (day_two) {
      case 0:
        DrawImageWH(&paith, 75, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 75, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 75, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 75, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 75, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 75, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 75, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 75, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 75, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 75, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    DrawImageWH(&paith, 97, 4, DATEPOINT, 4, 13, opposite_colorPrint);

    byte weekdayPrint = weekday(); // Sunday is day 1
    switch (weekdayPrint) {
      case 1:
        DrawImageWH(&paith, 132, 3, SUNDAYEN, 43, 13, opposite_colorPrint);
        DrawImageWH(&paith, 132, 3, SUNDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 132, 3, MONDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 132, 3, TUESDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 132, 3, WEDNESDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 132, 3, THURSDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 132, 3, FRIDAYEN, 43, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 132, 3, SATURDAYEN, 43, 13, opposite_colorPrint);
        DrawImageWH(&paith, 132, 3, SATURDAYEN, 43, 13, opposite_colorPrint);
        break;
    }
  #else
    byte year_one = yearPrint % 10;
    byte year_two = yearPrint % 100 / 10;
    byte year_three = yearPrint % 1000 / 100;
    byte year_four = yearPrint / 1000; 0;

    switch (year_one) {
      case 0:
        DrawImageWH(&paith, 32, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 32, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 32, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 32, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 32, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 32, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 32, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 32, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 32, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 33, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_two) {
      case 0:
        DrawImageWH(&paith, 43, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 43, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 43, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 43, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 43, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 43, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 43, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 43, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 43, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 43, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_three) {
      case 0:
        DrawImageWH(&paith, 54, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 54, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 54, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 54, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 54, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 54, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 54, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 54, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 54, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 54, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (year_four) {
      case 0:
        DrawImageWH(&paith, 65, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 65, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 65, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 65, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 65, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 65, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 65, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 65, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 65, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 65, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    int monthPrint = month();
    byte month_one;
    byte month_two;
    if (monthPrint > 9) {
      month_one = monthPrint / 10;
      month_two = monthPrint % 10;
    } else {
      month_one = 0;
      month_two = monthPrint;
    }
    switch (month_one) {
      case 0:
        DrawImageWH(&paith, 91, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 91, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 91, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 91, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 91, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 91, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 91, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 91, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 91, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 91, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (month_two) {
      case 0:
        DrawImageWH(&paith, 80, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 80, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 80, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 80, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 80, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 80, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 80, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 80, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 80, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 80, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    DrawImageWH(&paith, 76, 4, DATEPOINT, 4, 13, opposite_colorPrint);

    int dayPrint = day();
    byte day_one;
    byte day_two;
    if (dayPrint > 9) {
      day_one = dayPrint / 10;
      day_two = dayPrint % 10;
    } else {
      day_one = 0;
      day_two = dayPrint;
    }

    switch (day_one) {
      case 0:
        DrawImageWH(&paith, 117, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 117, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 117, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 117, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 117, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 117, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 117, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 117, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 117, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 117, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    switch (day_two) {
      case 0:
        DrawImageWH(&paith, 106, 4, DATE0, 11, 13, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 106, 4, DATE1, 11, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 106, 4, DATE2, 11, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 106, 4, DATE3, 11, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 106, 4, DATE4, 11, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 106, 4, DATE5, 11, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 106, 4, DATE6, 11, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 106, 4, DATE7, 11, 13, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 106, 4, DATE8, 11, 13, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 106, 4, DATE9, 11, 13, opposite_colorPrint);
        break;
    }

    DrawImageWH(&paith, 102, 4, DATEPOINT, 4, 13, opposite_colorPrint);

    weekdayPrint = weekday(); // Sunday is day 1

    switch (weekdayPrint) {
      case 1:
        DrawImageWH(&paith, 139, 3, SUNDAY, 37, 13, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 139, 3, MONDAY, 37, 13, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 139, 3, TUESDAY, 37, 13, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 139, 3, WEDNESDAY, 37, 13, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 139, 3, THURSDAY, 37, 13, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 139, 3, FRIDAY, 37, 13, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 139, 3, SATURDAY, 37, 13, opposite_colorPrint);
        break;
    }
  #endif
  }
  }
*/


void DrawTemperature(float temp) {
  int temperature_temp = round(temp * 10.0);

  if (temperature_temp >= 100) {
    byte one_t = temperature_temp / 100;
    byte two_t = temperature_temp % 100 / 10;
    byte three_t = temperature_temp % 10;

    switch (two_t) {
      case 0:
        DrawImageWH(&paith, 52, 25, L0, 48, 79, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 52, 25, L1, 48, 79, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 52, 25, L2, 48, 79, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 52, 25, L3, 48, 79, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 52, 25, L4, 48, 79, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 52, 25, L5, 48, 79, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 52, 25, L6, 48, 79, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 52, 25, L7, 48, 79, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 52, 25, L8, 48, 79, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 52, 25, L9, 48, 79, opposite_colorPrint);
        break;
    }

    switch (one_t) {
      case 0:
        DrawImageWH(&paith, 100, 25, L0, 48, 79, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 100, 25, L1, 48, 79, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 100, 25, L2, 48, 79, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 100, 25, L3, 48, 79, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 100, 25, L4, 48, 79, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 100, 25, L5, 48, 79, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 100, 25, L6, 48, 79, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 100, 25, L7, 48, 79, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 100, 25, L8, 48, 79, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 100, 25, L9, 48, 79, opposite_colorPrint);
        break;
    }
    DrawImageWH(&paith, 46, 95, LPOINT, 6, 8, opposite_colorPrint);

    switch (three_t) {
      case 0:
        DrawImageWH(&paith, 22, 67, S0, 24, 37, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 22, 67, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 22, 67, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 22, 67, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 22, 67, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 22, 67, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 22, 67, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 22, 67, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 22, 67, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 22, 67, S9, 24, 37, opposite_colorPrint);
        break;
    }
    DrawImageWH(&paith, 24, 37, TC, 23, 23, opposite_colorPrint);
  } else {

    byte one_t = temperature_temp / 10;
    byte two_t = temperature_temp % 10;

    switch (one_t) {
      case 0:
        DrawImageWH(&paith, 76, 25, L0, 48, 79, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 76, 25, L1, 48, 79, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 76, 25, L2, 48, 79, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 76, 25, L3, 48, 79, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 76, 25, L4, 48, 79, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 76, 25, L5, 48, 79, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 76, 25, L6, 48, 79, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 76, 25, L7, 48, 79, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 76, 25, L8, 48, 79, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 76, 25, L9, 48, 79, opposite_colorPrint);
        break;
    }
    DrawImageWH(&paith, 70, 95, LPOINT, 6, 8, opposite_colorPrint);

    switch (two_t) {
      case 0:
        DrawImageWH(&paith, 32, 67, S0, 24, 37, opposite_colorPrint);
        break;
      case 1:
        DrawImageWH(&paith, 32, 67, S1, 24, 37, opposite_colorPrint);
        break;
      case 2:
        DrawImageWH(&paith, 32, 67, S2, 24, 37, opposite_colorPrint);
        break;
      case 3:
        DrawImageWH(&paith, 32, 67, S3, 24, 37, opposite_colorPrint);
        break;
      case 4:
        DrawImageWH(&paith, 32, 67, S4, 24, 37, opposite_colorPrint);
        break;
      case 5:
        DrawImageWH(&paith, 32, 67, S5, 24, 37, opposite_colorPrint);
        break;
      case 6:
        DrawImageWH(&paith, 32, 67, S6, 24, 37, opposite_colorPrint);
        break;
      case 7:
        DrawImageWH(&paith, 32, 67, S7, 24, 37, opposite_colorPrint);
        break;
      case 8:
        DrawImageWH(&paith, 32, 67, S8, 24, 37, opposite_colorPrint);
        break;
      case 9:
        DrawImageWH(&paith, 32, 67, S9, 24, 37, opposite_colorPrint);
        break;
    }
    DrawImageWH(&paith, 48, 37, TC, 23, 23, opposite_colorPrint);
  }
}


void colorChange(bool flag) {
  if (flag == true) {
    colorPrint = true;
    opposite_colorPrint = false;
  } else {
    colorPrint = false;
    opposite_colorPrint = true;
  }
  saveState(106, flag);
}



// ####################################################################################################
// #                                                                                                  #
// #                                            HAPPY MODE                                            #
// #                                                                                                  #
// ####################################################################################################

void happy_init() {

  if (hwReadConfig(EEPROM_NODE_ID_ADDRESS) == 0) {
    hwWriteConfig(EEPROM_NODE_ID_ADDRESS, 255);
  }
  if (loadState(200) == 0) {
    saveState(200, 255);
  }
  CORE_DEBUG(PSTR("EEPROM NODE ID: %d\n"), hwReadConfig(EEPROM_NODE_ID_ADDRESS));
  CORE_DEBUG(PSTR("USER MEMORY SECTOR NODE ID: %d\n"), loadState(200));

  if (hwReadConfig(EEPROM_NODE_ID_ADDRESS) == 255) {
    mtwr = 25000;
  } else {
    mtwr = 8000;
    no_present();
  }
  CORE_DEBUG(PSTR("MY_TRANSPORT_WAIT_MS: %d\n"), mtwr);
}


void config_Happy_node() {
  if (mtwr == 25000) {
    myid = getNodeId();
    saveState(200, myid);
    if (isTransportReady() == true) {
      mypar = _transportConfig.parentNodeId;
      old_mypar = mypar;
      saveState(201, mypar);
      saveState(202, _transportConfig.distanceGW);
    }
    if (isTransportReady() == false)
    {
      no_present();
      err_delivery_beat = 7;
      _transportConfig.nodeId = myid;
      _transportConfig.parentNodeId = loadState(201);
      _transportConfig.distanceGW = loadState(202);
      mypar = _transportConfig.parentNodeId;
      happy_node_mode();
      gateway_fail();
    }
  }
  if (mtwr != 25000) {
    myid = getNodeId();
    if (myid != loadState(200)) {
      saveState(200, myid);
    }
    if (isTransportReady() == true) {
      mypar = _transportConfig.parentNodeId;
      if (mypar != loadState(201)) {
        saveState(201, mypar);
      }
      if (_transportConfig.distanceGW != loadState(202)) {
        saveState(202, _transportConfig.distanceGW);
      }
      present_only_parent();
    }
    if (isTransportReady() == false)
    {
      no_present();
      err_delivery_beat = 6;
      _transportConfig.nodeId = myid;
      _transportConfig.parentNodeId = loadState(201);
      _transportConfig.distanceGW = loadState(202);
      mypar = _transportConfig.parentNodeId;
      happy_node_mode();
      gateway_fail();
    }
  }
}


void check_parent() {
  _transportSM.findingParentNode = true;
  CORE_DEBUG(PSTR("MyS: SEND FIND PARENT REQUEST, WAIT RESPONSE\n"));
  _sendRoute(build(_msg, 255, NODE_SENSOR_ID, C_INTERNAL, 7).set(""));
  wait(1000, C_INTERNAL, 8);
  if (_msg.sensor == 255) {
    if (mGetCommand(_msg) == C_INTERNAL) {
      if (_msg.type == 8) {
        Ack_FP = true;
        CORE_DEBUG(PSTR("MyS: PARENT RESPONSE FOUND\n"));
      }
    }
  }
  if (Ack_FP == true) {
    CORE_DEBUG(PSTR("MyS: FIND PARENT PROCESS\n"));
    Ack_FP = false;
    transportSwitchSM(stParent);
    flag_nogateway_mode = false;
    CORE_DEBUG(PSTR("MyS: Flag_nogateway_mode = FALSE\n"));
    flag_find_parent_process = true;
  } else {
    _transportSM.findingParentNode = false;
    CORE_DEBUG(PSTR("MyS: PARENT RESPONSE NOT FOUND\n"));
    _transportSM.failedUplinkTransmissions = 0;
    CORE_DEBUG(PSTR("TRANSPORT: %d\n"), isTransportReady());
    transportDisable();
    change = true;
  }
}


void find_parent_process() {
  flag_update_transport_param = true;
  flag_find_parent_process = false;
  CORE_DEBUG(PSTR("MyS: STANDART TRANSPORT MODE IS RESTORED\n"));
  err_delivery_beat = 0;
}


void gateway_fail() {
  flag_nogateway_mode = true;
  CORE_DEBUG(PSTR("MyS: Flag_nogateway_mode = TRUE\n"));
  flag_update_transport_param = false;
  change = true;
}


void happy_node_mode() {
  _transportSM.findingParentNode = false;
  _transportSM.transportActive = true;
  _transportSM.uplinkOk = true;
  _transportSM.pingActive = false;
  _transportSM.failureCounter = 0u;
  _transportSM.uplinkOk = true;
  _transportSM.failureCounter = 0u;
  _transportSM.failedUplinkTransmissions = 0u;
  transportSwitchSM(stReady);
  CORE_DEBUG(PSTR("TRANSPORT: %d\n"), isTransportReady());
}


void present_only_parent() {
  if (old_mypar != mypar) {
    CORE_DEBUG(PSTR("MyS: SEND LITTLE PRESENT:) WITH PARENT ID\n"));
    if (_sendRoute(build(_msgTmp, 0, NODE_SENSOR_ID, C_INTERNAL, 6).set(mypar))) {
      flag_sendRoute_parent = false;
      old_mypar = mypar;
    } else {
      flag_sendRoute_parent = true;
    }
  }
}


void update_Happy_transport() {
  CORE_DEBUG(PSTR("MyS: UPDATE TRANSPORT CONFIGURATION\n"));
  mypar = _transportConfig.parentNodeId;
  if (mypar != loadState(201))
  {
    saveState(201, mypar);
  }
  if (_transportConfig.distanceGW != loadState(202))
  {
    saveState(202, _transportConfig.distanceGW);
  }
  present_only_parent();
  wait(shortWait);
  flag_update_transport_param = false;
  sleepTimeCount = SLEEP_TIME;
  BATT_COUNT = BATT_TIME;
  change = true;
}


void no_present() {
  _coreConfig.presentationSent = true;
  _coreConfig.nodeRegistered = true;
}


// ####################################################################################################
// #                                                                                                  #
// #            These functions are only included if the forecast function is enables.                #
// #          The are used to generate a weater prediction by checking if the barometric              #
// #                          pressure is rising or falling over time.                                #
// #                                                                                                  #
// ####################################################################################################

float getLastPressureSamplesAverage()
{
  float lastPressureSamplesAverage = 0;
  for (int i = 0; i < LAST_SAMPLES_COUNT; i++) {
    lastPressureSamplesAverage += lastPressureSamples[i];
  }
  lastPressureSamplesAverage /= LAST_SAMPLES_COUNT;

  return lastPressureSamplesAverage;
}

// Forecast algorithm found here
// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
// Pressure in hPa -->  forecast done by calculating kPa/h
int sample(float pressure) {
  // Calculate the average of the last n minutes.
  int index = minuteCount % LAST_SAMPLES_COUNT;
  lastPressureSamples[index] = pressure;

  minuteCount++;
  if (minuteCount > 185) {
    minuteCount = 6;
  }

  if (minuteCount == 5) {
    pressureAvg = getLastPressureSamplesAverage();
  }
  else if (minuteCount == 35) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { // first time initial 3 hour
      dP_dt = change * 2; // note this is for t = 0.5hour
    }
    else {
      dP_dt = change / 1.5; // divide by 1.5 as this is the difference in time from 0 value.
    }
  }
  else if (minuteCount == 65) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { //first time initial 3 hour
      dP_dt = change; //note this is for t = 1 hour
    }
    else {
      dP_dt = change / 2; //divide by 2 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 95) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { // first time initial 3 hour
      dP_dt = change / 1.5; // note this is for t = 1.5 hour
    }
    else {
      dP_dt = change / 2.5; // divide by 2.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 125) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    pressureAvg2 = lastPressureAvg; // store for later use.
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { // first time initial 3 hour
      dP_dt = change / 2; // note this is for t = 2 hour
    }
    else {
      dP_dt = change / 3; // divide by 3 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 155) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { // first time initial 3 hour
      dP_dt = change / 2.5; // note this is for t = 2.5 hour
    }
    else {
      dP_dt = change / 3.5; // divide by 3.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 185) {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) { // first time initial 3 hour
      dP_dt = change / 3; // note this is for t = 3 hour
    }
    else {
      dP_dt = change / 4; // divide by 4 as this is the difference in time from 0 value
    }
    pressureAvg = pressureAvg2; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
    firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
  }

  int16_t forecast = UNKNOWN;
  if (minuteCount < 35 && firstRound) { //if time is less than 35 min on the first 3 hour interval.
    forecast = UNKNOWN;
  }
  else if (dP_dt < (-0.25)) {
    forecast = THUNDERSTORM;
  }
  else if (dP_dt > 0.25) {
    forecast = UNSTABLE;
  }
  else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05))) {
    forecast = CLOUDY;
  }
  else if ((dP_dt > 0.05) && (dP_dt < 0.25))
  {
    forecast = SUNNY;
  }
  else if ((dP_dt > (-0.05)) && (dP_dt < 0.05)) {
    forecast = STABLE;
  }
  else {
    forecast = UNKNOWN;
  }
  return forecast;
}
