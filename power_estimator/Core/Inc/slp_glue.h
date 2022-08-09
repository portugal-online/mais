#ifndef SLPGLUE_H__
#define SLPGLUE_H__

#include <pthread.h>

typedef pthread_mutex_t slp_lock_t;

#define SLP_LOCK_INIT(x)
#define SLP_LOCK(x)
#define SLP_UNLOCK(x)

#endif
