#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define USE_I2C_LCD 1
#ifdef USE_I2C_LCD
//#define LCD_SCL_PIN SCL
//#define LCD_SDA_PIN SDA
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setUpLcd(){
 Wire.begin();
 lcd.begin();
 lcd.backlight();
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Starting");
20
 lcd.setCursor(0,1);
 lcd.print("wait.");
}
#endif //USE_I2C_LCD
///*******************************************************///
///*******************************************************///
/// SERVER CODE START FROM HERE ///
///*******************************************************///
///*******************************************************///
#define LOLIN_LED D4
#define PRODUCTION 1
21
//#define DEBUG_SERVER 1
String HOME = "/"; 
/* Global vareiables are here */
unsigned long timestamp = 0;
/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);
//@auto generated code
//QR Format
//WIFI:S:MySSID;T:WPA;P:MyPassW0rd;;
//hotspot config
const char* hotspot_name = "iota0521-breaking";
const char* hotspot_password = "iota0521";
struct{
String message;
String message_class = "hide";
String breakstate;
String breakstate_class = "success";
String fluid;
22
String fluid_class = "success";
} dataPacket;
struct{
const int BTN_NONE = -1;
} btnAction;
int userBtnAction = btnAction.BTN_NONE;
#ifdef PRODUCTION 
String getDataJson(){
return 
"{\"message\":\""+dataPacket.message+"\",\"message_class\":\""+dataPa
cket.message_class+"\", "
"\"breakstate\":\""+dataPacket.breakstate+"\",\"breakstate_class\":\
""+dataPacket.breakstate_class+"\", "
"\"fluid\":\""+dataPacket.fluid+"\",\"fluid_class\":\""+dataPacket.fl
uid_class+"\"}";
}
#endif
void handel_UserAction(){
for (uint8_t i = 0; i < server.args(); i++) {
}
server.send(200, "text/json", getDataJson());
}
23
#ifdef DEBUG_SERVER
String getTestClass(){
int r = random(0,4);
switch (r){
case 0: return "primary";
case 1: return "secondary";
case 2: return "success";
case 3: return "danger";
case 4: return "warning";
}
}
String getDataJson(){
return 
"{\"message\":\""+String(random(10,99))+"\",\"message_class\":\""+getT
estClass()+"\", "
"\"breakstate\":\""+String(random(10,99))+"\",\"breakstate_class\":
\""+getTestClass()+"\", "
"\"fluid\":\""+String(random(10,99))+"\",\"fluid_class\":\""+getTes
tClass()+"\"}";
}
#endif
// auto generated code
void forwardTo(String location){
 server.sendHeader("Location", location, true);
24
 server.send( 302, "text/plain", "");
}
void handle_Home() {
 server.send( 200, "text/html", getTemplate());
}
void handle_DataRequest(){ 
 server.send( 200, "text/json", getDataJson());
}
void handle_NotFound(){
 forwardTo(HOME);
}
void setUpServer(){
 delay(500);
 WiFi.softAP(hotspot_name, hotspot_password);
 WiFi.softAPConfig(local_ip, gateway, subnet);
 
 server.begin();
 delay(300);
 Serial.println("server started.");
}
25
#define FLUIDE_PIN A0
#define FLUIDL_PIN D6
#define FAIL_PIN D7
#define RELAY_PIN D5
#define BUZZER_PIN D8
void setUpGPIO(void);
void setup() {
 delay(500);
 Serial.begin(115200);
 Serial.println("\n\nstartng...");
 setUpServer();
 setUpGPIO();
#ifdef USE_I2C_LCD
 setUpLcd();
#endif //USE_I2C_LCD
 timestamp = millis();
}
void setUpGPIO(){
pinMode(LOLIN_LED,OUTPUT);
pinMode(FLUIDE_PIN,INPUT_PULLUP);
 pinMode(FLUIDL_PIN, INPUT_PULLUP);
26
}
uint32_t led_time_stamp = 0;
void blinkLed(int mil){
 if(led_time_stamp+mil < millis()){
 led_time_stamp = millis();
 if(digitalRead(LOLIN_LED) == 0){
 digitalWrite(LOLIN_LED, HIGH);
 }
 else{
 digitalWrite(LOLIN_LED, LOW);
 } 
 }
}
uint32_t lcd_update_time = 0;
uint32_t vantilation_timer = 0;
int vant_open = 0;
void loop() {
 server.handleClient();
 blinkLed(500);
 if(userBtnAction != btnAction.BTN_NONE){
 userBtnAction = btnAction.BTN_NONE;
 Serial.println("Button clicked");
 }
27
 
 if(lcd_update_time+500 < millis()){
 lcd_update_time = millis();
 int fail = digitalRead(FAIL_PIN);
 int bzr = 0;
 int relay = 1;
 
 lcd.clear();
 lcd.setCursor(0,0);
 
 if(fail == 1){
 dataPacket.breakstate = "Break Line Failed!";
 dataPacket.breakstate_class = "danger";
 lcd.print("Break Line FAIL");
 bzr = 1;
 relay = 0;
 }
 else{
 dataPacket.breakstate = "Break Line OK!";
 lcd.print("Break Line OK");
 dataPacket.breakstate_class = "success";
 }
28
 lcd.setCursor(0,1);
 if(analogRead(FLUIDE_PIN) >= 511){
 //empty
 dataPacket.fluid = "Break Fluid Empty!";
 dataPacket.fluid_class = "danger";
 bzr = 1;
 relay = 0;
 lcd.print("Fluid Empty");
 }
 else if(digitalRead(FLUIDL_PIN) == 1){
 // low
 dataPacket.fluid = "Break Fluid Low!";
 dataPacket.fluid_class = "warning";
 lcd.print("Fluid Low");
 }
 else{
 // ok
 dataPacket.fluid = "Break Fluid OK!";
 dataPacket.fluid_class = "success";
 lcd.print("Fluid OK");
 }
 
 if(bzr==1){
 digitalWrite(BUZZER_PIN, HIGH);
 }
 else{
 digitalWrite(BUZZER_PIN, LOW);
 }
29
 if(relay){
 digitalWrite(RELAY_PIN, HIGH);
 }
 else{
 digitalWrite(RELAY_PIN, LOW);
 }
 }
 
}
String getTemplate(){
return "<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<style>"
"body{\n"
"background-color: #F1FCFF;\n"
"padding:0px;\n"
"margin:0px;\n"
"text-align: center;\n"
30
"}\n"
"header{\n"
"height:35px;\n"
"padding:10px;\n"
"text-align:left;\n"
"display: flex;\n"
"background-color: #0093E9;\n"
"position:fixed;\n"
"width:100%;\n"
"z-index:100;\n"
"top:0;\n"
"}\n"
"footer{\n"
"padding:20px;\n"
"}\n"
"\n"
"form{\n"
"margin:15px auto 0px auto;\n"
"max-width:90%;\n"
"background-color: #AAAAAA;\n"
"padding: 15px 0 15px 0;\n"
"border-radius: 5px;\n"
"}\n"
"\n"
"button{\n"
"margin:8px auto 0px auto;\n"
"width:90%;\n"
"background-color: #AAAAAA;\n"
"padding: 10px 0 10px 0;\n"
31
"border-radius: 5px;\n"
"font-size: 24px;\n"
"font-weight: bold;\n"
"color:white;\n"
"border: none;\n"
"}\n"
"button:active {\n"
"width:89%;\n"
"padding: 10px 0 10px 0;\n"
"color:black;\n"
"}\n"
"input{\n"
"margin:8px auto 0px auto;\n"
"width:90%;\n"
"padding: 10px 0 10px 0;\n"
"border-radius: 5px;\n"
"font-size: 22px;\n"
"color:black;\n"
"border: none;\n"
"}\n"
"\n"
"label{\n"
"margin:15px auto 0px auto;\n"
"width:90%;\n"
"padding: 0px 0 0px 0;\n"
"font-size: 22px;\n"
"display:block;\n"
"}\n"
"\n"
32
".radio-group{\n"
"margin:15px auto 15px auto;\n"
"font-size: 24px;\n"
"width:100%;\n"
"display:flex;\n"
"flex-direction: row;\n"
"text-align:left;\n"
"}\n"
".radio-label{\n"
"margin:0px;\n"
"padding:0px;\n"
"}\n"
".radio{\n"
"width:32px;\n"
"margin:0px 10px 0px 15px;\n"
"}\n"
"\n"
".content{\n"
"margin-top:70px;\n"
"}\n"
".connection{\n"
"margin-left:20px;\n"
"color: white;\n"
"}\n"
".online{\n"
"margin: 8px 0 0 -8px;\n"
"font-size: 16px;\n"
"color:white;\n"
"}\n"
33
".card{\n"
"margin:15px auto 0px auto;\n"
"max-width:90%;\n"
"padding: 15px 0 15px 0;\n"
"border-radius: 5px;\n"
"}\n"
".primary{\n"
"background-color: #8BC6EC;\n"
"visibility: visible;\n"
"}\n"
".secondary{\n"
"background-color: #AAAAAA;\n"
"visibility: visible;\n"
"}\n"
".success{\n"
"background-color: #82c063;\n"
"visibility: visible;\n"
"}\n"
".danger{\n"
"background-color: #F76666;\n"
"visibility: visible;\n"
"}\n"
".warning{\n"
"background-color: #E3D377;\n"
"visibility: visible;\n"
"}\n"
".hide{\n"
"visibility: hidden;\n"
"}\n"
34
"@media only screen and (min-width: 500px) {\n"
".card {\n"
"max-width:400px;\n"
"}\n"
"button{\n"
"max-width:400px;\n"
"}\n"
"form{\n"
"max-width:400px;\n"
"}\n"
"label{\n"
"max-width:400px;\n"
"}\n"
"}\n"
"\n"
"h1 {\n"
"margin: 2px;\n"
"color: white;\n"
"}\n"
"h2 {\n"
"margin: 2px;\n"
"color: black;\n"
"}\n"
"</style>\n"
"<meta charset='utf-8'>\n"
"<meta http-equiv='X-UA-Compatible' content='IE=edge'>\n"
"<title>Iot</title>\n"
"<meta name='viewport' content='width=device-width, initialscale=1'>\n"
35
"<link rel='stylesheet' type='text/css' media='screen' 
href='main.css'>\n"
"\n"
"</head>\n"
"<body onload=\"liveDataAjax()\">\n"
"<header>\n"
"<span class=\"connection\" id=\"connected\">\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" 
height=\"32\" fill=\"currentColor\" class=\"connected\" viewBox=\"0 0 
16 16\">\n"
"<path d=\"M15.384 6.115a.485.485 0 0 0-.047-.736A12.444 
12.444 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 
0-.048.736.518.518 0 0 0 .668.05A11.448 11.448 0 0 1 8 4c2.507 0 
4.827.802 6.716 2.164.205.148.49.13.668-.049z\"/>\n"
"<path d=\"M13.229 8.271a.482.482 0 0 0-.063-.745A9.455 9.455 
0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 
0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 
1.336c.206.132.48.108.653-.065zm-2.183 
2.183c.226-.226.185-.605-.1-.75A6.473 6.473 0 0 0 8 9c-1.06 0-
2.062.254-
2.946.704-.285.145-.326.524-.1.75l.015.015c.16.16.407.19.611.09A5.47
8 5.478 0 0 1 8 10c.868 0 1.69.201 
2.42.56.203.1.45.07.61-.091l.016-.015zM9.06 
12.44c.196-.196.198-.52-.04-.66A1.99 1.99 0 0 0 8 11.5a1.99 1.99 0 0 0-
1.02.28c-.238.14-.236.464-.04.66l.706.706a.5.5 0 0 0 .707 
0l.707-.707z\"/>\n"
"</svg>\n"
"</span>\n"
"<span class=\"connection\" id=\"disconnected\">\n"
36
"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" 
height=\"32\" fill=\"currentColor\" class=\"connected\" viewBox=\"0 0 
16 16\">\n"
"<path d=\"M10.706 3.294A12.545 12.545 0 0 0 8 3C5.259 3 
2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.518.518 0 0 
0 .668.05A11.448 11.448 0 0 1 8 4c.63 0 1.249.05 
1.852.148l.854-.854zM8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 
0-.063.745.525.525 0 0 0 .652.065 8.448 8.448 0 0 1 3.51-1.27L8 
6zm2.596 1.404.785-.785c.63.24 1.227.545 1.785.907a.482.482 0 0 
1 .063.745.525.525 0 0 1-.652.065 8.462 8.462 0 0 0-1.98-.932zM8 
10l.933-.933a6.455 6.455 0 0 1 
2.013.637c.285.145.326.524.1.75l-.015.015a.532.532 0 0 
1-.611.09A5.478 5.478 0 0 0 8 10zm4.905-4.905.747-.747c.59.3 
1.153.645 1.685 1.03a.485.485 0 0 1 .047.737.518.518 0 0 1-.668.05 
11.493 11.493 0 0 0-1.811-1.07zM9.02 
11.78c.238.14.236.464.04.66l-.707.706a.5.5 0 0 1-.707 
0l-.707-.707c-.195-.195-.197-.518.04-.66A1.99 1.99 0 0 1 8 11.5c.374 
0 .723.102 1.021.28zm4.355-9.905a.53.53 0 0 1 .75.75l-10.75 
10.75a.53.53 0 0 1-.75-.75l10.75-10.75z\"/>\n"
"</svg>\n"
"</span>\n"
"<span class=\"connection\">\n"
"<p class=\"online\" id=\"online\" >Online</p>\n"
"</span>\n"
"</header>\n"
"\n"
"<div class=\"content\">\n"
"<div class=\"card hide\" id=\"message\">\n"
"<span></span>\n"
37
"<h1></h1>\n"
"</div>\n"
"<div class=\"card primary\" id=\"breakstate\">\n"
"<span>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" 
height=\"48\" fill=\"white\" class=\"bi bi-water\" viewBox=\"0 0 16 
16\">\n"
"<path fill-rule=\"evenodd\" d=\"M3.5 6a.5.5 0 0 0-.5.5v8a.5.5 0 0 
0 .5.5h9a.5.5 0 0 0 .5-.5v-8a.5.5 0 0 0-.5-.5h-2a.5.5 0 0 1 0-1h2A1.5 1.5 0 
0 1 14 6.5v8a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 14.5v-8A1.5 1.5 0 0 
1 3.5 5h2a.5.5 0 0 1 0 1h-2z\"/>\n"
"<path fill-rule=\"evenodd\" d=\"M7.646 11.854a.5.5 0 0 0 .708 
0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293V1.5a.5.5 0 0 0-1 0v8.793L5.354 
8.146a.5.5 0 1 0-.708.708l3 3z\"/>\n"
"</svg>\n"
"</span>\n"
"<h1>Not Detected</h1>\n"
"</div>\n"
"<div class=\"card primary\" id=\"fluid\">\n"
"<span>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" 
height=\"48\" fill=\"white\" class=\"bi bi-thermometer-half\" 
viewBox=\"0 0 16 16\">\n"
"<path d=\"M13.5 0a.5.5 0 0 0 0 1H15v2.75h-.5a.5.5 0 0 0 0 
1h.5V7.5h-1.5a.5.5 0 0 0 0 1H15v2.75h-.5a.5.5 0 0 0 0 1h.5V15h1.5a.5.5 0 0 0 0 1h2a.5.5 0 0 0 .5-.5V.5a.5.5 0 0 0-.5-.5h-2zM7 
1.5l.364-.343a.5.5 0 0 0-.728 
0l-.002.002-.006.007-.022.023-.08.088a28.458 28.458 0 0 0-1.274 
1.517c-.769.983-1.714 2.325-2.385 3.727C2.368 7.564 2 8.682 2 9.733 2 
38
12.614 4.212 15 7 15s5-2.386 5-5.267c0-1.05-.368-2.169-.867-
3.212-.671-1.402-1.616-2.744-2.385-3.727a28.458 28.458 0 0 0-1.354-
1.605l-.022-.023-.006-.007-.002-.001L7 1.5zm0 0-.364-.343L7 
1.5zm-.016.766L7 2.247l.016.019c.24.274.572.667.944 1.144.611.781 
1.32 1.776 1.901 2.827H4.14c.58-1.051 1.29-2.046 1.9-
2.827.373-.477.706-.87.945-1.144zM3 9.733c0-.755.244-1.612.638-
2.496h6.724c.395.884.638 1.741.638 2.496C11 12.117 9.182 14 7 14s-4-
1.883-4-4.267z\"/>\n"
"</svg>\n"
"</span>\n"
"<h1>30 &deg; C</h1>\n"
"</div>\n"
"\n"
"\n"
"\n"
"</div>\n"
"<footer>\n"
"\n"
"</footer>\n"
"\n"
"<script>"
"var DRT = 500;\n"
"function updateCSSClass(element, css){\n"
" if(css != 'primary')\n"
" element.classList.remove('primary');\n"
" if(css != 'secondary')\n"
" element.classList.remove('secondary');\n"
" if(css != 'success')\n"
" element.classList.remove('success');\n"
39
" if(css != 'danger')\n"
" element.classList.remove('danger');\n"
" if(css != 'warning')\n"
" element.classList.remove('warning');\n"
" if(css != 'hide')\n"
" element.classList.remove('hide');\n"
" element.classList.add(css);\n"
"}\n"
"\n"
"function updateData(data){\n"
"\tdocument.getElementById(\"message\").children[1].innerHTML 
= \"\"+data.message+\"\";\n"
"\tupdateCSSClass(document.getElementById(\"message\"), 
data.message_class);\n"
"\tdocument.getElementById(\"breakstate\").children[1].innerHTM
L = \"\"+data.breakstate+\"\";\n"
"\tupdateCSSClass(document.getElementById(\"breakstate\"), 
data.breakstate_class);\n"
"\tdocument.getElementById(\"fluid\").children[1].innerHTML = 
\"\"+data.fluid+\"\";\n"
"\tupdateCSSClass(document.getElementById(\"fluid\"), 
data.fluid_class);\n"
"}\n"
"\n"
"function getCommand(btn_id, value){\n"
" \n"
"}\n"
"\n"
"function onClickBtn(btn_id){\n"
40
"\tvar val = document.getElementById(btn_id).innerHTML;\n"
"\tvar cmd = getCommand(btn_id,val);\n"
" console.log(cmd)\n"
"\tsendButtonClick('/act?'+btn_id+'='+cmd)\n"
"}\n"
"\n"
"\n"
"\n"
"function updateNetwork(connected){\n"
" if(connected){\n"
" document.getElementById('disconnected').style.display = 
'none';\n"
" document.getElementById('connected').style.display = 
'block';\n"
" document.getElementById('online').innerHTML = 
'Online';\n"
" }\n"
" else{\n"
" document.getElementById('connected').style.display = 
'none';\n"
" document.getElementById('disconnected').style.display = 
'block';\n"
" document.getElementById('online').innerHTML = 
'Offline';\n"
" }\n"
"}\n"
"\n"
"\n"
"function sendButtonClick(url){\n"
41
"\t\n"
" const xhr = new XMLHttpRequest();\n"
" xhr.open('GET', url, true);\n"
" xhr.onload = () => {\n"
" if(xhr.readyState === XMLHttpRequest.DONE && 
xhr.status === 200) {\n"
" var data= JSON.parse(xhr.responseText);\n"
" updateData(data);\n"
" updateNetwork(true);\n"
" }\n"
" }\n"
" xhr.onerror = function() {\n"
" updateNetwork(false);\n"
" };\n"
" xhr.send();\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"var netcount = 0;\n"
"function reconnect(){\n"
" if(netcount == 0){\n"
" console.log(\"Retrying\");\n"
" document.getElementById('online').innerHTML = 
'Retrying..';\n"
" setTimeout(liveDataAjax,1000);\n"
" return\n"
" }\n"
42
" netcount -= 1;\n"
" console.log(\"count\",netcount);\n"
" document.getElementById('online').innerHTML = 'Offline 
('+netcount+')';\n"
" setTimeout(reconnect, 1000);\n"
"}\n"
"function liveDataAjax(){\n"
" const xhr = new XMLHttpRequest();\n"
" xhr.open('GET', '/data.json', true);\n"
" xhr.onload = () => {\n"
" if(xhr.readyState === XMLHttpRequest.DONE && 
xhr.status === 200) {\n"
" var data= JSON.parse(xhr.responseText);\n"
" updateData(data);\n"
" updateNetwork(true);\n"
" setTimeout(liveDataAjax, DRT);\n"
" }\n"
" else if (xhr.readyState === XMLHttpRequest.DONE){\n"
" updateNetwork(false);\n"
" netcount = 5;\n"
" reconnect();\n"
" }\n"
" };\n"
" xhr.onerror = function() {\n"
" updateNetwork(false);\n"
" netcount = 5;\n"
" reconnect();\n"
" };\n"
" xhr.send();\t\n"
43
"}\n"
"\n"
"</script>\n"
"</body>\n"
"</html>"