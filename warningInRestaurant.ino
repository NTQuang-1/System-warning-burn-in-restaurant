#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLxpKZiDRL"
#define BLYNK_TEMPLATE_NAME "Thiet Ke He Thong IoT"
#define BLYNK_AUTH_TOKEN "0ZnVljg-NJIQ4gZx3OpvOQ5j0sYC513C"
#define RS 3
#define EN 15
#define D4 13
#define D5 14
#define D6 2
#define D7 0
#define buzzer 5 //D1 còi và đèn cảnh báo
#define ledMode 12 //D6 led hiển thị chế độ hoạt động (0 là điều khiển,1 là tự động)
#define servopin 4 // D2 kết nối servo quay đóng mở cửa
#define quat 16
// vitual port for Automation
#define LED V0 
#define Gas V1
#define SetWarningLimit V2
#define RUNMODE V3 //0 là điều khiển, 1 là tự động
#define WarningStatus V4
#define ControlDoor V5
// vitual port for Controller
#define ControlModeWarning V6
#define DisplayLCD V7
// end definition

/*  include library */
// begin Library
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <Servo.h>
// end Library

/* connected networks */
// begin Network
const char  ssid[] = "ABC";
const char  pass[] = "uabcyekh";
//end Network

/* info serve MQTT */
// begin IsMQTT
#define mqtt_server "broker.hivemq.com"
const uint16_t mqtt_port = 1883;
unsigned long t;
// end IsMQTT

/* private variable */
// begin code PV
char GetMQ2Value[4];
char MQ2ValueChar[4];
int MQ2Value = 0;
int WarningLimit = 400; 
int RunMode = 0;
int DoorState = 0;
int ControlModeWarningStatus = 0;
int count = 1;
// end code PV

/* constructor */
//begin constructor
WiFiClient espClient;
PubSubClient client(espClient);
Servo myservo;
BlynkTimer timer;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
//end constructor

/* Function interract with Servo MQTT */
// begin FIWS_MQTT
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("\nCó tin nhắn mới từ topic:");
  Serial.println(topic);
  for (int i = 0; i < length; i++)
  { 
    MQ2ValueChar[i] += (char)payload[i];
  }
  Serial.print("Dữ liệu nhận được từ Broker : "); Serial.println(MQ2ValueChar);
}

void reconnect() 
{
  while (!client.connected()) // Chờ tới khi kết nối
  {
    // Thực hiện kết nối với mqtt user và pass
    if (client.connect("ESP8266_id1","ESP_offline",0,0,"ESP8266_id1_offline"))  //kết nối vào broker
    {
      Serial.println("Đã kết nối:");
      client.subscribe("MQ2_NHOM3_DT"); //đăng kí nhận dữ liệu từ topic Cảnh Báo Khí Gas
    }
    else 
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
    }
  }
}

void SendData(char dulieuguidi[])
{
  if (!client.connected())// Kiểm tra kết nối
    reconnect();
  client.loop();
  if(millis() - t > 1000) //nếu 500 mili giây trôi qua
  {
     t=millis();
     Serial.print("Gui tin nhan vao topic MQ2_NHOM3_DT ");
     client.publish("MQ2_NHOM3_DT", dulieuguidi); // gửi dữ liệu lên topic Cảnh Báo Khí Gas
  }
}
// end FIWS_MQTT

/* Function interract with Blynk */
// begin FIWB
BLYNK_CONNECTED()
{
  timer.setInterval(200L, LedStatusConnect);
  Blynk.syncVirtual(WarningStatus);
  Blynk.virtualWrite(SetWarningLimit, WarningLimit);
  Blynk.virtualWrite(Gas, 0);
  Blynk.virtualWrite(RUNMODE,0);
  Blynk.virtualWrite(ControlDoor, DoorState);
  Blynk.virtualWrite(ControlModeWarning, ControlModeWarningStatus);
}

BLYNK_WRITE(RUNMODE)
{
  if(param.asInt() == 1)
  {
    RunMode = 1;
  }else
  {
    RunMode = 0;
  }
}

BLYNK_WRITE(ControlDoor)
{
  if(RunMode == 0)
  {
    if(param.asInt() == 1)
    {
      OpenDoor();
      DoorState = 1;
    }else
    {
      CloseDoor();
      DoorState = 0;
    }
  }
}

BLYNK_WRITE(ControlModeWarning)
{
  if(RunMode==0)
  {
    if(param.asInt() == 1)
    {
      OnBuzz();
      Blynk.virtualWrite(WarningStatus,1);
      ControlModeWarningStatus =1;
    }else
    {
      OffBuzz();
      Blynk.virtualWrite(WarningStatus,0);
      ControlModeWarningStatus=0;
    }
  }
}

