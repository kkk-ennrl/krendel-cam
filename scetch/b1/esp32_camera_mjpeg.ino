int led1                   = 2;
int led2                   = 14;
int led3                   = 16;
int LedOn                  = 150;
int LedOff                 = 0; 
int btn1                   = 15;
int btn2                   = 13;
int btn3                   = 12;
int Mode                   = 0;

#include "src/OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <Preferences.h>

#define PWDN_GPIO_NUM        32
#define RESET_GPIO_NUM       -1
#define XCLK_GPIO_NUM         0
#define SIOD_GPIO_NUM        26
#define SIOC_GPIO_NUM        27

#define Y9_GPIO_NUM          35
#define Y8_GPIO_NUM          34
#define Y7_GPIO_NUM          39
#define Y6_GPIO_NUM          36
#define Y5_GPIO_NUM          21
#define Y4_GPIO_NUM          19
#define Y3_GPIO_NUM          18
#define Y2_GPIO_NUM           5
#define VSYNC_GPIO_NUM       25
#define HREF_GPIO_NUM        23
#define PCLK_GPIO_NUM        22

OV2640 cam;
Preferences pref;
WebServer server(80);


#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();


const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);

void ScanNetworks(void) {
  String message = "";
  int n = WiFi.scanNetworks();

  if (n == 0) {
    message = "{\"status\":\"No Networks found!\"}";
  } else {
    message += "{";
    message += "\"count\":\"" + String(n) + "\", ";
    message += "\"networks\":[";
    for (int i = 0; i < n; i++) {
      message += "{";
      message += "\"id\":\"" + String(i) + "\", ";
      message += "\"name\":\"" + WiFi.SSID(i) + "\", ";
      message += "\"rssi\":\"" + String(WiFi.RSSI(i)) + "\", ";
      message += "\"encryption\":\"" + String(WiFi.encryptionType(i)) + "\"";
      if (i+1 != n) {
        message += "}, ";
      } else {
        message += "}";
      }
    }
    message += "]}";
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void TemperatureScan (void) {
  String message = "";
  message += "{\"cpuTemp\":\"" + String((temprature_sens_read() - 32) / 1.8) + "\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void ReConnectWiFi(void) {
  String message = "";
  String wifiName = server.arg("net");
  String wifiPass = server.arg("pass");
  int Ilp = random(160, 169);
  String ip = "192.168.1." + String(Ilp);

  Serial.println(wifiName);
  Serial.println(wifiPass);

  pref.putString("Name", wifiName);
  pref.putString("Pass", wifiPass);
  pref.putString("ip", ip);
  pref.putInt("Ilp", Ilp);

  message += "{\"ip\":\"" + ip + "\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
  delay(1000);
  ESP.restart();
}

void clean(void) {
  pref.clear();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void GenerateCamID(void) {
  String message = "";

  String camID   = String(random(11111111111111111111111111111111, 99999999999999999999999999999999));

  Serial.println(camID);

  char camIDBuf[32];

  camID.toCharArray(camIDBuf, 32);

  pref.putString("camID", camID);

  message = "{\"camID\":\"" +camID + "\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json",  message);
}

void handle_jpg_stream(void)
{
  char buf[128];
  int s;

  WiFiClient client = server.client();

  client.write(HEADER, hdrLen);
  client.write(BOUNDARY, bdrLen);

  while (true)
  {
    if (!client.connected()) break;
    cam.run();
    s = cam.getSize();
    client.write(CTNTTYPE, cntLen);
    sprintf( buf, "%d\r\n\r\n", s );
    client.write(buf, strlen(buf));
    client.write((char *)cam.getfb(), s);
    client.write(BOUNDARY, bdrLen);
  }
}

const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Access-Control-Allow-Origin: *\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

void handle_jpg(void)
{
  WiFiClient client = server.client();

  if (!client.connected()) return;
  cam.run();
  client.write(JHEADER, jhdLen);
  client.write((char *)cam.getfb(), cam.getSize());
}

void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text / plain", message);
}


String WiFiName, WiFiPassword, CameraID;
void setup()
{

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);

  Serial.begin(115200);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_XGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;
  cam.init(config);



  IPAddress ip;

  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional

  pref.begin("krendel-cam", false);
  

  String wifiName = pref.getString("Name", "");
  String wifiPass = pref.getString("Pass", "");
  String camID = pref.getString("camID", "");

  char WiFiName[256];
  char WiFiPass[256];

  wifiName.toCharArray(WiFiName, 256);
  wifiPass.toCharArray(WiFiPass, 256);
 
  Serial.println(String(WiFiName));
  Serial.println(String(WiFiPass));

  if (wifiName == "") {
    WiFi.mode(WIFI_AP);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress local_IP(192, 168, 4, 1);
    
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
    WiFi.softAP("KrendelCam", NULL);
    Mode = 1;
  } else {
    WiFi.mode(WIFI_STA);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress local_IP(192, 168, 1, pref.getInt("Ilp", 1));
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
    WiFi.begin(WiFiName, WiFiPass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi Connected!");
    Serial.println(WiFi.localIP());
    Mode = 2;
    
  }

  server.on("/temp", HTTP_GET, TemperatureScan);
  server.on("/scan", HTTP_GET, ScanNetworks);
  server.on("/connect", HTTP_GET, ReConnectWiFi);


  if (Mode == 1) {
    server.on("/camID", HTTP_GET, GenerateCamID);
  }
  if (Mode == 2) {
    server.on("/stream", HTTP_GET, handle_jpg_stream);
    server.on("/screen", HTTP_GET, handle_jpg);
    server.on("/clean", HTTP_GET, clean);
  }
  
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop()
{
  server.handleClient();
}
