#include "lookup_tables.h"
#include <stdint.h>
const int startPin = 2;
const int endPin = 13;

// neg DC = 0,  pos DC = 1

struct Data66b {
  uint64_t lowBits = 0;
  uint8_t highBits = 0;
};

uint8_t rd = 0;
extern const uint16_t lookup_8B_10B[512];

void setup() {
    for (int pin = startPin; pin <= endPin; pin++) {
    pinMode(pin, OUTPUT);
  }
  Serial.begin(9600);
}

void print10BitBinary(uint16_t value) {
  for (int i = 9; i >= 0; i--) {
    Serial.print((value >> i) & 0x1); // Print each bit from highest to lowest
  }
  Serial.println();
}

void print8BitBinary(uint16_t value) {
  for (int i = 7; i >= 0; i--) {
    Serial.print((value >> i) & 0x1); // Print each bit from highest to lowest
  }
  Serial.println();
}

void print64BitBinary(uint64_t value) {
  for (int i = 63; i >= 0; i--) {
    Serial.print((uint16_t)(value >> i) & 0x1); // Print each bit from highest to lowest
  }
  Serial.println();
}

uint16_t convert_8B_10B(uint8_t byte) {
  uint16_t table_in = rd;
  table_in <<= 8;
  table_in |= byte;
  uint16_t encoded = lookup_8B_10B[table_in];
  rd = (encoded >> 10) & 0x01;
  return encoded & 0x3FF;
}

uint16_t reverseLowest10Bits(uint16_t value) {
  uint16_t reversed = 0;
  for (int i = 0; i < 10; i++) {
    reversed |= ((value >> i) & 1) << (9 - i);
  }
  return reversed;
}

void injectError(uint16_t &data, bool &isTransient) {
  int bitPosition = random(0, 15);
  data ^= (1 << bitPosition);   
  isTransient = random(0, 2);    
  // Modified this part to play with the transient rate.
  // isTransient = false;
}

bool isFaulty(uint16_t value) {
    uint16_t data = extractDataFromHamming(value) & 3FF;
    int consecutiveZeros = 0, maxConsecutiveZeros = 0;
    int consecutiveOnes = 0, maxConsecutiveOnes = 0;
    int onesCount = 0, zerosCount = 0;
    int transitions0To1 = 0, transitions1To0 = 0;

    bool previousBit = (data >> 9) & 1;
    for (int i = 9; i >= 0; i--) {
        bool currentBit = (data >> i) & 1;

        if (currentBit == 0) {
            consecutiveZeros++;
            consecutiveOnes = 0;
        } else {
            consecutiveOnes++;
            consecutiveZeros = 0;
        }

        maxConsecutiveZeros = max(maxConsecutiveZeros, consecutiveZeros);
        maxConsecutiveOnes = max(maxConsecutiveOnes, consecutiveOnes);

        if (currentBit == 1) {
            onesCount++;
        } else {
            zerosCount++;
        }

        if (i < 9 && currentBit != previousBit) {
            if (previousBit == 0 && currentBit == 1) {
                transitions0To1++;
            } else if (previousBit == 1 && currentBit == 0) {
                transitions1To0++;
            }
        }

        previousBit = currentBit;
    }
    
    if (maxConsecutiveZeros > 4) return true;
    if (maxConsecutiveOnes > 5) return true;
    if (transitions0To1 > 5) return true;
    if (transitions1To0 > 5) return true;
    if (abs(onesCount - zerosCount) > 3) return true;


    // Added for error detection.
    bool foundInTable = false;
    for (int i = 0; i < 512; i++) {
        if ((lookup_8B_10B[i] & 0x3FF) == data) {
            foundInTable = true;
            break;
        }
    }
    if (!foundInTable) {
        return true;
    }
    return false;
}

