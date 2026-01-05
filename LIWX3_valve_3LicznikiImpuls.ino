#include <SuplaDevice.h>//////https://github.com/SUPLA
#include <WiFiClientSecure.h>
#include <supla/network/esp_wifi.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/network/html/custom_parameter.h>
#include <supla/network/html/custom_text_parameter.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/device/notifications.h>
#include <supla/control/relay.h>
#include <supla/clock/clock.h> //int clock1.getYear(); getMonth(); getDay(); getDayOfWeek();1 - Sunday, 2 - Monday getHour(); getMin(); getSec();
#include "HWCDC.h"
#include <supla/device/enter_cfg_mode_after_power_cycle.h>
#include <supla/sensor/impulse_counter.h>
#include <supla/storage/eeprom.h>

Supla::Eeprom eeprom;

Supla::Sensor::ImpulseCounter *counter1 = nullptr;
Supla::Sensor::ImpulseCounter *counter2 = nullptr;
Supla::Sensor::ImpulseCounter *counter3 = nullptr;
Supla::ESPWifi wifi;
Supla::Clock clock1;
Supla::LittleFsConfig configSupla(4000);// powiększony rozmiar pamieci z powodu dodatkowych INPUTów
Supla::EspWebServer suplaServer;
Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;

HWCDC USBSerial;

int32_t counter1Pin = 0;
int32_t counter2Pin = 0;
int32_t counter3Pin = 0;
int32_t valveOpenPin = 0;
int32_t valveClosePin = 0;

const char PARAM1[] = "night_h_st:1";
const char PARAM2[] = "night_h_end:1";
const char PARAM3[] = "dvNameMssg:1";
const char PARAM4[] = "message:1";
const char PARAM5[] = "lvlAlarm:1";
const char PARAM6[] = "lvlAlarmN:1";
const char PARAM7[] = "periodTtime:1";
const char PARAM8[] = "periodTimeN:1";

const char PARAM9[] = "night_h_st:2";
const char PARAM10[] = "night_h_end:2";
const char PARAM11[] = "dvNameMssg:2";
const char PARAM12[] = "message:2";
const char PARAM13[] = "lvlAlarm:2";
const char PARAM14[] = "lvlAlarmN:2";
const char PARAM15[] = "periodTtime:2";
const char PARAM16[] = "periodTtimeN:2";

const char PARAM17[] = "night_h_st:3";
const char PARAM18[] = "night_h_end:3";
const char PARAM19[] = "dvNameMssg:3";
const char PARAM20[] = "message:3";
const char PARAM21[] = "lvlAlarm:3";
const char PARAM22[] = "lvlAlarmN:3";
const char PARAM23[] = "periodTtime:3";
const char PARAM24[] = "periodTtimeN:3";
const char PARAM25[] = "numberOfCounters";
const char PARAM26[] = "counter1Pin";
const char PARAM27[] = "counter2Pin";
const char PARAM28[] = "counter3Pin";
const char PARAM29[] = "valveOpenPin";
const char PARAM30[] = "valveClosePin";

unsigned long counter_1 = 0;
unsigned long tempCounter_1;
unsigned long tempCounterNight_1;
boolean ResetTempCounterNight_1= 0;
unsigned long counter_2 = 0;
unsigned long tempCounter_2;
unsigned long tempCounterNight_2;
boolean ResetTempCounterNight_2= 0;
unsigned long counter_3 = 0;
unsigned long tempCounter_3;
unsigned long tempCounterNight_3;
boolean ResetTempCounterNight_3= 0;

int32_t night_h_start_1 =1;
int32_t night_h_stop_1 =1;
char dev_name_message_1[200] ="a";
char message_1[200] ="a";
int32_t level_alarm_1 = 40;
int32_t level_alarm_night_1 = 10;
int32_t time_period_1 = 40;
int32_t time_period_night_1 = 10;

int32_t night_h_start_2 =1;
int32_t night_h_stop_2 =1;
char dev_name_message_2[200] ="a";
char message_2[200]="a";
int32_t level_alarm_2 = 40;
int32_t level_alarm_night_2 = 10;
int32_t time_period_2 = 40;
int32_t time_period_night_2 = 10;

