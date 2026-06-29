#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <MIDI.h>

#define EEPROM_SIZE 2
#define EEPROM_MAGIC 0xA5
#define ADDR_MAGIC 0
#define ADDR_CHANNEL 1
#define DEFAULT_CHANNEL 6

// SysEx: F0 7D 01 <0x00-0x0F> F7 — set MIDI channel 1-16
#define SYSEX_MFR_ID 0x7D // Non-commercial / educational
#define SYSEX_CMD_SET_CHANNEL 0x01

// CC 80 bits 0-4: relays 1-5 (GPIO 10-14)
// CC 81 bits 0-4: relays 6-10 (GPIO 15-19)
#define CC_RELAY_A 80
#define CC_RELAY_B 81
#define RELAY_BASE 10

Adafruit_USBD_MIDI usb_midi;

MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, USBMIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, SerialMIDI);

midi::Channel loadChannel() {
    EEPROM.begin(EEPROM_SIZE);
    if (EEPROM.read(ADDR_MAGIC) == EEPROM_MAGIC) {
        uint8_t ch = EEPROM.read(ADDR_CHANNEL);
        if (ch >= 1 && ch <= 16)
            return (midi::Channel)ch;
    }
    return (midi::Channel)DEFAULT_CHANNEL;
}

void saveChannel(midi::Channel ch) {
    EEPROM.write(ADDR_MAGIC, EEPROM_MAGIC);
    EEPROM.write(ADDR_CHANNEL, (uint8_t)ch);
    EEPROM.commit();
}

void setChannel(midi::Channel ch) {
    SerialMIDI.setInputChannel(ch);
    USBMIDI.setInputChannel(ch);
    saveChannel(ch);
}

void handleCC(byte channel, byte number, byte value) {
    if (number == CC_RELAY_A || number == CC_RELAY_B) {
        int offset = (number == CC_RELAY_A) ? 0 : 5;
        for (int i = 0; i < 5; i++)
            digitalWrite(RELAY_BASE + offset + i, (value >> i) & 1);
    }
}

void handleSysEx(byte *data, unsigned size) {
    // MIDI library passes data starting with F0, without trailing F7
    if (size >= 4 && data[0] == 0xF0 && data[1] == SYSEX_MFR_ID &&
        data[2] == SYSEX_CMD_SET_CHANNEL) {
        uint8_t ch = data[3] + 1; // 0x00-0x0F -> 1-16
        if (ch >= 1 && ch <= 16)
            setChannel((midi::Channel)ch);
    }
}

void setup() {
    for (int pin = 10; pin <= 29; pin++) {
        pinMode(pin, OUTPUT);
    }

    midi::Channel ch = loadChannel();

    Serial2.setRX(9);
    SerialMIDI.begin(ch);
    SerialMIDI.turnThruOff();

    USBMIDI.begin(ch);
    USBMIDI.turnThruOff();
    SerialMIDI.setHandleControlChange(handleCC);
    USBMIDI.setHandleControlChange(handleCC);
    USBMIDI.setHandleSystemExclusive(handleSysEx);
}

void loop() {
    SerialMIDI.read();
    USBMIDI.read();
}
