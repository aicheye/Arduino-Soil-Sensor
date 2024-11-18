#include <SoftwareSerial.h>

#define RE_PIN 2
#define DE_PIN 3
#define RO_PIN 4
#define DI_PIN 5 

SoftwareSerial max485(RO_PIN, DI_PIN);

float moisture = 0;
float temp = 0;
int ec = 0;
float ph = 0;
int n = 0;
int p = 0;
int k = 0; 

void setup() {
    Serial.begin(9600);
    max485.begin(4800);

    pinMode(RE_PIN, OUTPUT);
    pinMode(DE_PIN, OUTPUT);

    digitalWrite(RE_PIN, LOW);
    digitalWrite(DE_PIN, LOW);
    
    delay(1000);
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
    }
    
    uint8_t request[] = {
        0x01,
        0x03,
        0x00, 0x00,
        0x00, 0x07,
        0x04, 0x08
    };

    digitalWrite(RE_PIN, HIGH);
    digitalWrite(DE_PIN, HIGH);
    delay(100);

    for (int i = 0; i < 8; i++) {
        max485.write(request[i]);
    }
    max485.flush();
    
    delay(10);
    digitalWrite(RE_PIN, LOW);
    digitalWrite(DE_PIN, LOW);

    while (max485.available()) {
        max485.read();
    }
    
    unsigned long startTime = millis();
    const int expectedBytes = 19;
    uint8_t buffer[expectedBytes];
    int bytesRead = 0;

    while (bytesRead < expectedBytes && (millis() - startTime) < 1000) {
        if (max485.available()) {
            buffer[bytesRead] = max485.read();
            bytesRead++;
        }
    }

    if (bytesRead == expectedBytes) {
        if (buffer[0] != 0x01 || buffer[1] != 0x03) {
            return;
        }
        
        uint16_t receivedCRC = (buffer[bytesRead - 1] << 8) | buffer[bytesRead - 2];

        int length = bytesRead - 2;
        uint16_t crc = 0xFFFF;

        uint8_t *b = buffer;
    
        while (length--) {
            crc ^= *b++;
            for (uint8_t i = 0; i < 8; i++) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }

        if (receivedCRC == crc) {
            moisture = ((buffer[3] << 8) | buffer[4]) / 10.0;
            temp = ((buffer[5] << 8) | buffer[6]) / 10.0;
            ec = (buffer[7] << 8) | buffer[8];
            ph = ((buffer[9] << 8) | buffer[10]) / 10.0;
            n = (buffer[11] << 8) | buffer[12];
            p = (buffer[13] << 8) | buffer[14];
            k = (buffer[15] << 8) | buffer[16];
        } 
        
        else return;
    }

    else return;

    Serial.print("Moisture_(%):");
    Serial.print(moisture, 1);
    Serial.print(",");
    Serial.print("Temp_(C):");
    Serial.print(temp, 1);
    Serial.print(",");
 // Serial.print("ec_(us_cm):");
 // Serial.print(ec);
 // Serial.print(",");
    Serial.print("pH:");
    Serial.print(ph, 1);
    Serial.print(",");
    Serial.print("Nitrogen_(mg_kg):");
    Serial.print(n);
    Serial.print(",");
    Serial.print("Phosphorus_(mg_kg):");
    Serial.print(p);
    Serial.print(",");
    Serial.print("Potassium_(mg_kg):");
    Serial.println(k);

    delay(2000);
}
