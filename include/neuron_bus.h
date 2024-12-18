/*
 * This file is part of the Neuron Extension Settings application.
 * Copyright (c) 2024 Bernhard Trinnes.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NEURON_BUS_H
#define NEURON_BUS_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "modbus_client.h"

class NeuronBus {
 public:
  enum DeviceType { xS10 = 1, xS30 = 784, xS40 = 528, xS50 = 5, xS11 = 272, xS51 = 273 };

  enum Baudrate {
    Baudrate_2400 = 11,
    Baudrate_4800 = 12,
    Baudrate_9600 = 13,
    Baudrate_19200 = 14,
    Baudrate_38400 = 15,
    Baudrate_57600 = 4097,
    Baudrate_115200 = 4098
  };

  struct TestResult {
    uint cycles;
    uint errors;
    double avarage_response_time;
  };

  struct DeviceSettings {
    int address;
    Baudrate baudrate;
    ModbusClient::Parity parity;
  };

  explicit NeuronBus(ModbusClient *client) : client_(client) {};

  TestResult test(uint address, uint cycles);
  static DeviceSettings createDeviceSettings(int address, int baudrate, const std::string &parity);
  void writeSettings(uint address, const DeviceSettings &settings);
  static std::optional<DeviceType> getDeviceType(uint16_t registerValue);
  static std::string getDeviceTypeString(DeviceType deviceType);
  std::map<uint, DeviceType> discoverDevices(uint startAddress = 1, uint endAddress = 15);

 private:
  ModbusClient *client_ = nullptr;
  std::optional<uint16_t> getRegisterAddress(uint address);
};

#endif  // NEURON_BUS_H
