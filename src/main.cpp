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

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "debug.h"
#include "modbus_client.h"
#include "neuron_bus.h"

namespace po = boost::program_options;

std::optional<ModbusClient::Parity> stringToParity(const std::string &parity_string)
{
  if (parity_string == "even") {
    return ModbusClient::Parity::Even;
  } else if (parity_string == "odd") {
    return ModbusClient::Parity::Odd;
  } else if (parity_string == "none") {
    return ModbusClient::Parity::None;
  } else {
    return std::nullopt;
  }
}

int main(int argc, char *argv[])
{
  std::string port_name;
  std::string command;
  std::string parity_string;
  int baudrate = 19200;
  uint address = 15;

  int w_baudrate = 19200;
  uint w_address = 15;
  std::string w_parity_string;

  try {
    po::options_description desc("Neuron Extension Bus Settings\nOptions");
    desc.add_options()("help,h", "Show help message")("command", po::value(&command)->required(),
                                                      "Command: test, discover, write")(
        "serial,s", po::value(&port_name)->required(), "Set the serial port")(
        "baud,b", po::value(&baudrate)->default_value(19200), "Set the baudrate")(
        "address,a", po::value(&address)->default_value(15), "Set the slave address")(
        "parity,p", po::value(&parity_string)->default_value("none"), "Set parity (even|none|odd)")(
        "wbaud", po::value(&w_baudrate)->default_value(19200), "Write baudrate for write option")(
        "wparity", po::value(&w_parity_string)->default_value("none"), "Write parity (even|none|odd) for write option")(
        "waddress", po::value(&w_baudrate)->default_value(19200), "Write address");

    po::positional_options_description p;
    p.add("command", 1);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error parsing command line arguments: " << e.what() << std::endl;
    return -1;
  }

  try {
    auto parity = stringToParity(parity_string);
    if (!parity) {
      throw;
    }
    auto client = ModbusClient{port_name, baudrate, parity.value()};
    client.connect();
    client.setSlave(address);
    auto bus = NeuronBus{&client};

    int testCycles = 100;
    if (command == "test") {
      std::cout << "Testing...\n";
      auto test_result = bus.test(address, testCycles);
      std::cout << "Test finished" <<
      "\n  - Error count: " << test_result.errors <<
      "\n  - Average response time: " << test_result.avarage_response_time <<
      "\n  - Cycles: " << test_result.cycles << std::endl;

    } else if (command == "discover") {
      const auto endAddress = 15;
      const auto startAddress = 1;
      std::cout << "Discovering, from " << startAddress << " to " << endAddress << std::endl;
      auto devices = bus.discoverDevices(startAddress, endAddress);
      if (devices.empty()) {
        std::cout << "Found no devices" << std::endl;
      } else {
        for (const auto &[address, device_type] : devices) {
          std::cout << "  - Device " << device_type << " address: " << address << std::endl;
        }
      }
    } else if (command == "write") {
      NeuronBus::DeviceSettings settings;
      settings.baudrate = NeuronBus::Baudrate::Baudrate_19200;
      settings.parity = stringToParity(w_parity_string).value_or(ModbusClient::None);
      settings.address = 1;
      std::cout << "Writting settings to device address: " << address << std::endl;
      bus.writeSettings(address, settings);
      std::cout << "Settings have been written successfully" << std::endl;

    } else {
      throw std::invalid_argument{"Invalid command. Must be 'test', 'discover' or 'write',"};
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}