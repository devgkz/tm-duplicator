#include <OneWire.h>

#define pin 2

#define PIN_BUTTON_READ 12 // D6
#define PIN_BUTTON_WRITE 14 // D5


byte MODE_IDLE = 0;
byte MODE_READ = 1;
byte MODE_WRITE = 2;

OneWire myWire (pin);

byte mode;
byte addr[8];

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Program started. Idle mode.");
  mode = MODE_IDLE;
}

void loop() {
  myWire.reset_search();

  // prevent pin noise
  pinMode(PIN_BUTTON_READ, OUTPUT); digitalWrite(PIN_BUTTON_READ, HIGH);
  pinMode(PIN_BUTTON_WRITE, OUTPUT); digitalWrite(PIN_BUTTON_WRITE, HIGH);

  pinMode(PIN_BUTTON_READ, INPUT);
  pinMode(PIN_BUTTON_WRITE, INPUT);

  if (digitalRead(PIN_BUTTON_WRITE) == LOW) {
    mode = MODE_WRITE;
  } else if (digitalRead(PIN_BUTTON_READ) == LOW) {
    mode = MODE_READ;
  }

  if (mode == MODE_READ) {
      Serial.println("Mode read.");
      resetMode();      
      readKey();
      delay(1000);
  } 
  if (mode == MODE_WRITE) {
      Serial.println("Mode write.");      
      resetMode();
      writeKey();
      delay(1000);
  }
}

void resetMode() {
  mode = MODE_IDLE;
}

int writeByte(byte data) {
  int data_bit;
  for (data_bit = 0; data_bit < 8; data_bit++) {
    if (data & 1) {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    } else {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
  return 0;
}

void readKey() {  
  if (myWire.search(addr)) {
    String keycode = String(addr[0], HEX)+' '+String(addr[1], HEX)+' '+
              String(addr[2], HEX)+' '+String(addr[3], HEX)+' '+
              String(addr[4], HEX)+' '+String(addr[5], HEX)+' '+
              String(addr[6], HEX)+' '+String(addr[7], HEX);
    Serial.println("Key code: " + keycode);    

  } else {
    Serial.println("Key not found. Put the key and select mode.");
  }  
}

void writeKey() {
  byte addrOld[8];
  byte addrNew[8];

  if (myWire.search(addrOld)) {
    byte keyCrc;
    keyCrc = myWire.crc8(addr, 7);
    
    if (keyCrc == 0) {
      Serial.println("Write error. No source key.");
      return;
    }

    Serial.println("Writing: ");
    myWire.skip(); myWire.reset(); myWire.write(0x33);
    Serial.print("*");
    myWire.skip(); myWire.reset(); myWire.write(0xD1);
    Serial.print("*");
    digitalWrite(2, LOW); pinMode(2, OUTPUT); delayMicroseconds(60);
    Serial.print("*");
    pinMode(2, INPUT); digitalWrite(2, HIGH); delay(10);
    Serial.print("*");
    myWire.skip(); myWire.reset(); myWire.write(0xD5);
    
    for (byte v = 0; v < 8; v++) {
      writeByte(addr[v]);
      Serial.print(">");
      delay(100);
    }
    myWire.reset();
    myWire.write(0xD1);
    Serial.print("*");
    digitalWrite(2, LOW); pinMode(2, OUTPUT); delayMicroseconds(10);
    pinMode(2, INPUT); digitalWrite(2, HIGH); delay(10);
    Serial.println("*");

    myWire.reset_search();

    if (myWire.search(addrNew)) {
      byte crcSourceKey;
      crcSourceKey = myWire.crc8(addr, 7);
      
      byte crcTargetKey;
      crcTargetKey = myWire.crc8(addrNew, 7);
      
      if ( crcSourceKey == crcTargetKey ) {
        Serial.println("Write completed successfully.");
      } else {
        Serial.println("Write failed. CRC not equal");
      }
    } else {
      Serial.println("Error. Key removed too early.");
      Serial.println("Put the key and select mode.");
    }

  } else {
    Serial.println("Key not found. Put the key and select mode.");
  }
}
