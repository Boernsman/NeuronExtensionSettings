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

#include "neuron_bus.h"

#include <chrono>
#include <iostream>
#include <ranges>

void NeuronBus::writeSettings(uint address, const NeuronBus::DeviceSettings &settings)
{
  client_->setSlave(address);

  std::cout << "Writing settings" << std::endl;
  if (address < 1 || address > 255) {
    throw std::runtime_error("Invalid device address [1, 255]");
  }
  std::vector<u_int16_t> registers(2, 0);

  uint16_t valueReg0 = 0x0000;          // Register 1027
  registers.at(1) = settings.address;   // Register 1028
  registers.at(0) = settings.baudrate;  // First 12 bits

  if (settings.parity != ModbusClient::Parity::None) {
    registers.at(0) |= (1 << 13);
    if (settings.parity == ModbusClient::Parity::Odd) {
      registers.at(0) |= (1 << 14);
    }
  }
  auto register_address = getRegisterAddress(address);
  if (!register_address) {
    throw std::runtime_error("Could not gather register address");
  }
  client_->writeRegisters(register_address.value(), registers);
  client_->writeCoil(1003, 0x0001);
}

NeuronBus::DeviceSettings NeuronBus::createDeviceSettings(int address, int baudrate, const std::string &parity)
{
  DeviceSettings settings;
  if (address < 1 || address > 255) {
    throw std::runtime_error("Invalid device address [1, 255]");
  }
  settings.address = address;

  switch (baudrate) {
    case 2400:
      settings.baudrate = Baudrate_2400;
      break;
    case 4800:
      settings.baudrate = Baudrate_4800;
      break;
    case 9600:
      settings.baudrate = Baudrate_9600;
      break;
    case 19200:
      settings.baudrate = Baudrate_19200;
      break;
    case 38400:
      settings.baudrate = Baudrate_38400;
      break;
    case 57600:
      settings.baudrate = Baudrate_57600;
      break;
    case 115200:
      settings.baudrate = Baudrate_115200;
      break;
    default:
      throw std::runtime_error("Invalid baud rate setting");
  }

  if (parity == "even") {
    settings.parity = ModbusClient::Parity::Even;
  } else if (parity == "odd") {
    settings.parity = ModbusClient::Parity::Odd;
  } else if (parity == "none") {
    settings.parity = ModbusClient::Parity::None;
  } else {
    throw std::runtime_error("Invalid parity string, must be 'even', 'odd' or 'none'");
  }

  return settings;
}

std::optional<uint16_t> NeuronBus::getRegisterAddress(uint address)
{
  client_->setSlave(address);
  uint16_t result = client_->readRegister(1004);

  auto deviceType = getDeviceType(result);

  if (!deviceType) {
    return std::nullopt;
  }

  switch (deviceType.value()) {
    case DeviceType::xS10:
      return 1031;
    case DeviceType::xS11:
      return 1027;
    case DeviceType::xS30:
      return 1034;
    case DeviceType::xS40:
      return 1023;
    case DeviceType::xS50:
      return 1023;
    case DeviceType::xS51:
      return 1023;
    default:
      return std::nullopt;
  }
}

std::optional<NeuronBus::DeviceType> NeuronBus::getDeviceType(uint16_t registerValue)
{
  if (registerValue == 1) {
    return DeviceType::xS10;
  } else if (registerValue == 784) {
    return DeviceType::xS30;
  } else if (registerValue == 528) {
    return DeviceType::xS40;
  } else if (registerValue == 5) {
    return DeviceType::xS50;
  } else if (registerValue == 272) {
    return DeviceType::xS11;
  } else if (registerValue == 273) {
    return DeviceType::xS51;
  } else {
    return std::nullopt;
  }
}

std::string NeuronBus::getDeviceTypeString(NeuronBus::DeviceType deviceType)
{
  switch (deviceType) {
    case DeviceType::xS10:
      return "xS10";
    case DeviceType::xS11:
      return "xS11";
    case DeviceType::xS30:
      return "xS30";
    case DeviceType::xS40:
      return "xS40";
    case DeviceType::xS50:
      return "xS50";
    case DeviceType::xS51:
      return "xS51";
  }
  return "unknown";
}

std::map<uint, NeuronBus::DeviceType> NeuronBus::discoverDevices(uint startAddress, uint endAddress)
{
  std::map<uint, DeviceType> devices;
  std::cout << "Starting discover devices. From address " << startAddress << " to " << endAddress << std::endl;

  for (int address : std::ranges::iota_view{startAddress, endAddress + 1}) {
    client_->setSlave(address);
    std::cout << address << " " << std::flush;
    try {
      auto result = client_->readRegisters(1000, 7);
      auto deviceType = getDeviceType(result.at(4));
      if (!deviceType) {
        continue;
      }
      devices[address] = deviceType.value();
    } catch (...) {
      continue;
    }
  }
  std::cout << std::endl;
  return devices;
}

NeuronBus::TestResult NeuronBus::test(uint address, uint cycles)
{
  TestResult result = {.cycles = cycles, .errors = 0u, .avarage_response_time = 0.00};

  client_->setSlave(address);
  std::cout << "Starting test. " << cycles << " cycles ('.'=OK, 'x'=Failed)" << std::endl;

  for (int i : std::ranges::iota_view{0u, cycles}) {
    try {
      const auto start_time = std::chrono::steady_clock::now();

      auto value = client_->readRegister(1004);
      std::cout << "." << std::flush;
      const std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now() - start_time;

      result.avarage_response_time += elapsed_seconds.count();
    } catch (...) {
      std::cout << "x" << std::flush;
      result.errors++;
    }
  }
  result.avarage_response_time /= (cycles - result.errors);
  std::cout << std::endl;
  return result;
}