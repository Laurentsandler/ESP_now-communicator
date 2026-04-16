#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> // Most 1.3" OLEDs use the SH1106 controller

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C    // Typical I2C address for 1.3" OLEDs

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Button Setup ---
const int numButtons = 12;
const byte buttonPins[numButtons] = {4, 5, 6, 7, 15, 16, 17, 18, 10, 11, 12, 13};
const char buttonChars[numButtons] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'M', '0', 'S'};

#define NO_KEY '\0'

char getCustomKey() {
  static unsigned long lastDebounceTime = 0;
  static char lastStableChar = NO_KEY;
  static char lastReading = NO_KEY;
  const unsigned long debounceDelay = 50;

  char currentReading = NO_KEY;
  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      currentReading = buttonChars[i];
      break;
    }
  }

  if (currentReading != lastReading) {
    lastDebounceTime = millis();
  }

  char finalKey = NO_KEY;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentReading != lastStableChar) {
      lastStableChar = currentReading;
      if (lastStableChar != NO_KEY) {
        finalKey = lastStableChar;
      }
    }
  }
  
  lastReading = currentReading;
  return finalKey;
}

// --- Multi-tap (T9 style) Setup ---
const char* const t9map[10] = {
  " 0",      // 0 button: Space, 0
  ".,?!1",   // 1 button: Punctuation, 1
  "abc2",    // 2 button
  "def3",    // 3 button
  "ghi4",    // 4 button
  "jkl5",    // 5 button
  "mno6",    // 6 button
  "pqrs7",   // 7 button
  "tuv8",    // 8 button
  "wxyz9"    // 9 button
};

char currentMsg[64] = "";
int msgLen = 0;
char receivedMsg[64] = "";
bool newMsgReceived = false;

char lastKey = NO_KEY;
unsigned long lastPressTime = 0;
int t9Index = 0;

// Broadcast MAC Address (FF:FF:FF:FF:FF:FF) 
// This sends to any listener so no need to hardcode each device's specific MAC.
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define MSG_TYPE_PING 0
#define MSG_TYPE_TEXT 1

const uint32_t PAIRING_SECRET = 0x1A2B3C4D; // Unique app ID so they only pair with each other

typedef struct struct_message {
  uint8_t msgType;
  uint32_t secret;
  char txt[64];
} struct_message;

uint8_t peerAddress[6] = {0, 0, 0, 0, 0, 0};
bool peerDiscovered = false;
unsigned long lastSeenTime = 0;
unsigned long lastPingTime = 0;

esp_now_peer_info_t peerInfo;