int32_t night_h_start_3 =1;
int32_t night_h_stop_3 =1;
char dev_name_message_3[200] ="a";
char message_3[200] ="a";
int32_t level_alarm_3 = 40;
int32_t level_alarm_night_3 = 10;
int32_t time_period_3 = 40;
int32_t time_period_night_3 = 10;

int32_t num_of_counter = 0;
String top1 = "xxx";
String top2 = "xxx";
String top3 = "xxx";
String tim;

boolean first_run1=true;
boolean first_run2=true;
boolean first_run3=true;
boolean night_1 = 0;
boolean night_2 = 0;
boolean night_3 = 0;
boolean on_milli_start = 0;
boolean off_milli_start = 0;
unsigned long valveOn_millis;
unsigned long valveOff_millis;
unsigned long sek_millis;
unsigned long prev_min_millis_1;
unsigned long prev_hour_millis_1;
unsigned long prev_min_millis_2;
unsigned long prev_hour_millis_2;
unsigned long prev_min_millis_3;
unsigned long prev_hour_millis_3;
unsigned long prev_minute_millis;
unsigned long lastReconnectAttempt = 0;
Supla::Control::Relay *valveOpen = nullptr;
Supla::Control::Relay *valveClose = nullptr;

void setup() {
  USBSerial.begin(9600);
  Supla::Storage::Init();
 
  SuplaDevice.addClock(new Supla::Clock);
  auto clock1 = SuplaDevice.getClock();
  if(num_of_counter==0){
   new Supla::Html::DeviceInfo(&SuplaDevice);
   new Supla::Html::WifiParameters;
   new Supla::Html::ProtocolParameters;
   new Supla::Html::CustomParameter(PARAM25, "number of counters(1,2,3)");
  }

  if (Supla::Storage::ConfigInstance()->getInt32(PARAM25, &num_of_counter)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM25, num_of_counter);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM25);
  }

  if(num_of_counter==1){
    inputs1();
  }
  else if(num_of_counter==2){
    inputs1();
    inputs2();
   }
  else if(num_of_counter==3){
    inputs1();
    inputs2();
    inputs3();
  }
  counter1 = new Supla::Sensor::ImpulseCounter(counter1Pin, true, 10);

  counter2 = new Supla::Sensor::ImpulseCounter(counter2Pin, true, 10);

  counter3 = new Supla::Sensor::ImpulseCounter(counter3Pin, true, 10);

  
  new Supla::Device::EnterCfgModeAfterPowerCycle(15000, 3, true);
  Supla::Notification::RegisterNotification(-1);
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);
    
  paramSave();
  
  valveOpen = new Supla::Control::Relay(valveOpenPin, false);
  valveClose = new Supla::Control::Relay(valveClosePin, false);
  pinMode(counter1Pin, INPUT);
  pinMode(counter2Pin, INPUT);
  pinMode(counter3Pin, INPUT);
  pinMode(valveClosePin, OUTPUT);
  pinMode(valveOpenPin, OUTPUT);
  pinMode(valveClosePin, OUTPUT);
  valveOpen->turnOff();
  valveClose->turnOff();
  prev_min_millis_1 = millis();
  prev_hour_millis_1 = millis();
  prev_min_millis_2 = millis();
  prev_hour_millis_2 = millis();
  prev_min_millis_3 = millis();
  prev_hour_millis_3 = millis();
  sek_millis = millis();
  lastReconnectAttempt = 0;

  counter_1 = counter1->getCounter();
  counter_2 = counter1->getCounter();
  counter_3 = counter1->getCounter();

  delay(2000);
  
  USBSerial.print("message:.............1: ");
  USBSerial.println(message_1);
  USBSerial.print("devName message:.............1: ");
  USBSerial.println(dev_name_message_1);
  USBSerial.print("level alarm:.............1: ");
  USBSerial.println(level_alarm_1);
  USBSerial.print("level alarm night:.............1: ");
  USBSerial.println(level_alarm_night_1);
  USBSerial.print("time period.............1: ");
  USBSerial.println(time_period_1);
  USBSerial.print("time period night.............1: ");
  USBSerial.println(time_period_night_1);
  USBSerial.print("message:.............2: ");
  USBSerial.println(message_2);
  USBSerial.print("devName message:.............2: ");
  USBSerial.println(dev_name_message_2);
  USBSerial.print("level alarm:.............2: ");
  USBSerial.println(level_alarm_2);
  USBSerial.print("level alarm night:.............2: ");
  USBSerial.println(level_alarm_night_2);
  USBSerial.print("time period.............2: ");
  USBSerial.println(time_period_2);
  USBSerial.print("time period night.............2: ");
  USBSerial.println(time_period_night_2);
  USBSerial.print("message:.............3: ");
  USBSerial.println(message_3);
  USBSerial.print("devName message:.............3: ");
  USBSerial.println(dev_name_message_3);
  USBSerial.print("level alarm:.............3: ");
  USBSerial.println(level_alarm_3);
  USBSerial.print("level alarm night:.............3: ");
  USBSerial.println(level_alarm_night_3);
  USBSerial.print("time period.............3: ");
  USBSerial.println(time_period_3);
  USBSerial.print("time period night.............3: ");
  USBSerial.println(time_period_night_3);
  USBSerial.print("numofcounter.............: ");
  USBSerial.println(num_of_counter);
  USBSerial.print("counter1Pin.............: ");
  USBSerial.println(counter1Pin);
  USBSerial.print("counter2Pin.............: ");
  USBSerial.println(counter2Pin);
  USBSerial.print("counter3Pin.............: ");
  USBSerial.println(counter3Pin);
  USBSerial.print("valveOpenPin.............: ");
  USBSerial.println(valveOpenPin);
  USBSerial.print("valveClosePin.............: ");
  USBSerial.println(valveClosePin);
  
  SuplaDevice.begin();
}

