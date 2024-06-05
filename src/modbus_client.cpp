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

#include "modbus_client.h"

#include <iostream>

ModbusClient::ModbusClient(const std::string &port_name, int baudrate, Parity parity)
{
  char parity_char;
  switch (parity) {
    case Parity::None:
      parity_char = 'N';
      break;
    case Parity::Odd:
      parity_char = 'O';
      break;
    case Parity::Even:
      parity_char = 'E';
      break;
  }
  mb_ = modbus_new_rtu(port_name.c_str(), baudrate, parity_char, 8, 1);
  if (!mb_) {
    throw std::runtime_error("Could not create Modbus RTU: " + std::string(modbus_strerror(errno)));
  }
}

ModbusClient::~ModbusClient()
{
  if (mb_) {
    modbus_close(mb_);
    modbus_free(mb_);
  }
}

void ModbusClient::connect()
{
  if (modbus_connect(mb_) == -1) {
    throw std::runtime_error("Could not connect: " + std::string(modbus_strerror(errno)));
  }
}

void ModbusClient::setSlave(uint address)
{
  if (modbus_set_slave(mb_, address) == -1) {
    throw std::runtime_error("Could not set slave address: " + std::string(modbus_strerror(errno)));
  }
}

uint16_t ModbusClient::readRegister(uint address)
{
  uint16_t value;
  if (modbus_read_registers(mb_, address, 1, &value) == -1) {
    throw std::runtime_error(modbus_strerror(errno));
  }
  return value;
}

void ModbusClient::writeRegister(uint address, uint16_t value)
{
  if (modbus_write_register(mb_, address, value) == -1) {
    throw std::runtime_error(modbus_strerror(errno));
  }
}

void ModbusClient::writeCoil(uint address, bool value)
{
  if (modbus_write_bit(mb_, address, value) == -1) {
    throw std::runtime_error(modbus_strerror(errno));
  }
}

std::vector<uint16_t> ModbusClient::readRegisters(uint address, int count)
{
  std::vector<uint16_t> values(count);
  if (modbus_read_registers(mb_, address, count, values.data()) == -1) {
    throw std::runtime_error(modbus_strerror(errno));
  }
  return values;
}

void ModbusClient::writeRegisters(uint address, const std::vector<uint16_t> &values)
{
  if (modbus_write_registers(mb_, address, values.size(), values.data()) == -1) {
    throw std::runtime_error(modbus_strerror(errno));
  }
}
