#include <ESP32Servo.h>
#include <DHT.h>          //引入DHT库
#include <WiFi.h>   //安装esp8266arduino开发环境
#include <PubSubClient.h>  //安装PubSubClient库
#include <ArduinoJson.h>   //json  V5版本
#include <Wire.h>
#include "aliyun_mqtt.h"
#include <Adafruit_SSD1306.h>//引入OLED显示器模块
#define DHTPIN 4          //定义DHT11的引脚为4
#define DHTTYPE DHT11     //定义使用的传感器为DHT11
#define MQ6_PIN 2         // 定义MQ-6传感器引脚2
#define BUZZER_PIN 13     //定义蜂鸣器引脚为13
float temperature=23.2;  //读取温度值
float humidity=56.2;       // 读取湿度值
float mq6_value=0.0;//获取MQ-6传感器的值
float mq6_concentration=0.0;// 将传感器值转换为气体浓度
DHT dht(DHTPIN, DHTTYPE); //初始化DHT传感器

#define trigPin 25        //超声波传感器发送信号引脚25
#define echoPin 26        //超声波传感器接受信号引脚26
#define TrigPin 33        //超声波传感器发送信号引脚33
#define EchoPin 32        //超声波传感器接受信号引脚32

#define servoPin 27       //定义sg-90舵机引脚27
Servo myservo;// 创建舵机实例

#define OLED_RESET -1     //OLED显示器状态
#define SCREEN_WIDTH 128  //屏幕宽
#define SCREEN_HEIGHT 64  //屏幕长
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define WIFI_SSID        "DciPhone14"//替换自己的WIFI
#define WIFI_PASSWD      "66666666"//替换自己的WIFI

#define PRODUCT_KEY      "idxoCkRSquB" //替换自己的PRODUCT_KEY
#define DEVICE_NAME      "device" //替换自己的DEVICE_NAME
#define DEVICE_SECRET    "81a29dfe7d488eee9cfd01494edd0f23"//替换自己的DEVICE_SECRET

/*-----------------------------订阅消息的TOPIC -------------------------------------------*/
#define ALINK_TOPIC_PROP_SET      "/sys/idxoCkRSquB/device/thing/service/property/set" //这个就是我们接收消息的TOPIC        

/***************************这个是发布数据的TOPIC******************************************/
#define ALINK_TOPIC_PROP_POST      "/idxoCkRSquB/device/user/ESP32Duan" //这个是上传数据的TOPIC

unsigned long lastMs = 0;
int RunningState=0;
int HavePeople=0;//浴室内是否有人
WiFiClient   espClient;
PubSubClient mqttClient(espClient);

void setup()
{ 
    Serial.begin(115200);
    Serial.println("Demo Start");
    init_wifi(WIFI_SSID, WIFI_PASSWD);
    mqttClient.setCallback(mqtt_callback);
      pinMode(BUZZER_PIN, OUTPUT); //将蜂鸣器引脚设为输出
      digitalWrite(BUZZER_PIN, HIGH); //初始化为高电平，使蜂鸣器停止响
      dht.begin();                  //初始化DHT传感器
      pinMode(MQ6_PIN, INPUT); // 初始化mq-6传感器
        pinMode(trigPin, OUTPUT);//给超声波传感器设置初始化
        pinMode(echoPin, INPUT);
        pinMode(TrigPin, OUTPUT);//给超声波传感器设置初始化
        pinMode(EchoPin, INPUT);
      myservo.attach(servoPin);//初始化舵机
      

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // 0x3C是OLED显示器的I2C地址
  display.clearDisplay();

}

