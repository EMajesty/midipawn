# midipawn

MIDI-controlled relay switcher built on the Raspberry Pi Pico (RP2040). Accepts MIDI over USB and 5-pin DIN (GPIO 9). Controls 10 bypass relays via GPIO 10-19.

## MIDI interface

The device accepts messages on both USB MIDI and serial MIDI (DIN input on GPIO 9). Default MIDI channel is **6**.

### Relay control

Relay states are set via two CC messages. Each CC value is a 5-bit bitmask where bit 0 is the lowest-numbered relay in the group.

| CC  | Bit 0 | Bit 1 | Bit 2 | Bit 3 | Bit 4 |
|-----|-------|-------|-------|-------|-------|
| 80  | Relay 1 (GPIO 10) | Relay 2 (GPIO 11) | Relay 3 (GPIO 12) | Relay 4 (GPIO 13) | Relay 5 (GPIO 14) |
| 81  | Relay 6 (GPIO 15) | Relay 7 (GPIO 16) | Relay 8 (GPIO 17) | Relay 9 (GPIO 18) | Relay 10 (GPIO 19) |

Example: sending CC 80 with value `0x15` (binary `10101`) activates relays 1, 3, 5 and deactivates relays 2, 4.

### Channel configuration

The MIDI channel is configurable via SysEx over USB and persists across power cycles (stored in flash).

```
F0 7D 01 <channel> F7
```

`<channel>` is `0x00`-`0x0F` for MIDI channels 1-16.

Example: `F0 7D 01 05 F7` sets the device to channel 6 (the default).

## Building

Requires [PlatformIO](https://platformio.org/).

```
cd firmware
pio run
```

## Flashing

Hold the BOOTSEL button on the Pico while plugging in USB, then:

```
cd firmware
pio run -t upload
```
