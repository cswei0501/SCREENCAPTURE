#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void ( * EventCallback_t)(int msg, int ext1, int ext2, void * userData);

int wfd_start(const char * url, void * surface, EventCallback_t callback, void * userData);
void wfd_stop(void);

#ifdef __cplusplus
}
#endif

