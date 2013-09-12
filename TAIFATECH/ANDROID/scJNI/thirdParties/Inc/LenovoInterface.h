#ifndef __LENOVO_HWJPEGENC_H__
#define __LENOVO_HWJPEGENC_H__

#include <sys/resource.h>
#include <cutils/sched_policy.h>

#define LIB_NAME_LENOVO "/system/lib/libhwjpeg.so"
#define CM_CONTROL_LENOVO "/dev/cm_control"
#define CM_CHANNEL_LENOVO "/dev/cm_channel"

#define JPEG_QUALITY 85

typedef enum tagLEN_ROTATION
{
	LENOVO_ROTATE_0 = 0,
	LENOVO_ROTATE_90 = 90,
	LENOVO_ROTATE_180 = 180,
	LENOVO_ROTATE_270 = 270
}LEN_ROTATION;

#include <stdint.h>
#include <strings.h>
#include <sys/cdefs.h>
#include <sys/types.h>
//#include <cutils/bitops.h>

//__BEGIN_DECLS
#define HWJPEG_MODULE_TAG MAKE_TAG_CONSTANT('H', 'W', 'J', 'P')
#define HWJPEG_MODULE_ID "hwjpeg"
#define HWJPEG_MODULE_INTERFACE "hwjpeg_if"

#define HAL_INFO_SYM         HMJPEG
#define HAL_INFO_SYM_STR         "HMJPEG"

namespace android {

typedef struct hwjepgapi_device_t {
	/** tag must be initialized to HARDWARE_DEVICE_TAG */    
	uint32_t tag;    
	/** version number for hw_device_t */    
	uint32_t version;    
	/** reference to the module this device belongs to */    
	struct hwjepgapi_module_t* module;
	/** padding reserved for future use */    
	uint32_t reserved[12];    
	/** Close this device */    
	int (*close)(struct hwjepgapi_device_t* device);
} hwjepgapi_device_t;


typedef struct hwjepgapi_module_methods_t {
	/** Open a specific device */    
	int (*open)(const struct hwjepgapi_module_t* module, const char* id,
		struct hwjepgapi_device_t** device,int quality);
} hwjepgapi_module_methods_t;


typedef struct hwjepgapi_module_t {
	/** tag must be initialized to HARDWARE_MODULE_TAG */    
	uint32_t tag;    
	/** major version number for the module */    
	uint16_t version_major;    
	/** minor version number of the module */    
	uint16_t version_minor;    
	/** Identifier of module */    
	const char *id;    
	/** Name of this module */    
	const char *name;    
	/** Author/owner/implementor of the module */    
	const char *author;    
	/** Modules methods */    
	struct hwjepgapi_module_methods_t* methods;
	/** module's dso */    
	void* dso;    
	/** padding to 128 bytes, reserved for future use */    
	uint32_t reserved[32-7];
} hwjepgapi_module_t;

struct hwjpeg_module {
    struct hwjepgapi_module_t common;
};

struct hwjepgapi_device {
    struct hwjepgapi_device_t common;

	int (*encode)(const struct hwjepgapi_device *dev,char**outBuffer, int* outSize,int rotation,int64_t * timeStamp);
	int (*getHWResolution)(const struct hwjepgapi_device *dev, int* width, int *height);    
};

typedef struct hwjepgapi_device jpeg_hw_device_t;

static inline int jpeg_hw_device_open(const struct hwjepgapi_module_t* module,
                                       struct hwjepgapi_device** device,int rotation)
{    
	return module->methods->open(module, HWJPEG_MODULE_INTERFACE,
		(struct hwjepgapi_device_t**)device,rotation);
}
static inline int jpeg_hw_device_close(struct hwjepgapi_device* device)
{
	return device->common.close(&device->common);
}

};

#endif /*__LENOVO_HWJPEGENC_H__*/