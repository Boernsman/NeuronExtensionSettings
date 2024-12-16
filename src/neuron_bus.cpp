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
#include <ranges>

#include "debug.h"

void NeuronBus::writeSettings(uint address, const NeuronBus::DeviceSettings &settings)
{
  debug() << "Writing settings";
  if (settings.address < 1 || settings.address > 255) {
    throw std::runtime_error("Invalid device address [1, 255]");
  }
  std::vector<u_int16_t> registers(2, 0);

  uint16_t valueReg0 = 0x0000;         // Register 1027
  registers.at(1) = settings.address;  // Register 1028

  registers.at(0) = settings.baudrate;  // first 12 bits

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
    debug() << "Model xS10";
    return DeviceType::xS10;
  } else if (registerValue == 784) {
    debug() << "Model xS30";
    return DeviceType::xS30;
  } else if (registerValue == 528) {
    debug() << "Model xS40";
    return DeviceType::xS40;
  } else if (registerValue == 5) {
    debug() << "Model xS50";
    return DeviceType::xS50;
  } else if (registerValue == 272) {
    debug() << "Model xS11";
    return DeviceType::xS11;
  } else if (registerValue == 273) {
    debug() << "Model xS51";
    return DeviceType::xS51;
  } else {
    debug() << "Unkown model";
    return std::nullopt;
  }
}

std::map<uint, NeuronBus::DeviceType> NeuronBus::discoverDevices(uint startAddress, uint endAddress)
{
  std::map<uint, DeviceType> devices;
  for (uint address = startAddress; address < endAddress; address++) {
    client_->setSlave(address);
    auto result = client_->readRegisters(1000, 7);
    std::cout << "Probing address" << address << std::endl;

    if (result.empty()) {
      continue;
    }

    auto deviceType = getDeviceType(result.at(4));
    if (!deviceType) {
      continue;
    }
    devices[address] = deviceType.value();
  }
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