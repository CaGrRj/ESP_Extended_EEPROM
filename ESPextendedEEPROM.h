#ifndef ESPextendedEEPROM_h
#define ESPextendedEEPROM_h

#include <stddef.h>
#include <stdint.h>
#include <string.h>

class ESPextendedEEPROMClass {
public:
  ESPextendedEEPROMClass(void);

  void begin();
  bool selectSector(const uint32_t sector);
  void end();
  byte read(const uint32_t address);
  void write(const uint32_t address, const byte val);
  bool commit();

  size_t length() {return _size;}

protected:
  uint32_t _offset;
  uint32_t _sector;
  uint8_t* _data;
  size_t _size;
  bool _dirty;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern ESPextendedEEPROMClass extEEPROM;
#endif

#endif