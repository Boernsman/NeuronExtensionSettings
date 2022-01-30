# Neuron Extension Settings

Discover Neuron extension on the Rs-485 bus and change baudraute, parity and address.

## Install

For 32 Bit Raspbian. 

~$ sudo apt update && sudo apt upgrade -y
~$ sudo apt install qt5-default libqt5serialbus5 libqt5serialport5
~$ wget TODO

## Usage

## Discovery

~$ sudo ./neuronextensionsettings --discover

In this mode the settings are like the same as unconfigured Neuron extensions.
Select the serial port where the neuron connections are connected to, usually "ttyNS0".
The application starts to discover all Neuron extension devices on the bus.

## Write settings

~$ sudo ./neuronextensionsettings --write --wb=115200 --wa=1 --wp=even

This command writes the settings baudrate 115200, address 1 and parity even to an unconfigured Neuron extesion. After a successfull write the extension needs to be restarted.

TO change the settings all DIP switches must be set 'off'. Only one unconfigured Neuron extension at a time is allowed per bus.

## More

Use the help option to display all available options

~$ ./neuroextensionsettings --help