uint16_t encodeHamming(uint16_t data) {
    uint16_t code = 0;

    code |= (data & 0b0000000001) << 2; // Bit 3
    code |= (data & 0b0000000010) << 3; // Bit 5
    code |= (data & 0b0000000100) << 3; // Bit 6
    code |= (data & 0b0000001000) << 3; // Bit 7
    code |= (data & 0b0000010000) << 4; // Bit 9
    code |= (data & 0b0000100000) << 4; // Bit 10
    code |= (data & 0b0001000000) << 4; // Bit 11
    code |= (data & 0b0010000000) << 4; // Bit 12
    code |= (data & 0b0100000000) << 4; // Bit 13
    code |= (data & 0b1000000000) << 4; // Bit 14

    // Calculate parity bits
    // Position 1 (bit 1)
    uint8_t p1 = ((code >> 2) & 1) ^ ((code >> 4) & 1) ^ ((code >> 6) & 1) ^ ((code >> 8) & 1) ^ ((code >> 10) & 1) ^ ((code >> 12) & 1) ^ ((code >> 14) & 1);
    code |= (p1 << 0);

    // Position 2 (bit 2)
    uint8_t p2 = ((code >> 1) & 1) ^ ((code >> 2) & 1) ^ ((code >> 5) & 1) ^ ((code >> 6) & 1) ^ ((code >> 9) & 1) ^ ((code >> 10) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
    code |= (p2 << 1);

    // Position 4 (bit 4)
    uint8_t p4 = ((code >> 3) & 1) ^ ((code >> 4) & 1) ^ ((code >> 5) & 1) ^ ((code >> 6) & 1) ^ ((code >> 11) & 1) ^ ((code >> 12) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
    code |= (p4 << 3);

    // Position 8 (bit 8)
    uint8_t p8 = ((code >> 7) & 1) ^ ((code >> 8) & 1) ^ ((code >> 9) & 1) ^ ((code >> 10) & 1) ^ ((code >> 11) & 1) ^ ((code >> 12) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
    code |= (p8 << 7);

    // if ((extractDataFromHamming(code) & 0x3FF) != data) {
    //   Serial.println("Critical error");
    // }

    return code;
}

uint16_t decodeHamming(uint16_t code) {

    uint16_t hamming = code;
    
    uint8_t p1 = ((code >> 0) & 1) ^ ((code >> 2) & 1) ^ ((code >> 4) & 1) ^ ((code >> 6) & 1) ^ ((code >> 8) & 1) ^ ((code >> 10) & 1) ^ ((code >> 12) & 1) ^ ((code >> 14) & 1);
    uint8_t p2 = ((code >> 1) & 1) ^ ((code >> 2) & 1) ^ ((code >> 5) & 1) ^ ((code >> 6) & 1) ^ ((code >> 9) & 1) ^ ((code >> 10) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
    uint8_t p4 = ((code >> 3) & 1) ^ ((code >> 4) & 1) ^ ((code >> 5) & 1) ^ ((code >> 6) & 1) ^ ((code >> 11) & 1) ^ ((code >> 12) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
    uint8_t p8 = ((code >> 7) & 1) ^ ((code >> 8) & 1) ^ ((code >> 9) & 1) ^ ((code >> 10) & 1) ^ ((code >> 11) & 1) ^ ((code >> 12) & 1) ^ ((code >> 13) & 1) ^ ((code >> 14) & 1);
   
    uint16_t syndrome = (p1 << 0) | (p2 << 1) | (p4 << 2) | (p8 << 3);

    if (syndrome != 0) {
    int errorBit = syndrome - 1; // Calculate the position of the error (0-indexed)
    hamming ^= (1 << errorBit);
    Serial.print("Hamming: Error corrected at bit ");
    Serial.println(errorBit);
    }

    return hamming;
}


uint16_t extractDataFromHamming(uint16_t hammingCode) {
  
    uint16_t data = 0;
    // Data bits are at positions 3, 5, 6, 7, 9, 10, 11, 12, 13, 14
    data |= ((hammingCode >> 2) & 1) << 0; // Bit 3 -> Bit 0
    data |= ((hammingCode >> 4) & 1) << 1; // Bit 5 -> Bit 1
    data |= ((hammingCode >> 5) & 1) << 2; // Bit 6 -> Bit 2
    data |= ((hammingCode >> 6) & 1) << 3; // Bit 7 -> Bit 3
    data |= ((hammingCode >> 8) & 1) << 4; // Bit 9 -> Bit 4
    data |= ((hammingCode >> 9) & 1) << 5; // Bit 10 -> Bit 5
    data |= ((hammingCode >> 10) & 1) << 6; // Bit 11 -> Bit 6
    data |= ((hammingCode >> 11) & 1) << 7; // Bit 12 -> Bit 7
    data |= ((hammingCode >> 12) & 1) << 8; // Bit 13 -> Bit 8
    data |= ((hammingCode >> 13) & 1) << 9; // Bit 14 -> Bit 9

    return data;
}


void loop() {
  Data66b data_66b;

  for (int i = 0; i < 6; i++) {
    uint64_t analogValue = analogRead(A0 + i);
    for (int bitPos = 0; bitPos < 10; bitPos++) {
      int bitValue = (analogValue >> bitPos) & 1;
      int targetBitPos = i * 10 + bitPos;
      if (bitValue == 0x01) {
        data_66b.lowBits |= (uint64_t)((uint64_t)0x01 << targetBitPos);
      }
    }
  }

  uint8_t last_six_bits = analogRead(A0) & 0x3F;
  data_66b.lowBits |= last_six_bits << (60);
  data_66b.highBits = random(1, 3);

  uint16_t encodedValues[8];    
  uint16_t originalValues[8];     
  uint16_t originalHamming[8];
  uint16_t encodedHamming[8];
  bool transientErrors[8] = {0};   
  bool suspectedFaults[8] = {0};  
  bool guessedTransient[8] = {false};
  bool guessedPermenant[8] = {false};

  for (int i = 0; i < 8; i++) {
    uint8_t byte = (data_66b.lowBits >> (i * 8)) & 0xFF;
    uint16_t encoded = convert_8B_10B(byte);
    encoded = reverseLowest10Bits(encoded);
    // encodedValues[i] = encoded;
    originalValues[i] = encoded; 
    originalHamming[i] = encodeHamming(encoded);
    encodedHamming[i] = encodeHamming(encoded);


    if (random(0, 10) < 9) {
      injectError(encodedHamming[i], transientErrors[i]);
    }

    suspectedFaults[i] = isFaulty(encodedHamming[i]);
  }

Serial.println("Correcting transient errors...");
for (int i = 0; i < 8; i++) {
  if (transientErrors[i]) {
    encodedHamming[i] = originalHamming[i];
    transientErrors[i] = false;
    Serial.print("Transient error corrected at index ");
    Serial.println(i);
    if (suspectedFaults[i]) {
      Serial.println("This bit is also suspected by the encoding.");
    }
  }
}


Serial.println("Current suspected values are:");
for (int i = 0; i < 8; i++) {
  if (suspectedFaults[i]) {
    Serial.print("Index ");
    Serial.println(i);
  }
}

delay(2000);

Serial.println("Analyzing faults with Hamming code...");
for (int i = 0; i < 8; i++) {
  if (suspectedFaults[i]) {
    uint16_t correctedHamming = decodeHamming(encodedHamming[i]);
    if (correctedHamming != encodedHamming[i]) {
      // Permanent fault detected and corrected
      uint16_t correctedRawData = 0;
      correctedRawData = extractDataFromHamming(correctedHamming);

    if (correctedRawData != originalValues[i]) {
        Serial.println("Decoding hammingi s not correct");
      }

      guessedPermenant[i] = true;
      Serial.print("Permanent fault detected at index ");
      Serial.print(i);
      // Serial.print(". Original value: ");
      // print10BitBinary(extractDataFromHamming(encodedHamming[i]));
      Serial.print("Corrected value: ");
      print10BitBinary(correctedRawData);
    } else {
      // Error was transient and fixed earlier, or a false alarm
      guessedTransient[i] = true;
      Serial.print("False alarm or transient error at index ");
      Serial.println(i);
    }
  }
}


Serial.println("Final data:");

for (int i = 0; i < 8; i++) {
    print10BitBinary(originalValues[i]);
    for (int pin = startPin + 2; pin <= endPin; pin++) {
      int delta = pin - startPin - 2;
      if ((originalValues[i] >> delta) & 1) {
        digitalWrite(pin, HIGH);
      }
      else {
        digitalWrite(pin, LOW);
      }
  }
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  if (guessedPermenant[i] == true) {
    digitalWrite(3, HIGH);
  }
  if (guessedTransient[i] == true) {
    digitalWrite(2, HIGH);
  }
  delay(60000);
}

  delay(300000);
}
