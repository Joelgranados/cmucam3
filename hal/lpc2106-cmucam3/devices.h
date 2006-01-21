#ifndef INC_DEVICE_TABLE_H
#define INC_DEVICE_TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <reent.h>

	// describes what is found at driver level.
typedef const struct {
	const char	*name;
		// constant to define device type: mmc, etc.
	uint16_t	device_type;
		// device methods for newlib interface.
	int (*open)(struct _reent *r,const char *name, int flags, int mode);
	int (*close)(struct _reent *r,int file);
	_ssize_t (*read)(struct _reent *r,int file, void *ptr, size_t len);
	_ssize_t (*write)(struct _reent *r,int file, const void *ptr, size_t len);
		// init the device / software layers.
	int (*init)(void);
		// and the venerable catch-22...
	int (*ioctl)(struct _reent *r, int file, int cmd, void *ptr);
} DEVICE_TABLE_ENTRY;

	// device number is high byte of FILE "pointer".
#define	DEVICE(D)	(D << 8)
#define	DEVICE_TYPE(D)	((D >> 8) & 0xff)

typedef const struct {
	const DEVICE_TABLE_ENTRY * item;
} DEVICE_TABLE_ARRAY;

typedef const DEVICE_TABLE_ARRAY * DEVICE_TABLE_LIST;


#endif

