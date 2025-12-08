#include <Arduino.h>
#include <SPI.h>
#include <M5Module_LAN.h>
#include <EthernetClient.h>
#include <ArduinoHttpClient.h>

// InfluxDB settings
//
const char INFLUXDB_URL[] = "yourAddress.org";
const char INFLUXDB_USER[] = "yourName";
const char INFLUXDB_PASSWORD[] = "yourPasseord";
const char* INFLUXDB_PATH = "/write?db=yourDBname";

//Ethernet setting
uint8_t cs_pin = 19; // stamp CS pin
uint8_t rst_pin = 0; // stamp RST pin
uint8_t int_pin = 21;// stamp SDA pin
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xFE};
M5Module_LAN LAN;

EthernetClient client;
HttpClient http = HttpClient(client, INFLUXDB_URL, 8086);

String payload;

//--- write fuction --------------------------------------------

bool sendToInfluxDB(String payload) {

  Serial.println(payload);

  http.beginRequest();
  http.post(INFLUXDB_PATH);
  http.sendBasicAuth(INFLUXDB_USER, INFLUXDB_PASSWORD);
  http.sendHeader("Content-Type", "text/plain");
  http.sendHeader("Content-Length", payload.length());
  http.beginBody();
  http.print(payload);
  http.endRequest();

  int statusCode = http.responseStatusCode();

  if (statusCode == 204) {
    Serial.printf("Status Code %d\n", statusCode);
    http.stop();
    return true;
  } else {
    Serial.printf("Failed Code %d\n", statusCode);
    http.stop();
    return false;
  }
}

//--- setuo --------------------------------------------

void setup(){

  Serial.begin(115200);

  //Ethernet init
  Serial.print("\nEthernet Connect");
  SPI.begin(SCK, MISO, MOSI, -1);

  LAN.setResetPin(rst_pin);
  LAN.reset();
  LAN.init(cs_pin);

  while (LAN.begin(mac) != 1) {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(2000);
  }
  Serial.print("\nLAN connected\n");
  Serial.println("IP address: ");
  Serial.println(LAN.localIP());

}

//--- loop --------------------------------------------

void loop(){

  payload = "measurement1,Controller=1 ";
  payload += "VOL=" + String(48.0);
  payload += ",CUR=" + String(5.0);
  payload += ",TMP=" + String(15.6);

  if (sendToInfluxDB(payload)) {
    payload = "";
  }
  delay(100);

  payload = "measurement1,Controller=2 ";
  payload += "VOL=" + String(12.0);
  payload += ",CUR=" + String(2.0);
  payload += ",TMP=" + String(15.8);

  if (sendToInfluxDB(payload)) {
    payload = "";
  }

  while(1){}
}
