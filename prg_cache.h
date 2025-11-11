#ifndef __PRG_CACHE_H__
#define __PRG_CACHE_H__

#include <common.h>
#include <stdlib.h>
#include "enc.h"

/**** PRG cache handling functions *********/
#define CEIL(x,y) (((x) + (y) - 1) / (y))

/* This is a key schedule cache to factorize common key schedules across the repetitions
 * of 'e' and 'i' in the various calls to PRG.
 * NOTE: we can bound the size of the cache depending on the MQOM2 parameters.
 * */
typedef struct {
        uint8_t active;
        enc_ctx ctx;
	/* Size in bytes of the cache (tracked for free) */
	uint32_t size;
} prg_key_sched_cache;

/* Function that deals with the PRG cache */
static inline void destroy_prg_cache(prg_key_sched_cache *cache)
{ 
        if(cache != NULL){
		uint32_t i;
		/* Clean the enc contexts in all the active cells */
		for(i = 0; i < (cache->size / sizeof(prg_key_sched_cache)); i++){
			if(cache[i].active){
				enc_clean_ctx(&cache[i].ctx);
			}
		}
                mqom_free(cache, cache->size);
        }
}

static inline uint8_t is_entry_active_prg_cache(const prg_key_sched_cache *cache, uint32_t i){
	return (cache != NULL) ? cache[i].active : 0;
}

static inline void get_entry_prg_cache(const prg_key_sched_cache *cache, uint32_t i, enc_ctx *ctx){
	(*ctx) = cache[i].ctx;
}

static inline void invalidate_entry_prg_cache(prg_key_sched_cache *cache, uint32_t i){
	if((cache != NULL) && cache[i].active){
		enc_clean_ctx(&cache[i].ctx);
		cache[i].active = 0;
	}
}

static inline void set_entry_prg_cache(prg_key_sched_cache *cache, uint32_t i, const enc_ctx *ctx){
	if(cache != NULL){
		/* If the cell was already active from a previous run, clean the enc context */
		invalidate_entry_prg_cache(cache, i);
		cache[i].ctx = (*ctx);
		cache[i].active = 1;
	}
}

static inline prg_key_sched_cache *init_prg_cache(uint32_t n_bytes)
{
        (void)n_bytes;
	prg_key_sched_cache *prg_cache = NULL;

#ifdef USE_PRG_CACHE
	prg_cache = (prg_key_sched_cache*)mqom_calloc(CEIL(n_bytes, MQOM2_PARAM_SEED_SIZE), sizeof(prg_key_sched_cache));
	if(prg_cache == NULL){
		goto err;
	}
	prg_cache->size = CEIL(n_bytes, MQOM2_PARAM_SEED_SIZE) * sizeof(prg_key_sched_cache);
err:
#endif
        /* NOTE: when USE_PRG_CACHE is not defined,
         * prg_cache is NULL which is equivalent to not using a cache
         * */
        return prg_cache;
}
#endif
