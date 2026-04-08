#include <WiFi.h>
#include <HTTPClient.h>

// ------------ WIFI ------------
const char* ssid = "TOQA";
const char* password = "1237900&$#@*";

// ------------ THINGSPEAK ------------
String apiKey = "8VG51YKAWPSBQM73";
String server = "http://api.thingspeak.com/update";

// ------------ PINS (ADC1) ------------
#define NTC_PIN 34   // NTC analog output
#define GAS_PIN 35   // Potentiometer wiper (gas simulation)

// ------------ TIMING ------------
unsigned long lastSend = 0;
const unsigned long interval = 20000; // 20 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("BOOT OK");

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED - check SSID/password");
  }
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend < interval) return;
  lastSend = now;

  // -------- Read sensors (RAW ADC) --------
  int gasRaw = analogRead(GAS_PIN);  // 0..4095
  int ntcRaw = analogRead(NTC_PIN);  // 0..4095

  // -------- Convert NTC raw to an approximate temperature --------
  // (Simple Wokwi/educational mapping, not a calibrated NTC equation)
  float voltage = ntcRaw * (3.3 / 4095.0);
  float temperatureC = (voltage - 0.5) * 100.0;

  // -------- Serial output --------
  Serial.print("Gas (Pot): ");
  Serial.print(gasRaw);
  Serial.print(" | NTC raw: ");
  Serial.print(ntcRaw);
  Serial.print(" | Temp approx: ");
  Serial.print(temperatureC, 2);
  Serial.println(" C");

  // -------- Send to ThingSpeak --------
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = server + "?api_key=" + apiKey +
                 "&field1=" + String(gasRaw) +
                 "&field2=" + String(temperatureC, 2);

    http.begin(url);
    int httpCode = http.GET();
    http.end();

    Serial.println(httpCode > 0 ? "Sent to ThingSpeak" : "HTTP request failed");
  } else {
    Serial.println("WiFi disconnected");
  }
}