void loop() {
 SuplaDevice.iterate();
  if(num_of_counter != 0){
   valveControl();
    nightReady();

  if(num_of_counter==1){
    waterControl1();
  }
  else if(num_of_counter==2){
    waterControl1();
    waterControl2();
   }
  else if(num_of_counter==3){
    waterControl1();
    waterControl2();
    waterControl3();
  }
}
}


void paramSave(){
 if(num_of_counter==1){
  stor1();
 }
 else if(num_of_counter==2){
  stor1();
  stor2();
 }
 else if(num_of_counter==3){
  stor1();
  stor2();
  stor3();
 }
}
void valveControl(){
  if((millis() - sek_millis) >100){
    isOnValve();
    sek_millis = millis();
  }
}
void isOnValve(){
  if(valveClose->isOn()) { 
    valveOpen->turnOff();
    if(!off_milli_start){
     USBSerial.print("valveClose:  ");
     USBSerial.println(valveClose->isOn());
     valveOff_millis = millis();
     off_milli_start = true;
    }
    else if((off_milli_start) && ((millis() - valveOff_millis) > 60000)){
     valveClose->turnOff();
     USBSerial.print("valveClose:  ");
     USBSerial.println(valveClose->isOn());
     off_milli_start = false;
    }
  }
  if(valveOpen->isOn()) { 
    valveClose->turnOff();
    if(!on_milli_start){
     USBSerial.print("valveOpen:  ");
     USBSerial.println(valveOpen->isOn());
     valveOn_millis = millis();
     on_milli_start = true;
    }
    else if((on_milli_start) && ((millis() - valveOn_millis) > 60000)){
     valveOpen->turnOff();
     USBSerial.print("valveOpen:  ");
     USBSerial.println(valveOpen->isOn());
     on_milli_start = false;
    }
  }
}