bool isOnline() {
  return peerDiscovered && ((millis() - lastSeenTime) <= 10000);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(struct_message)) return;
  
  struct_message incomingMsg;
  memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));
  
  if (incomingMsg.secret == PAIRING_SECRET) {
    if (incomingMsg.msgType == MSG_TYPE_PING) {
      if (!peerDiscovered || memcmp(peerAddress, mac, 6) != 0) {
        memcpy(peerAddress, mac, 6);
        peerDiscovered = true;
        
        if (!esp_now_is_peer_exist(peerAddress)) {
          esp_now_peer_info_t specificPeer;
          memset(&specificPeer, 0, sizeof(specificPeer));
          memcpy(specificPeer.peer_addr, peerAddress, 6);
          specificPeer.channel = 0;
          specificPeer.encrypt = false;
          esp_now_add_peer(&specificPeer);
        }
      }
      lastSeenTime = millis();
    } else if (incomingMsg.msgType == MSG_TYPE_TEXT) {
      lastSeenTime = millis();
      strncpy(receivedMsg, incomingMsg.txt, sizeof(receivedMsg));
      receivedMsg[sizeof(receivedMsg)-1] = '\0';
      newMsgReceived = true;
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  // Status Indicator
  display.setCursor(0, 0);
  if (isOnline()) {
    display.println("Status: ONLINE");
  } else {
    display.println("Status: OFFLINE");
  }
  
  // Received Message Section
  display.setCursor(0, 10);
  display.println("Received:");
  display.setCursor(0, 20);
  display.println(receivedMsg);
  
  // Divider line
  display.drawLine(0, 31, 128, 31, SH110X_WHITE);
  
  // Typing Message Section
  display.setCursor(0, 36);
  display.println("You:");
  display.setCursor(0, 46);
  display.print(currentMsg);

  display.display();
}

void setup() {
  Serial.begin(115200);

  // Initialize buttons
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Initialize display
  Wire.begin(8, 9);
  // Use typical I2C pins for your ESP32-S3 (SDA/SCL usually 8/9, but check your board map)
  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register broadcast peer
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  updateDisplay();
}

void loop() {
  char key = getCustomKey();
  
  if (key != NO_KEY) {
    unsigned long now = millis();
    
    // T9 Text Entry Logic for buttons 0-9
    if (key >= '0' && key <= '9') {
      int num = key - '0';
      int mapLen = strlen(t9map[num]);

      if (key == lastKey && (now - lastPressTime) < 1000) {
        // Cycle to next letter if pressed same key within 1 second
        t9Index = (t9Index + 1) % mapLen;
        if (msgLen > 0) {
          currentMsg[msgLen - 1] = t9map[num][t9Index]; // Swap last character
        }
      } else {
        // Start entering a new character
        t9Index = 0;
        if (msgLen < 63) { // Ensure buffer doesn't overflow
          currentMsg[msgLen] = t9map[num][t9Index];
          currentMsg[msgLen + 1] = '\0';
          msgLen++;
        }
      }
      lastKey = key;
      lastPressTime = now;
      updateDisplay();
    }
    else if (key == 'S') { 
      // 'S' mapped to: SEND MESSAGE
      if (peerDiscovered) {
        struct_message outgoingMsg;
        outgoingMsg.msgType = MSG_TYPE_TEXT;
        outgoingMsg.secret = PAIRING_SECRET;
        strncpy(outgoingMsg.txt, currentMsg, sizeof(outgoingMsg.txt));
        outgoingMsg.txt[sizeof(outgoingMsg.txt)-1] = '\0';
        
        esp_now_send(peerAddress, (uint8_t *) &outgoingMsg, sizeof(outgoingMsg));
      }
      
      // Clear input buffer after send
      memset(currentMsg, 0, sizeof(currentMsg));
      msgLen = 0;
      lastKey = NO_KEY;
      updateDisplay();
    }
    else if (key == 'M') { 
      // 'M' mapped to: MENU or BACKSPACE/DELETE
      if (msgLen > 0) {
        msgLen--;
        currentMsg[msgLen] = '\0';
      }
      lastKey = NO_KEY;
      updateDisplay();
    }
  }

  // Auto lock in the typed character after 1 second of inactivity
  if (lastKey != NO_KEY && (millis() - lastPressTime) > 1000) {
    lastKey = NO_KEY;
  }
  
  // Check if we got something new via ESP-NOW to update the screen
  if (newMsgReceived) {
    newMsgReceived = false;
    updateDisplay();
  }
  
  // Send PING every 3 seconds
  unsigned long currentMillisMillis = millis();
  if (currentMillisMillis - lastPingTime >= 3000) {
    lastPingTime = currentMillisMillis;
    struct_message pingMsg;
    pingMsg.msgType = MSG_TYPE_PING;
    pingMsg.secret = PAIRING_SECRET;
    pingMsg.txt[0] = '\0';
    esp_now_send(broadcastAddress, (uint8_t *) &pingMsg, sizeof(pingMsg));
  }
  
  // Update display if online status changes
  static bool lastOnlineStatus = false;
  bool currentStatus = isOnline();
  if (currentStatus != lastOnlineStatus) {
    lastOnlineStatus = currentStatus;
    updateDisplay();
  }
}
