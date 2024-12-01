#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <LittleFS.h> // ใช้ LittleFS
#include "Audio.h"    // ไลบรารีสำหรับเล่นเสียง
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
//#include "esp_camera.h"
#include <WebSocketsServer.h>


// #include <cpr/cpr.h>

HardwareSerial SerialPort(1);

const char* ssid = "Yoghurt";
const char* password = "12345ABC";

// Airtable Base ID และ Table Name
const String base_id = "appziW4xMaSecmrGV"; // Base ID ของคุณ
const String table_name = "Member"; // ชื่อ Table ของคุณ

// Personal Access Token
const String token = "patvGoOsz7DuihEfO"; // Token ของคุณ
const char* serverUrl = "http://192.168.1.100/api/data";


AsyncWebServer server(80);
WebSocketsServer webSocket(81);
Audio audio;

#define I2S_DOUT 27 // ขา DIN ของ MAX98357A
#define I2S_BCLK 26 // ขา BCLK ของ MAX98357A
#define I2S_LRC 25  // ขา LRC ของ MAX98357A

#define I2S_DOUT_2 18
#define I2S_BCLK_2 19
#define I2S_LRC_2 21
#define I2S_NUM_NEW I2S_NUM_1

bool hasPlayed = false; // สถานะการเล่นเสียง


   void setup() {
    Serial.begin(115200); // Debugging
    Serial1.begin(115200, SERIAL_8N1, 17, 16); // UART2 RX=GPIO16, TX=GPIO17
   
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());    
    //setupCamera();
    // เริ่มต้น LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return;
    }

    // ตั้งค่า MAX98357A
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // ระดับเสียง 0-21

// void setupCamera() {
//     // กำหนดค่ากล้อง (เหมือนตัวอย่างก่อนหน้า)
//     camera_config_t config;
//     // ... ตั้งค่ากล้องตามที่เคยเขียนไว้ในตัวอย่าง
//     esp_camera_init(&config);
// }

// void captureAndUploadPhoto() {
//   // ถ่ายภาพ
//   camera_fb_t* fb = esp_camera_fb_get();
//   if (!fb) {
//     Serial.println("Camera capture failed");
//     return;
//   }

//   // ส่งภาพไปยัง Google Drive ผ่าน Web App
//   HTTPClient http;
//   http.begin(webAppURL);
//   http.addHeader("Content-Type", "image/jpeg"); // กำหนด Content-Type เป็น JPEG

//   int httpResponseCode = http.POST(fb->buf, fb->len);

//   if (httpResponseCode > 0) {
//     String response = http.getString();
//     Serial.println("Photo URL: " + response);
//   } else {
//     Serial.printf("Upload failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
//   }

//   http.end();
//   esp_camera_fb_return(fb);
// }


    // String url = "https://api.airtable.com/v0/" + base_id + "/" + table_name;

    // // สร้าง HTTPClient object
    // HTTPClient http;

    // // เริ่มต้นการเชื่อมต่อ
    // http.begin(url);

    // // เพิ่ม Header สำหรับ Authorization โดยใช้ Bearer Token
    // http.addHeader("Authorization", "Bearer " + token);
    
    // // ส่ง HTTP GET Request
    // int httpCode = http.GET();
    
    // // ตรวจสอบผลลัพธ์จาก Request
    // if (httpCode > 0) {                                                    cx
    //     // แสดงผลลัพธ์ที่ได้รับจาก Airtable
    //     String payload = http.getString();
    //     Serial.println("Response from Airtable:");
    //     Serial.println(payload);
    // } else {
    //     Serial.println("Error on HTTP request");
    // }

    // // ปิดการเชื่อมต่อ HTTP
    // http.end();

    // สร้างเซิร์ฟเวอร์สำหรับหน้าเว็บ
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // เริ่มเซิร์ฟเวอร์
    server.begin();

    analogReadResolution(12); // ตั้งค่าความละเอียด ADC 12 บิต

    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        if (type == WStype_CONNECTED) {
            Serial.printf("Client [%u] connected.\n", num);
        } else if (type == WStype_DISCONNECTED) {
            Serial.printf("Client [%u] disconnected.\n", num);
        }
    });
    
}

void loop() {
    if (Serial1.available()) { // ตรวจสอบว่ามีข้อมูลเข้ามาหรือไม่
        String receivedData = "";
        while (Serial1.available()) { // อ่านข้อมูลทั้งหมดที่เข้ามา
            char c = Serial1.read();
            receivedData += c;
        }

        //Serial.println("Received: " + receivedData); // Debugging

        // แปลงข้อมูล JSON
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, receivedData);

        if (!error) {
            int buttonState = doc["button"]; // อ่านสถานะปุ่มกด
            String jsonData = "{\"button\":" + String(buttonState) + "}";
            // เช็คสถานะปุ่มกด
            // Send data to the backend API

            if (buttonState == 0 && !hasPlayed) { // กดปุ่ม และยังไม่ได้เล่นเสียง
                
                Serial.println("Playing doorbell sound...");
                audio.connecttoFS(LittleFS, "doorbell.mp3"); // เล่นไฟล์เสียง doorbell.mp3 จาก LittleFS
                hasPlayed = true;
            } else if (buttonState == 1) { // รีเซ็ตสถานะเมื่อปล่อยปุ่ม
                hasPlayed = false;
            }

        } else {
            Serial.println("Failed to parse JSON");
        }

    }
    
    webSocket.loop();
    audio.loop(); // ต้องเรียกใช้ใน loop() สำหรับเล่นเสียง
}