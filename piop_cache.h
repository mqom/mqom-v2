#ifndef __PIOP_CACHE_H__
#define __PIOP_CACHE_H__

#include <stdlib.h>
#include <fields.h>

/**** PIOP cache handling functions *********/

/*
 * This is a cache to factorize t1 computation across all the tau
 * repetitions.
 */
typedef struct {
        uint8_t active;
        field_base_elt t1[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
} piop_cache;

/* Function that deals with the PIOP cache */
static inline void destroy_piop_cache(piop_cache *cache)
{   
        if(cache != NULL){
		free(cache);
        }
}

static inline uint8_t is_entry_active(const piop_cache *cache, uint32_t i){
	return (cache != NULL) ? cache[i].active : 0;
}
static inline void get_entry(const piop_cache *cache, uint32_t i, field_base_elt t1[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)]){
	memcpy(t1, cache[i].t1, FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N));
}
static inline void set_entry(piop_cache *cache, uint32_t i, const field_base_elt t1[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)]){
	if(cache != NULL){	
		memcpy(cache[i].t1, t1, FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N));
		cache[i].active = 1;
	}
}

static inline piop_cache *init_piop_cache(uint32_t num_elt)
{
        (void)num_elt;
	piop_cache *cache = NULL;

#ifdef USE_PIOP_CACHE
	cache = (piop_cache*)calloc(num_elt, sizeof(piop_cache));
	if(cache == NULL){
		goto err;
	}
err:
#endif
        /* NOTE: when USE_PIOP_CACHE is not defined,
         * cache is NULL which is equivalent to not using a cache
         * */
        return cache;
}
#endif
