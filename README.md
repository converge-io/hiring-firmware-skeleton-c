# Hiring Firmware Skeleton C Project

A firmware skeleton C project demonstrating a reproducible build setup using Nix flakes, CMake, and clang.

## Project Structure

```
.
├── flake.nix              # Nix flake configuration
├── CMakeLists.txt         # CMake build configuration
├── src/
│   └── main.c            # Main C source file
├── vendor/
│   ├── ds18b20_driver.h  # DS18B20 temperature sensor driver API
│   └── radio_driver.h    # Wireless radio transceiver driver API
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
