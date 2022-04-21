#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <DHTesp.h>
  
#define LED 2
#define DHTpin 2

const char* ssid = "spynet-2.4g";
const char* password = "MW9pDbkK";

const char* fname = "/s.txt";

const long utcOffsetInSeconds = 3600 * 3;

int tonePin = 4;

DHTesp dht;
ESP8266WebServer server(25566);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html lang="ru">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <title>Умная теплица</title>
  <!--For offline ESP graphs see this tutorial https://circuits4you.com/2018/03/10/esp8266-jquery-and-ajax-web-server/ -->
  <script src = "https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js"></script>  
  <style>
  canvas{
    -moz-user-select: none;
    -webkit-user-select: none;
    -ms-user-select: none;
  }

  #dataTable {
    font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
    border-collapse: collapse;
    width: 100%;
  }

  #dataTable td, #dataTable th {
    border: 1px solid #ddd;
    padding: 8px;
  }

  #dataTable tr:nth-child(even){background-color: #f2f2f2;}

  #dataTable tr:hover {background-color: #ddd;}

  #dataTable th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: left;
    background-color: #4CAF50;
    color: white;
  }
  </style>
</head>

<body>
    
    <div style="text-align:center;"><b>Графики умной теплицы</b></div>
    <div class="chart-container" position: relative; height:350px; width:100%">
        <canvas id="Chart" width="400" height="400"></canvas>
    </div>
<div>
  <table id="dataTable">
    <tr><th>Время</th><th>Влажность почвы</th><th>Температура (&deg;C)</th><th>Влажность воздуха (%)</th></tr>
  </table>
</div>
<br>
<br>  

<script>
var ADCvalues = [];
var Tvalues = [];
var Hvalues = [];
var timeStamp = [];
function showGraph()
{
    var ctx = document.getElementById("Chart").getContext('2d');
    var Chart2 = new Chart(ctx, {
        type: 'line',
        data: {
            labels: timeStamp,  
            datasets: [{
                label: "Влажность почвы",
                fill: false,  //Try with true
                backgroundColor: 'rgba( 243,18, 156 , 1)', 
                borderColor: 'rgba( 243, 18, 156 , 1)', 
                data: ADCvalues,
            },{
                label: "Температура",
                fill: false,  //Try with true
                backgroundColor: 'rgba( 199, 0, 0 , 1)', 
                borderColor: 'rgba( 199, 0, 0 , 1)', 
                data: Tvalues,
            },
            {
                label: "Влажность воздуха",
                fill: false,  //Try with true
                backgroundColor: 'rgba(84, 58, 232 , 1)',
                borderColor: 'rgba(84, 58, 232 , 1)',
                data: Hvalues,
            }],
        },
        options: {
            title: {
                    display: false,
                    text: "ADC Voltage"
                },
            maintainAspectRatio: false,
            elements: {
            line: {
                    tension: 0.5 
                }
            },
            animation: {
              duration: 1000
            },
            scales: {
                    yAxes: [{
                        ticks: {
                            beginAtZero:true
                        }
                    }]
            }
        }
    });

}

window.onload = function() {
  console.log(new Date().toLocaleTimeString());
};

getData();

setInterval(function() {
  getRecent();
}, 60250); 

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
  var time = new Date().toLocaleTimeString();
  var txt = this.responseText;
  console.log(txt);
  var obj = JSON.parse(txt);
      for(i in obj.measurments){
        ADCvalues.push(obj.measurments[i].soil);
        Tvalues.push(obj.measurments[i].temperature);
        Hvalues.push(obj.measurments[i].humidity);
        timeStamp.push(obj.measurments[i].time);
        var table = document.getElementById("dataTable");
        var row = table.insertRow(1); //Add after headings
        var cell1 = row.insertCell(0);
        var cell2 = row.insertCell(1);
        var cell3 = row.insertCell(2);
        var cell4 = row.insertCell(3);
        cell1.innerHTML = obj.measurments[i].time;
        cell2.innerHTML = obj.measurments[i].soil;
        cell3.innerHTML = obj.measurments[i].temperature;
        cell4.innerHTML = obj.measurments[i].humidity;
      }
      showGraph();
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}

