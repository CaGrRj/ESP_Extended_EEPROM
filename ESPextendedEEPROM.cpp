#include "Arduino.h"
#include "ESPextendedEEPROM.h"

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}

extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;

ESPextendedEEPROMClass::ESPextendedEEPROMClass(void)
: _offset((((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE))
, _data(0)
, _size(0)
, _dirty(false)
{
}

void ESPextendedEEPROMClass::begin() {
  _size = ((((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE) - (((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE));

  //In case begin() is called a 2nd+ time, don't reallocate if size is the same
  if(!_data) {
    _data = new uint8_t[SPI_FLASH_SEC_SIZE];
  }

  _dirty = false; //make sure dirty is cleared in case begin() is called 2nd+ time
}

bool ESPextendedEEPROMClass::selectSector(const uint32_t sector) {
  if (sector < 0) return false;
  if (sector >= _size) return false;

  if (_dirty)
  commit();

  _sector = sector + _offset;

  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), SPI_FLASH_SEC_SIZE);
  interrupts();
  
  return true;
}

void ESPextendedEEPROMClass::end() {
  if (!_size)
    return;

  commit();
  
  if(_data) {
    delete[] _data;
  }
  _data = 0;
  _size = 0;
  _dirty = false;
}


byte ESPextendedEEPROMClass::read(const uint32_t address) {
  if (address < 0 || (size_t)address >= SPI_FLASH_SEC_SIZE)
    return 0;
  if(!_data)
    return 0;

  return _data[address];
}

void ESPextendedEEPROMClass::write(const uint32_t address, const byte value) {
  if (address < 0 || (size_t)address >= SPI_FLASH_SEC_SIZE)
    return;
  if(!_data)
    return;

  // Optimise _dirty. Only flagged if data written is different.
  uint8_t* pData = &_data[address];
  if (*pData != value)
  {
    *pData = value;
    _dirty = true;
  }
}

bool ESPextendedEEPROMClass::commit() {
  bool ret = false;
  if (!_size)
    return false;
  if(!_dirty)
    return true;
  if(!_data)
    return false;

  noInterrupts();
  if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
    if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), SPI_FLASH_SEC_SIZE) == SPI_FLASH_RESULT_OK) {
      _dirty = false;
      ret = true;
    }
  }
  interrupts();

  return ret;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
ESPextendedEEPROMClass extEEPROM;
#endif