BLYNK_WRITE(SetWarningLimit)
{
  WarningLimit = param.asInt();
  Serial.print("\nNGƯỠNG GIỚI HẠN : ");
  Blynk.virtualWrite(DisplayLCD, WarningLimit);
  Serial.println(WarningLimit);
}
// end FIWB

/* code function user */
// begin FU
void LedStatusConnect()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    Blynk.virtualWrite(LED,1);
    delay(200);
    Blynk.virtualWrite(LED,0);
    delay(200);
  }
}
void connected_wifi() 
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(3);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void configPort()
{
  pinMode(servopin,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(quat,OUTPUT);
  pinMode(ledMode,OUTPUT);
  digitalWrite(buzzer,LOW); 
  digitalWrite(ledMode,LOW);
  myservo.attach(servopin);
}

void ModeControl()
{
  digitalWrite(ledMode,0);
  Blynk.virtualWrite(DisplayLCD, "CONTROL MODE");
  Blynk.virtualWrite(WarningStatus, ControlModeWarningStatus);
  Blynk.virtualWrite(ControlModeWarning, ControlModeWarningStatus);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CONTROL MODE");
  Serial.println("\nCONTROL MODE");
}

void ModeAuto()
{
  digitalWrite(ledMode,1);
  Serial.println("AUTO MODE");  
  Blynk.virtualWrite(WarningStatus, 0);
  // handle tx rx data MQTT
  String inputString="";
  inputString +=String(analogRead(A0));
  for(int i=0;i<4;i++)
  {
    GetMQ2Value[i] = inputString[i];
  }
  inputString = "";
  SendData(GetMQ2Value);
  for(int i=0;i<4;i++)
  {
    GetMQ2Value[i] = inputString[i];
  }

  for(int i=0;i<4;i++)
  {
    inputString += MQ2ValueChar[i];
  }
  MQ2Value=inputString.toInt();
  Serial.print("dữ liệu nhan duoc sau khi xu ly : ");
  Serial.println(MQ2Value);
  inputString = "";
  for(int i=0;i<4;i++)
  {
    MQ2ValueChar[i] = inputString[i];
  }
  // end handle
  Blynk.virtualWrite(Gas,MQ2Value); 

  if(MQ2Value >= WarningLimit)
  {
    digitalWrite(buzzer,1);
    Blynk.virtualWrite(WarningStatus,1);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("KHI GAS VUOT MUC !");
    Blynk.virtualWrite(DisplayLCD, "KHI GAS VUOT MUC");
    Serial.println("TURN ON WARNING !");
    ControlModeWarningStatus=1;
    digitalWrite(quat, 1);
    if(DoorState == 0){
      OpenDoor();
      delay(100);
      DoorState = 1;
    }
    Blynk.virtualWrite(ControlDoor, DoorState);
    Blynk.virtualWrite(ControlModeWarning, ControlModeWarningStatus);
  }else
  {
    digitalWrite(buzzer, 0);
    digitalWrite(quat, 0);
    ControlModeWarningStatus = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("AUTO MODE");
    lcd.setCursor(1,1);
    lcd.print("NORMAL");
    Serial.println("TURN OFF WARNING");
    Blynk.virtualWrite(WarningStatus, 0);
    Blynk.virtualWrite(ControlDoor, DoorState);
    Blynk.virtualWrite(DisplayLCD, "AUTO MODE");
    Blynk.virtualWrite(ControlModeWarning,ControlModeWarningStatus);
  }
}

void SelectMode()
{
  if(RunMode == 0)
  {
    ModeControl();
  }
  if(RunMode == 1)
  {
    ModeAuto();
  }
}

void CloseDoor()
{
  for (int pos = 0; pos <= 180; pos += 10)
  {
    DoorState = 1;
    myservo.write(pos);
  }
}

void OpenDoor()
{
  DoorState = 0;
  for (int pos = 180; pos >= 0; pos -= 10) 
  {
    myservo.write(pos);
  }
}

void OnBuzz()
{
  digitalWrite(buzzer, 1);
}

void OffBuzz()
{
  digitalWrite(buzzer, 0);
}
// end FU

/* Main */
// Begin Main
void setup()
{
  Serial.begin(115200);
  configPort();
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("NHOM 3 CANH BAO");
  lcd.setCursor(0,1);
  lcd.print("RO RI KHI GAS");
  delay(3000);
  lcd.clear();
  connected_wifi();
  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(callback);
  timer.setInterval(100L, SelectMode);
}                                                                               

void loop()
{
  Blynk.run();
  timer.run(); 
}
// end Main