function getRecent() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
  var time = new Date().toLocaleTimeString();
  var txt = this.responseText;
  console.log(txt);
  var obj = JSON.parse(txt);
  var i = obj.measurments.length - 1;
  if(obj.measurments[i].time != timeStamp[timeStamp.length-1]){
    console.log("updating charts!");
        ADCvalues.push(obj.measurments[i].soil);
        Tvalues.push(obj.measurments[i].temperature);
        Hvalues.push(obj.measurments[i].humidity);
        timeStamp.push(obj.measurments[i].time);
        var table = document.getElementById("dataTable");
        var row = table.insertRow(1);
        var cell1 = row.insertCell(0);
        var cell2 = row.insertCell(1);
        var cell3 = row.insertCell(2);
        var cell4 = row.insertCell(3);
        cell1.innerHTML = obj.measurments[i].time;
        cell2.innerHTML = obj.measurments[i].soil;
        cell3.innerHTML = obj.measurments[i].temperature;
        cell4.innerHTML = obj.measurments[i].humidity;
      
      showGraph();
  }
    
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}   
</script>
<center>
<img src="https://static.wikia.nocookie.net/plantsvszombies/images/f/f8/2zfsax5_th.gif" height=72 width=64><br>
<a href="/time"><input type="button" value="Изменить время освещения"></a><br>
<a href="/watering"><input type="button" value="Принудительный полив"></a><br>
<a href="/clear"><input type="button" value="Очистить кэш"></a><br>
<a href="/reboot"><input type="button" value="Перезагрузить теплицу"></a><br></center>
</body>

</html>

)=====";




const char time_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>Установка времени</title>
</head>
<body>

<h2>Здесь можно установить время включения и выключения света</h2>

<form action="/time/action_page">
  Время включения:<br>
  <input type="text" name="ontime" value="">
  <br>
  Время выключения:<br>
  <input type="text" name="offtime" value="">
  <br><br>
  <input type="submit" value="Обновить">
</form> 

</body>
</html>
)=====";

void handleRoot() {
 String s = MAIN_page;
 server.send(200, "text/html", s);
}

void handleTime() {
 String s = time_page;
 server.send(200, "text/html", s);
}

void handleForm() {
 String ont = server.arg("ontime"); 
 String offt = server.arg("offtime"); 

 Serial.print("New on time:");
 Serial.println(ont.toInt());

 Serial.print("New off time:");
 Serial.println(offt.toInt());

 EEPROM.write(0, ont.toInt());
 EEPROM.write(1, offt.toInt());

 if (EEPROM.commit()) {
      Serial.println("EEPROM successfully committed");
    } else {
      Serial.println("ERROR! EEPROM commit failed");
    }

 String s = "<a href='/'> Go Back </a>";
 server.send(200, "text/html", s);
}

float humidity, temperature;

long int lastWatering = 0;

void doWatering(){
  if(millis() - lastWatering >= 60000){
    digitalWrite(15, HIGH);
    delay(1000);
    digitalWrite(15, LOW);
    server.send(200, "text/html", "watering done! <a href='/'> Go Back </a>");
    lastWatering = millis();
  }
  else{
    server.send(200, "text/html", "Too recent! Please wait. <a href='/'> Go Back </a>");
  }
}

void handleClear(){
  int a = 100-(analogRead(A0)/10);
  delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  String mins = String(timeClient.getMinutes());
  if(mins.length()<2) mins = "0"+mins;
  String hrs = String(timeClient.getHours());
  if(hrs.length()<2) hrs = "0"+hrs;
  File f = SPIFFS.open(fname, "w");
  f.print("{\"measurments\":[{\"time\":\""+hrs+":"+mins+"\",\"soil\":"+String(a)+",\"temperature\":"+String(temperature)+",\"humidity\":"+String(humidity)+"}");
  f.close();
  String s = "Cache cleared, <a href='/'> Go Back </a>";
  server.send(200, "text/html", s);
  
}

void handleADC() {
 String data;

 File f = SPIFFS.open(fname, "r");
 for(int i=0;i<f.size();i++) {
        data += (char)f.read();
 }
 f.close();
 data += "]}";
 Serial.println(data);
 digitalWrite(LED,!digitalRead(LED));
 server.send(200, "text/plane", data);
}


void nokia() {
    tone(4, 659, 102.272625);
    delay(113.63625);
    tone(4, 587, 102.272625);
    delay(113.63625);
    tone(4, 369, 204.54525);
    delay(227.2725);
    tone(4, 415, 204.54525);
    delay(227.2725);
    tone(4, 554, 102.272625);
    delay(113.63625);
    tone(4, 493, 102.272625);
    delay(113.63625);
    tone(4, 293, 204.54525);
    delay(227.2725);
    tone(4, 329, 204.54525);
    delay(227.2725);
    tone(4, 493, 102.272625);
    delay(113.63625);
    tone(4, 440, 102.272625);
    delay(113.63625);
    tone(4, 277, 204.54525);
    delay(227.2725);
    tone(4, 329, 204.54525);
    delay(227.2725);
    tone(4, 440, 818.181);
    delay(909.09);
    

}

