# Hiring Firmware Skeleton C Project

A firmware skeleton C project which serves as the basis for a technical interview.

The firmware is for a hypothetical device which can take temperature measurements and transmit them upstream.
The hypothetical device has a DS18B20 for temperature measurement and an imaginary radio peripheral for communication.

## What to expect

The interview will consist of a 1.5h pair-programming session in which you will make changes to this codebase based on problems we will give you.
We'll be exploring this with you, so it should be very much a dialogue, but you'll be doing most of the programming 😉.
Please make sure you have at least sorted your development environment beforehand, and are able to share your screen on Zoom.
We'd suggest closing other windows not relating to this challenge as we would like to see the whole screen -- sometimes you might need to look something up, and we'd like to have context of that thing too!

## Project Structure

```
.
├── flake.nix              # Nix flake configuration
├── CMakeLists.txt         # CMake build configuration
├── src/
│   └── main.c            # Main C source file
├── vendor/
│   ├── ds18b20_driver.h  # DS18B20 temperature sensor driver API
│   ├── ds18b20_driver.c  # ... and mock implementation
│   ├── radio_driver.h    # Wireless radio transceiver driver API
│   ├── radio_driver.c    # ... and mock implementation
│   ├── microcontroller.h # MCU API
│   └── microcontroller.c # ... and mock implementation
├── .gitignore            # Git ignore rules
└── README.md             # This file
```

## Building and Running

### Using Nix (Recommended)

Build the project:
```bash
nix build
```

Run the executable:
```bash
./result/bin/hiring-firmware-skeleton-c
```

### Using the Development Shell

Enter the development environment:
```bash
nix develop
```

Then build manually with CMake:
```bash
mkdir build && cd build
cmake ..
make
./hiring-firmware-skeleton-c
```

## Features

- **Reproducible builds** with Nix flakes
- **Modern CMake** configuration (minimum version 3.20)
- **Clang compiler** with useful warning flags
- **C17 standard** compliance
- **Development shell** with debugging tools (gdb, valgrind)
- **DS18B20 sensor integration** with vendor-supplied driver API
- **Wireless radio module** with comprehensive driver API
- **Organized project structure** with separate vendor directory

## Hardware Components

### DS18B20 Temperature Sensor
- 1-Wire digital temperature sensor
- Configurable resolution (9-bit to 12-bit)
- Temperature range: -55°C to +125°C
- Parasitic or external power modes
- Built-in CRC validation

### Wireless Radio Module
- Multi-frequency operation (configurable)
- Multiple modulation schemes (FSK, GFSK, LoRa, OOK)
- Advanced power management (Sleep/Standby/Active)
- Packet-based communication with auto-retry
- Network security (WEP/WPA/AES encryption)
- Signal strength monitoring (RSSI)
- Maximum payload: 246 bytes per packet

## Development Tools

The development shell includes:
- `cmake` - Build system
- `clang` - C compiler
- `gdb` - GNU debugger
- `valgrind` - Memory debugging tool

## Requirements

- Nix with flakes enabled
- That's it! All other dependencies are managed by Nix.

## License

MIT License
