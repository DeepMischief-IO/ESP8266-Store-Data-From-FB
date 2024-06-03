

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <LittleFS.h>
#include <addons/RTDBHelper.h>
#include <ArduinoJson.h>
#include <typeinfo>
// Insert your network credentials
#define WIFI_SSID "redmi"
#define WIFI_PASSWORD "12345678ABc"
// Insert Firebase project API Key
#define API_KEY "AIzaSyB0T5yhA1GPiGCYGihrT4vQseUwxr7RBSA"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp-firebase-demo-1c5d9-default-rtdb.asia-southeast1.firebasedatabase.app/test/home"
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "chan@gmail.com"
#define USER_PASSWORD "chanchan"
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;

////Method for Write to Flash Memory////
void writeDataToFile(const char* filename, const char* data) {
    File file = LittleFS.open(filename, "w");
    if (file) {
        file.print(data);
        file.close();
        Serial.println("Data written successfully");
    } else {
        Serial.println("Error opening file for writing");
    }
} ///////////////////////////////////////////////

/// REad From File /////////////////////////
void readDataFromFile(const char* filename) {
    File file = LittleFS.open(filename, "r");
    if (file) {
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
    } else {
        Serial.println("Error opening file for reading");
    }
}
/// end read function //////////

void setup()
{

    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }
    Serial.println("LittleFS mounted successfully");

    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Since Firebase v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    // Or use legacy authenticate method
    // config.database_url = DATABASE_URL;
    // config.signer.tokens.legacy_token = "<database secret>";
    Firebase.begin(&config, &auth);

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);
}

void loop()
{


    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        // FirebaseJson json;
        // json.setDoubleDigits(3);
        // json.add("value", count);

        // Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, "/test/json", &json) ? "ok" : fbdo.errorReason().c_str());

        // Serial.print("Raw Data : ");
        // Serial.println(Firebase.RTDB.getJSON(&fbdo, "/test/setting/range"));

        if (Firebase.RTDB.getJSON(&fbdo, "/test/setting/"))
        {
            const char* data = fbdo.to<FirebaseJson>().raw();
            writeDataToFile("test.json",data);
            readDataFromFile("test.json");
            
            StaticJsonDocument<512> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, data);
            if (error)

            {
                Serial.print("DeserializationJson()  Failed :");
                Serial.println(error.f_str());
                return;
                /* code */
            }

            int max = jsonDoc["range"]["max"];
            // String jsonStr = "{\"temperature\": 25.5, \"humidity\": 60}";
            // writeDataToFile("test1.json",jsonStr.c_str());
            // readDataFromFile("test1.json");

            const char *deviceName = jsonDoc["deviceid"];
            String firmwareName = jsonDoc["firmware_version"];
            //    Serial.println("Value:" +maxValue);
            // Serial.printf("Device ID: %s,Firmware: %s, Temperature Max Value: %s", deviceName, firmwareName);
            //  Serial.print("Firmware Version :" +firmwareName);
            
            Serial.print("Max Value: ");
            Serial.println(max);
             Serial.print("Device Name: ");
            Serial.println(deviceName);
             Serial.print(" Firmware Version : ");
            Serial.println(firmwareName);
        }
        else
        {
            Serial.println(fbdo.errorReason().c_str());
        }

        // Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/setting/range") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());

        // FirebaseJson jVal;
        // Serial.printf("Get json ref... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/setting/", &jVal) ? jVal.raw() : fbdo.errorReason().c_str());

        // FirebaseJsonArray arr;
        // arr.setFloatDigits(2);
        // arr.setDoubleDigits(4);
        // arr.add("a", "b", "c", true, 45, (float)6.1432, 123.45692789);

        // Serial.printf("Set array... %s\n", Firebase.RTDB.setArray(&fbdo, "/test/array", &arr) ? "ok" : fbdo.errorReason().c_str());

        // Serial.printf("Get array... %s\n", Firebase.RTDB.getArray(&fbdo, "/test/array") ? fbdo.to<FirebaseJsonArray>().raw() : fbdo.errorReason().c_str());

        // Serial.printf("Push json... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/test/push", &json) ? "ok" : fbdo.errorReason().c_str());

        // json.set("value", count + 1);
        // Serial.printf("Update json... %s\n\n", Firebase.RTDB.updateNode(&fbdo, "/test/update/" + fbdo.pushName(), &json) ? "ok" : fbdo.errorReason().c_str());

        // count++;
    }
    delay(1000);
}