void midi() {
    tone(tonePin, 493, 288.4615);
    delay(288.4615);
    tone(tonePin, 164, 144.23075);
    delay(144.23075);
    tone(tonePin, 523, 144.23075);
    delay(144.23075);
    tone(tonePin, 493, 144.23075);
    delay(144.23075);
    tone(tonePin, 466, 144.23075);
    delay(144.23075);
    tone(tonePin, 659, 288.4615);
    delay(288.4615);
    tone(tonePin, 246, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 932, 288.4615);
    delay(288.4615);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 195, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 493, 288.4615);
    delay(288.4615);
    tone(tonePin, 164, 144.23075);
    delay(144.23075);
    tone(tonePin, 523, 144.23075);
    delay(144.23075);
    tone(tonePin, 493, 144.23075);
    delay(144.23075);
    tone(tonePin, 466, 144.23075);
    delay(144.23075);
    tone(tonePin, 659, 288.4615);
    delay(288.4615);
    tone(tonePin, 246, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 932, 288.4615);
    delay(288.4615);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 246, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 659, 432.69225);
    delay(432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 130, 144.23075);
    delay(144.23075);
    tone(tonePin, 880, 144.23075);
    delay(144.23075);
    tone(tonePin, 1046, 144.23075);
    delay(144.23075);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    tone(tonePin, 880, 432.69225);
    delay(432.69225);
    tone(tonePin, 246, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 1318, 432.69225);
    delay(432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 195, 144.23075);
    delay(144.23075);
    tone(tonePin, 1244, 144.23075);
    delay(144.23075);
    tone(tonePin, 1318, 144.23075);
    delay(144.23075);
    tone(tonePin, 1479, 144.23075);
    delay(144.23075);
    tone(tonePin, 184, 432.69225);
    delay(432.69225);
    tone(tonePin, 932, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 233, 432.69225);
    delay(432.69225);
    tone(tonePin, 1108, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 220, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 1046, 288.4615);
    delay(288.4615);
    tone(tonePin, 261, 144.23075);
    delay(144.23075);
    delay(432.69225);
    tone(tonePin, 246, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 311, 144.23075);
    delay(144.23075);
    tone(tonePin, 880, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 739, 144.23075);
    delay(144.23075);
    tone(tonePin, 493, 288.4615);
    delay(288.4615);
    tone(tonePin, 164, 144.23075);
    delay(144.23075);
    tone(tonePin, 523, 144.23075);
    delay(144.23075);
    tone(tonePin, 493, 144.23075);
    delay(144.23075);
    tone(tonePin, 466, 144.23075);
    delay(144.23075);
    tone(tonePin, 659, 288.4615);
    delay(288.4615);
    tone(tonePin, 195, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 932, 288.4615);
    delay(288.4615);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 195, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 493, 288.4615);
    delay(288.4615);
    tone(tonePin, 164, 144.23075);
    delay(144.23075);
    tone(tonePin, 523, 144.23075);
    delay(144.23075);
    tone(tonePin, 493, 144.23075);
    delay(144.23075);
    tone(tonePin, 466, 144.23075);
    delay(144.23075);
    tone(tonePin, 659, 288.4615);
    delay(288.4615);
    tone(tonePin, 246, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 932, 288.4615);
    delay(288.4615);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 246, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 110, 432.69225);
    delay(432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 164, 144.23075);
    delay(144.23075);
    tone(tonePin, 880, 144.23075);
    delay(144.23075);
    tone(tonePin, 1046, 144.23075);
    delay(144.23075);
    tone(tonePin, 987, 144.23075);
    delay(144.23075);
    tone(tonePin, 123, 432.69225);
    delay(432.69225);
    tone(tonePin, 880, 432.69225);
    delay(432.69225);
    tone(tonePin, 184, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 1318, 432.69225);
    delay(432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 195, 144.23075);
    delay(144.23075);
    tone(tonePin, 1244, 144.23075);
    delay(144.23075);
    tone(tonePin, 1318, 144.23075);
    delay(144.23075);
    tone(tonePin, 1479, 144.23075);
    delay(144.23075);
    tone(tonePin, 184, 432.69225);
    delay(432.69225);
    tone(tonePin, 1318, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 369, 432.69225);
    delay(432.69225);
    tone(tonePin, 1108, 144.23075);
    delay(144.23075);
    delay(288.4615);
    tone(tonePin, 220, 432.69225);
    delay(432.69225);
    delay(432.69225);
    tone(tonePin, 1046, 288.4615);
    delay(288.4615);
    tone(tonePin, 329, 144.23075);
    delay(144.23075);
    delay(432.69225);
    tone(tonePin, 246, 432.69225);
    delay(432.69225);
    tone(tonePin, 987, 432.69225);
    delay(432.69225);
    delay(288.4615);
    tone(tonePin, 311, 144.23075);
    delay(144.23075);
    tone(tonePin, 880, 144.23075);
    delay(144.23075);
    tone(tonePin, 783, 144.23075);
    delay(144.23075);
    tone(tonePin, 739, 144.23075);
    delay(144.23075);

}

