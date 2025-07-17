#ifndef HASHMAP_H
#define HASHMAP_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define HASH_MAP_BUCKETS 512
#define FNV_OFFSET 2166136261
#define FNV_PRIME 16777619

typedef struct {
    char *key;
    uint32_t hash;
    void *value;
    size_t value_size;
} Slot;

typedef struct {
    Slot *slots;
    size_t size;
    size_t capacity;
} HashMap;

bool allocHashMap(HashMap **map, const size_t slot_count);
bool insertHashMap(HashMap *t, const char *key, const void *value, size_t value_size);
void deallocHashMap(HashMap *t);
void *getPointerInHashMap(HashMap *t, const char *key);

#endif