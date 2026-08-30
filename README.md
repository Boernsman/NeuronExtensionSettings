# Neuron Extension Settings

[![CI](https://github.com/Boernsman/NeuronExtensionSettings/actions/workflows/release.yml/badge.svg)](https://github.com/Boernsman/NeuronExtensionSettings/actions/workflows/release.yml)

Discover Neuron extension available on the bus and change baud rate, parity and address.

## Requirements

* Linux (Debian/Ubuntu).
* CMake 3.19 or higher.
* A C++20 compatible compiler (GCC 11 or newer).
* pkg-config.
* libmodbus.
* Boost.ProgramOptions.

## Build

```
sudo apt update
sudo apt install git cmake g++ pkg-config libmodbus-dev libboost-program-options-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The binary is written to `build/neuron_extension_settings`.

Alternatively, the bundled CMake presets (Ninja based) can be used:

```
cmake --preset release
cmake --build --preset app-release
```

## Usage

### Discovery

```
sudo ./build/neuron_extension_settings discover --serial /dev/ttyNS0
```

In this mode the settings are like the same as unconfigured Neuron extensions.
Select the serial port where the neuron connections are connected to, usually "ttyNS0".
The application starts to discover all Neuron extension devices on the bus.

### Test

```
sudo ./build/neuron_extension_settings test --serial /dev/ttyNS0
```


### Write settings

```
sudo ./build/neuron_extension_settings write --serial /dev/ttyNS0 --w_baud=115200 --w_address=1 --w_parity=even
```

This command writes the settings baudrate 115200, address 1 and parity even to an unconfigured Neuron extesion. After a successfull write the extension needs to be restarted.

To change the settings all DIP switches must be set 'off'. Only one unconfigured Neuron extension at a time is allowed per bus.

### More

Use the help option to display all available options

```
./build/neuron_extension_settings --help
```

## License

This project is licensed under the GNU General Public License v3.0. You are free to use, modify, and distribute this software under the terms of the GPLv3 license.

A copy of the license is included in this repository as `LICENSE`, or you can review it at [https://www.gnu.org/licenses/gpl-3.0.en.html](https://www.gnu.org/licenses/gpl-3.0.en.html).

### Key Points of the GPLv3 License:
- You are allowed to modify and distribute the software, but you must retain the same license.
- If you distribute modified versions, you must provide access to the source code.
- There is no warranty for the software.

For more details, consult the full license text.