void setup()
{
  EEPROM.begin(256);
  Serial.begin(115200);
  SPIFFS.begin();
  dht.setup(DHTpin, DHTesp::DHT11); 
  WiFi.begin(ssid, password);
  Serial.println("");
  //Onboard LED port Direction output
  pinMode(LED,OUTPUT); 
  pinMode(5,OUTPUT);
  pinMode(15,OUTPUT);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  timeClient.begin();
pinMode(4, OUTPUT);
nokia();
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readADC", handleADC); //This page is called by java Script AJAX

server.on("/time", handleTime);      //Which routine to handle at root location
  server.on("/time/action_page", handleForm);
  server.on("/clear", handleClear);      //Which routine to handle at root location
  server.on("/watering", doWatering);
  server.on("/reboot", ESP.restart);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}

//==============================================================
//                     LOOP
//==============================================================
void loop()
{
  if(millis()%10000==0){
    timeClient.update();
    if(timeClient.getHours()>=EEPROM[0] and timeClient.getHours()<EEPROM[1]) digitalWrite(4, 1);
    else digitalWrite(4, 0);
    if(timeClient.getHours()==6 and timeClient.getMinutes() == 50) midi();
    if(timeClient.getHours()%12==0 and timeClient.getMinutes() == 0 and timeClient.getSeconds() <= 15){
      int a = 100-(analogRead(A0)/10);
      delay(dht.getMinimumSamplingPeriod());
      humidity = dht.getHumidity();
      temperature = dht.getTemperature();
      String mins = String(timeClient.getMinutes());
      if(mins.length()<2) mins = "0"+mins;
      String hrs = String(timeClient.getHours());
      if(hrs.length()<2) hrs = "0"+hrs;
      File f = SPIFFS.open(fname, "w");
      f.print("{\"measurments\":[{\"time\":\""+hrs+":"+mins+"\",\"soil\":"+String(a)+",\"temperature\":"+String(temperature)+",\"humidity\":"+String(humidity)+"}");
      f.close();
      delay(15000);
    }
    
    if(timeClient.getHours()==19 and timeClient.getMinutes() == 0 and timeClient.getSeconds() <= 15){
      digitalWrite(15, HIGH);
      delay(1000);
      digitalWrite(15, LOW);
      delay(15000);
    }
    
    delay(1);
  }
  if(millis()%60000<=5){
    Serial.println("writing data to file!");
    timeClient.update();
    int a = 100-(analogRead(A0)/10);
    delay(dht.getMinimumSamplingPeriod());
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    String mins = String(timeClient.getMinutes());
    if(mins.length()<2) mins = "0"+mins;
    String hrs = String(timeClient.getHours());
    if(hrs.length()<2) hrs = "0"+hrs;
    File f = SPIFFS.open(fname, "a");
    f.print(",{\"time\":\""+hrs+":"+mins+"\",\"soil\":"+String(a)+",\"temperature\":"+String(temperature)+",\"humidity\":"+String(humidity)+"}");
    f.close();
  
  
  }
  /*
  if(millis()%1000==0){
    if(100-(analogRead(A0)/10) <45){
      while(100-(analogRead(A0)/10) <55){
        digitalWrite(5, 1);
        delay(10);
      }
      digitalWrite(5, 0);
    }
    else digitalWrite(5, 0);
    delay(1);
  }
  

  if(millis()%5000<=5){
    byte pixNum = 0;
    strip.clear();
    pixNum = (timeClient.getHours() + 3) % 12;
    if(timeClient.getHours()>7 and timeClient.getHours()<=21){
      strip.setBrightness(32);
    }
    else {
      strip.setBrightness(2);
    }
    strip.setPixelColor(pixNum,  0xff7514);
    pixNum = (timeClient.getMinutes() / 5 + 3) % 12;
    strip.setPixelColor(pixNum, 0xa7fc00);
    pixNum = (timeClient.getSeconds() / 5 + 3) % 12;
    strip.setPixelColor(pixNum, 0x1faee9);
    strip.show();
    delay(5);
  }
  */
  server.handleClient();          //Handle client requests
}
