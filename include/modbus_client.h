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

#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <modbus.h>

#include <stdexcept>
#include <string>
#include <vector>

class ModbusClient {
 public:
  enum Parity { Even, Odd, None };

  explicit ModbusClient(const std::string &serial_port, int baudrate, Parity parity);
  ~ModbusClient();

  void connect();
  void setSlave(uint address);
  uint16_t readRegister(uint address);
  void writeRegister(uint address, uint16_t value);
  void writeCoil(uint address, bool value);

  std::vector<uint16_t> readRegisters(uint address, int count);
  void writeRegisters(uint address, const std::vector<uint16_t> &values);

  std::string error() { return modbus_strerror(errno); }

 private:
  modbus_t *mb_;
};

#endif  // MODBUSCLIENT_H