void loop()
{
  /*********************显示器初始化*********************/
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  temperature = dht.readTemperature();  //读取温度值
  humidity = dht.readHumidity();       // 读取湿度值
  mq6_value = analogRead(MQ6_PIN);//获取MQ-6传感器的值
  mq6_concentration = mq6_value * (3.3 / 1023.0);// 将传感器值转换为气体浓度
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");
  Serial.print("gas: ");
  Serial.println(mq6_concentration);
    display.print("temperature:");
   display.println(temperature);
   display.print("humidity:");
   display.println(humidity);
   display.print("gas:");
   display.println(mq6_concentration);
   display.print("OpenState:");
  display.display();

    // 读取门外超声波传感器距离
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration1 = pulseIn(echoPin, HIGH);
  int distance1 = duration1 / 58;//计算出超声波测距的数值
  Serial.print("Distance1: ");
  Serial.print(distance1);
  Serial.println(" cm");
    // 读取门内超声波传感器距离
   digitalWrite(TrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  long duration2 = pulseIn(EchoPin, HIGH);
  int distance2 = duration2 / 58;//计算出超声波测距的数值
   Serial.print("Distance2: ");
  Serial.print(distance2);
  Serial.println(" cm");
  delay(1000);
if(!HavePeople){
  if (distance1 < 20) {//distance1是外面那个传感器测的距离
   HavePeople+=1;//有人进浴室了 锁门
    Serial.println(HavePeople);
        Serial.println("there is some one in the room close the suo");
      myservo.write(0);
    display.println("open");
    display.display();
     delay(1000);

  } else{
     myservo.write(360);
      display.println("close");
      display.display();

  }

  } else{
    if (distance2 < 20) {//distance2是里面那个传感器测的距离
       HavePeople-=1;//有人出去了 解锁
           Serial.println("there is some one leave the room open the suo");
       Serial.println(HavePeople);
    myservo.write(0);
    display.println("open");
    display.display();
     delay(1000);
  }
  else{
     myservo.write(360);
      display.println("close");
      display.display();
  }


}
if(HavePeople){
 RunningState=1;
}else{
   RunningState=0;
}
   if (millis() - lastMs >= 5000){
    lastMs = millis();
    mqtt_check_connect();
    mqtt_interval_post(temperature,humidity,mq6_concentration,RunningState);
   }
    mqttClient.loop();  
      if (temperature > 40||humidity > 80) {       //当温度超过23摄氏度或者湿度高于80%时，蜂鸣器发出警报
    
      Serial.println("Temperature oover 23C ");
      digitalWrite(BUZZER_PIN, LOW);  //将蜂鸣器引脚设为低电平，使蜂鸣器响起
      delay(500);                    //等待500毫秒
      digitalWrite(BUZZER_PIN, HIGH); //将蜂鸣器引脚设为高电平，使蜂鸣器停止响
      delay(500);                    //等待500毫秒

  //温度或湿度过高控制风扇转动
      /*
      代码待补充
      */
  }
}

//回调函数-阿里云端数据发送过来之后，此函数进行解析
void mqtt_callback(char *topic, byte *payload, unsigned int length) //mqtt回调函数“byte *payload”这东西是个指针
{
    Serial.print("Message arrived [");
    Serial.print(topic);//传回来的topic
    Serial.print("] ");
    payload[length] = '\0';
    
    Serial.println((char *)payload);
    DynamicJsonDocument doc(100);
     deserializeJson(doc, payload);
     int numberInt = doc["RunningState"].as<int>();
  
      
           if(numberInt == 1){
          Serial.println("led on");
            
             myservo.write(360);
             delay(1000);
             myservo.write(0);
        
         }else if(numberInt == 0){
         Serial.println("led off");
       
         }
}
//订阅topic
void mqtt_check_connect(){                                        
    while (!mqttClient.connected())
    {
        while (connect_aliyun_mqtt(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
        {
            Serial.println("MQTT connect succeed!");
            mqttClient.subscribe(ALINK_TOPIC_PROP_SET);//这个就是引用开始定义的topic 订阅topic
            Serial.println("subscribe done");
           
        }
    }   
}

//上传数据到云端
void mqtt_interval_post(float temperature,float humidity,float mq6_concentration,int RunningState)
{
    char param[512];
    char jsonBuf[1024];
    
    sprintf(param, "{\"temperature\":%.2f,\"humidity\":%.2f,\"gas\":%.2f,\"RunningState\":%d}",temperature,humidity,mq6_concentration,RunningState);
    Serial.println(param);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, param); //这个是上传数据的topic,jsonBuf这个是上传的数据
}

//连接WiFi
void init_wifi(const char *ssid, const char *password)      
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi does not connect, try again ...");
        delay(500);
    }
    Serial.println("Wifi is connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