void waterControl1() {
  if (millis() - prev_min_millis_1 > time_period_1) {
    counter_1 = counter1->getCounter();
    USBSerial.print("counter1 reading:.............: ");
    USBSerial.println(counter_1);
    if (counter_1 > (tempCounter_1 + level_alarm_1)) {
      valveOpen->turnOff();
      valveClose->turnOn();
      Supla::Notification::Send(-1, dev_name_message_1, message_1);
    }
    tempCounter_1 = counter_1;
    prev_min_millis_1=millis();
  }
  if ((prev_hour_millis_1 - millis() > time_period_night_1) && night_1) {
    counter_1 = counter1->getCounter();
    USBSerial.print("counter1 reading:.............: ");
    USBSerial.println(counter_1);
    if (counter_1 > (tempCounterNight_1 + level_alarm_night_1)) {
      valveOpen->turnOff();
      valveClose->turnOn();
      String n = "NIGHT1: ";
      String n_message = n + message_1;
      Supla::Notification::Send(-1, dev_name_message_1, n_message.c_str());
    }
    tempCounterNight_1 = counter_1;
    prev_hour_millis_1=millis();
  }
}
void waterControl2() {
  if (millis() - prev_min_millis_2 > time_period_2) {
    counter_2 = counter2->getCounter();
    USBSerial.print("counter2 reading:.............: ");
    USBSerial.println(counter_2);
    if (counter_2 > (tempCounter_2 + level_alarm_2)) {
      Supla::Notification::Send(-1, dev_name_message_2, message_2);
    }
    tempCounter_2 = counter_2;
    prev_min_millis_2=millis();
  }
  if ((prev_hour_millis_2 - millis() > time_period_night_2) && night_2) {
    counter_2 = counter2->getCounter();
    USBSerial.print("counter2 reading:.............: ");
    USBSerial.println(counter_2);
    if (counter_2 > (tempCounterNight_2 + level_alarm_night_2)) {
      String n = "NIGHT2: ";
      String n_message = n + message_2;
      Supla::Notification::Send(-1, dev_name_message_2, n_message.c_str());
    }
    tempCounterNight_2 = counter_2;
    prev_hour_millis_2=millis();
  }
}
void waterControl3() {
  if (millis() - prev_min_millis_3 > time_period_3) {
    counter_3 = counter3->getCounter();
    USBSerial.print("counter3 reading:.............: ");
    USBSerial.println(counter_3);
    if (counter_3 > (tempCounter_3 + level_alarm_3)) {
      Supla::Notification::Send(-1, dev_name_message_3, message_3);
    }
    tempCounter_3 = counter_3;
    prev_min_millis_3=millis();
  }
  if ((prev_hour_millis_3 - millis() > time_period_night_3) && night_3) {
    counter_3 = counter3->getCounter();
    USBSerial.print("counter3 reading:.............: ");
    USBSerial.println(counter_3);
    if (counter_3 > (tempCounterNight_3 + level_alarm_night_3)) {
      String n = "NIGHT3: ";
      String n_message = n + message_3;
      Supla::Notification::Send(-1, dev_name_message_3, n_message.c_str());
    }
    tempCounterNight_3 = counter_3;
    prev_hour_millis_3=millis();
  }
}
static bool isNight(int32_t tim, int32_t night_start, int32_t night_end){
    if(night_start < night_end){
      return tim >= night_start && tim < night_end;
    }
    else{
      return tim >= night_start || tim < night_end;
    }
}
void nightReady(){
  if((millis()-prev_minute_millis)> 30000){//sprawdzamy czas co 30s

     bool nn_1 = isNight(clock1.getHour(), night_h_start_1, night_h_stop_1);
     bool nn_2 = isNight(clock1.getHour(), night_h_start_2, night_h_stop_2);
     bool nn_3 = isNight(clock1.getHour(), night_h_start_3, night_h_stop_3);
  
  if (!night_1 && nn_1){
    tempCounterNight_1 = counter_1;  
  }   
  if (!night_2 && nn_2){
    tempCounterNight_2 = counter_2;  
   }  
  if (!night_3 && nn_3){
   tempCounterNight_3 = counter_3;  
  }  
     night_1 = nn_1;
     night_2 = nn_2;
     night_3 = nn_3;
     prev_minute_millis=millis(); 
  }
}
void inputs1(){
  new Supla::Html::CustomParameter(PARAM26, "Counter1Pin:");
  new Supla::Html::CustomParameter(PARAM29, "ValveOpenPin :");
  new Supla::Html::CustomParameter(PARAM30, "ValveClosePin :");
   new Supla::Html::CustomParameter(PARAM1, "Night_h_start :1");
   new Supla::Html::CustomParameter(PARAM2, "Night_h_end :1");
   new Supla::Html::CustomTextParameter(PARAM3, "DevName message :1", 25);
   new Supla::Html::CustomTextParameter(PARAM4, "Push message :1", 25);
   new Supla::Html::CustomParameter(PARAM5, "Level alarm :1");
   new Supla::Html::CustomParameter(PARAM6, "Level alarm night :1");
   new Supla::Html::CustomParameter(PARAM7, "Time period(s) :1");
   new Supla::Html::CustomParameter(PARAM8, "Time period(s) night :1");
}
void inputs2(){
  new Supla::Html::CustomParameter(PARAM27, "Counter2Pin :");
  new Supla::Html::CustomParameter(PARAM9, "Night_h_start :2");
  new Supla::Html::CustomParameter(PARAM10, "Night_h_end :2");
  new Supla::Html::CustomTextParameter(PARAM11, "DevName message :2", 25);
  new Supla::Html::CustomTextParameter(PARAM12, "Push message :2", 25);
  new Supla::Html::CustomParameter(PARAM13, "Level alarm :2");
  new Supla::Html::CustomParameter(PARAM14, "Level alarm night :2");
  new Supla::Html::CustomParameter(PARAM15, "Time period(s) :2");
  new Supla::Html::CustomParameter(PARAM16, "Time period night(s) :2");
}
void inputs3(){
  new Supla::Html::CustomParameter(PARAM28, "Counter3Pin :");
  new Supla::Html::CustomParameter(PARAM17, "Night_h_start :3");
  new Supla::Html::CustomParameter(PARAM18, "Night_h_end :3");
  new Supla::Html::CustomTextParameter(PARAM19, "DevName message :3", 25);
  new Supla::Html::CustomTextParameter(PARAM20, "Push message :3", 25);
  new Supla::Html::CustomParameter(PARAM21, "Level alarm :3");
  new Supla::Html::CustomParameter(PARAM22, "Level alarm night :3");
  new Supla::Html::CustomParameter(PARAM23, "Time period(s) :3");
  new Supla::Html::CustomParameter(PARAM24, "Time period night(s) :3");
}
void stor1(){
if (Supla::Storage::ConfigInstance()->getInt32(PARAM26, &counter1Pin)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM26, counter1Pin);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM26);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM29, &valveOpenPin)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM29, valveOpenPin);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM29);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM30, &valveClosePin)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM30, valveClosePin);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM30);
  }
  ////////////////////
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM1, &night_h_start_1)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM1, night_h_start_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM1);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM2, &night_h_stop_1)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM2, night_h_stop_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM2);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM3, dev_name_message_1, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM3, dev_name_message_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM3);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM4, message_1, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM4, message_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM4);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM5, &level_alarm_1)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM5, level_alarm_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM5);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM6, &level_alarm_night_1)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM6, level_alarm_night_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM6);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM7, &time_period_1)) {
    time_period_1 = time_period_1*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM7, time_period_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM7);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM8, &time_period_night_1)) {
    time_period_night_1 = time_period_night_1*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM8, time_period_night_1);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM8);
  }
}
void stor2(){
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM27, &counter2Pin)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM27, counter2Pin);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM27);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM9, &night_h_start_2)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM9, night_h_start_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM9);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM10, &night_h_stop_2)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM10, night_h_stop_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM10);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM11, dev_name_message_2, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM11, dev_name_message_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM11);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM12, message_2, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM12, message_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM12);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM13, &level_alarm_2)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM13, level_alarm_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM13);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM14, &level_alarm_night_2)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM14, level_alarm_night_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM14);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM15, &time_period_2)) {
    time_period_2 = time_period_2*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM15, time_period_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM15);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM16, &time_period_night_2)) {
    time_period_night_2 = time_period_night_2*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM16, time_period_night_2);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM16);
  }
}
void stor3(){
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM28, &counter3Pin)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM28, counter3Pin);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM28);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM17, &night_h_start_3)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM17, night_h_start_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM17);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM18, &night_h_stop_3)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM18, night_h_stop_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM18);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM19, dev_name_message_3, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM19, dev_name_message_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM19);
  }
  if (Supla::Storage::ConfigInstance()->getString(PARAM20, message_3, 200)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %s", PARAM20, message_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM20);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM21, &level_alarm_3)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM21, level_alarm_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM21);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM22, &level_alarm_night_3)) {
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM22, level_alarm_night_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM22);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM23, &time_period_3)) {
    time_period_3 = time_period_3*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM23, time_period_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM23);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM24, &time_period_night_3)) {
    time_period_night_3 = time_period_night_3*1000;
    SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM24, time_period_night_3);
  } else {
    SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM24);
  }
}
