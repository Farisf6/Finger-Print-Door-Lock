#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Initialize Serial for Fingerprint Sensor
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
    Serial.begin(115200);
    mySerial.begin(57600, SERIAL_8N1, 16, 17);  // TX = 16, RX = 17

    Serial.println("Fingerprint Enrollment System");
    if (finger.verifyPassword()) {
        Serial.println("✅ Fingerprint Sensor Detected!");
        Serial.print("Stored Fingerprints: ");
        Serial.println(finger.templateCount);
    } else {
        Serial.println("❌ ERROR: Fingerprint Sensor NOT Found!");
        while (1); // Stop execution
    }
}

void loop() {
    Serial.println("\nEnter a Fingerprint ID (1-127) to Enroll:");
    while (Serial.available() == 0);
    int id = Serial.parseInt();
    if (id > 0 && id <= 127) {
        Serial.print("Enrolling ID: ");
        Serial.println(id);
        enrollFingerprint(id);
    } else {
        Serial.println("⚠ Invalid ID! Please enter a number between 1 and 127.");
    }
}

// Function to Enroll a Fingerprint with Better Error Handling
void enrollFingerprint(int id) {
    int p = -1;
    Serial.println("Place your finger on the sensor...");
    
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        if (p == FINGERPRINT_NOFINGER) continue;
        if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("⚠ Communication error. Check connections.");
            return;
        }
        if (p != FINGERPRINT_OK) {
            Serial.println("❌ ERROR: Could not read fingerprint. Try again.");
        }
    }

    p = finger.image2Tz(1);
    if (p != FINGERPRINT_OK) {
        Serial.println("❌ ERROR: Image conversion failed. Try again.");
        return;
    }

    Serial.println("✅ Fingerprint captured. Remove your finger.");
    delay(2000);

    Serial.println("Place the same finger again...");
    p = -1;
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        if (p == FINGERPRINT_NOFINGER) continue;
        if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("⚠ Communication error. Check connections.");
            return;
        }
        if (p != FINGERPRINT_OK) {
            Serial.println("❌ ERROR: Could not read fingerprint. Try again.");
        }
    }

    p = finger.image2Tz(2);
    if (p != FINGERPRINT_OK) {
        Serial.println("❌ ERROR: Image conversion failed.");
        return;
    }

    p = finger.createModel();
    if (p != FINGERPRINT_OK) {
        Serial.println("❌ ERROR: Fingerprint match failed.");
        return;
    }

    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
        Serial.println("✅ Fingerprint Enrolled Successfully!");
    } else {
        Serial.println("❌ ERROR: Failed to store fingerprint.");
    }
}
