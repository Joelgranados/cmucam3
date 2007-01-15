#include "cc3.h"
#include "devices.h"

static _cc3_device_driver_t *device_driver_table[_CC3_NUM_DEVICES] = {
  [_CC3_DEVICE_UART] = &_cc3_uart_driver
};

void cc3_filesystem_init (void)
{
  // put MMC into the table
  device_driver_table[_CC3_DEVICE_MMC] = &_cc3_mmc_driver;
}

_cc3_device_driver_t *_cc3_get_driver_for_name (const char *name)
{
  int i;
  for (i = 0; i < _CC3_NUM_DEVICES; i++) {
    _cc3_device_driver_t *dev = device_driver_table[i];
    if (dev != NULL && dev->recognize(name)) {
      return dev;
    }
  }
  return NULL;
}

uint8_t _cc3_get_internal_file_number (const int file)
{
  return file & 0xFF;
}

_cc3_device_driver_t *_cc3_get_driver_for_file_number (const int file)
{
  int d = (file >> 8) & 0xFF;

  if (d > _CC3_NUM_DEVICES) {
    return NULL;
  }
  return device_driver_table[d];
}

int _cc3_make_file_number (const _cc3_device_driver_t *dev,
			   const int16_t file)
{
  int result = file;
  if (file >= 0) {
    result = (dev->id << 8) | file;
  }
  return result;
}
