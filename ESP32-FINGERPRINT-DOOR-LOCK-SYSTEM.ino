#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Blynk Credentials
char auth[] = "k3E_C69GE--0z82idLxC3nMTz6qunskz";
char ssid[] = "IT solutions";
char pass[] = "Python@786";

// Fingerprint Sensor Setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Hardware Pins
#define relay 15     // Relay module
#define buzzer 5     // Buzzer
#define redLED 18    // Red LED (Invalid Finger)
#define greenLED 19  // Green LED (Valid Finger)

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // TX=16, RX=17 for R307

  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  
  digitalWrite(relay, HIGH);   // Ensure relay starts OFF
  digitalWrite(buzzer, LOW);
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi!");

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  
  if (finger.verifyPassword()) {
    Serial.println("✅ Fingerprint Sensor detected!");
  } else {
    Serial.println("❌ ERROR: Fingerprint Sensor NOT found!");
    while (1);
  }
}

// Function to Match Fingerprint and Trigger Relay
int getFingerprintID() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    Serial.println("❌ Invalid Fingerprint!");
    digitalWrite(buzzer, HIGH);
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Blynk.virtualWrite(V2, 1); // Red LED ON in Blynk
    Blynk.virtualWrite(V1, 0); // Green LED OFF in Blynk
    delay(1000);
    digitalWrite(buzzer, LOW);
    return -1;
  }

  Serial.print("✅ Finger Matched! ID: "); Serial.println(finger.fingerID);

  // Unlock door
  digitalWrite(relay, LOW);   // Turn relay ON
  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, LOW);
  Blynk.virtualWrite(V1, 1); // Green LED ON in Blynk
  Blynk.virtualWrite(V2, 0); // Red LED OFF in Blynk

  delay(2000); // Keep door unlocked for 2 sec

  // Lock door again (Fix: Ensure relay turns off)
  Serial.println("🔒 Locking Door...");
  digitalWrite(relay, HIGH);  
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);
  Blynk.virtualWrite(V1, 0); // Green LED OFF in Blynk
  Blynk.virtualWrite(V2, 1); // Red LED ON in Blynk

  return finger.fingerID;
}

// Blynk Button to Manually Unlock Door
BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  digitalWrite(relay, pinValue);

  if (pinValue) {
    Serial.println("🔓 Door Unlocked via Blynk!");
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    Blynk.virtualWrite(V1, 1);
    Blynk.virtualWrite(V2, 0);
  } else {
    Serial.println("🔒 Door Locked via Blynk!");
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 1);
  }
}

void loop() {
  int id = getFingerprintID();
  if (id > 0) {
    Serial.println("🔓 Door Unlocked by Fingerprint!");
  }
  Blynk.run();
}
