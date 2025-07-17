#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

uint32_t fnv1a32(const char *string) {
    uint32_t hash = FNV_OFFSET;
    size_t idx = 0;
    while (string[idx] != '\0') {
        hash = FNV_PRIME * (hash ^ (uint8_t)string[idx++]);
    }
    return hash;
}

bool allocHashMap(HashMap **map, const size_t slot_count) {
    Slot *alloc_slots = (Slot *)calloc(slot_count, sizeof(Slot));
    if (!alloc_slots) {
        return false;
    }
    HashMap *alloc_table = (HashMap *)calloc(1, sizeof(HashMap));
    if (!alloc_table) {
        free(alloc_slots);
        return false;
    }
    alloc_table->slots = alloc_slots;
    alloc_table->capacity = slot_count;
    *map = alloc_table;
    return true;
}

bool insertHashMap(HashMap *t, const char *key, const void *value, size_t value_size) {
    uint32_t hash = fnv1a32(key);
    /* Hash the initial bucket*/
    size_t idx = hash & (t->capacity - 1);
    for (size_t probe = 0; probe < t->capacity; probe++) {
        Slot *slot = &t->slots[idx];
        if (slot->key == NULL) {
            /*  Allocate new string with key */
            slot->key = strdup(key);
            if (!slot->key) {
                return false;
            }
            slot->hash = hash;
            slot->value = malloc(value_size);
            if (!slot->value) {
                free(slot->key);
                return false;
            }
            memcpy(slot->value, value, value_size);
            slot->value_size = value_size;
            t->size++;
            return true;
        } else if (slot->hash == hash && strcmp(slot->key, key) == 0) {
            /* Already exiasts */
            if (value_size != slot->value_size) {
                /* Inserted different types, not allowed */
                return false;
            }
            /* Don't allow duplicate entries */
            // memcpy(slot->value, value, value_size);
            return false;
        }
        /* On collision linear probe next idx */
        idx = (idx + 1) & (t->capacity - 1);
    }
    /* Table is full */
    // TODO: Resize / handle gracefully
    return false;
}

bool searchHashMap(HashMap *t, const char *key, void *out_value, size_t value_size) {
    uint32_t hash = fnv1a32(key);
    size_t idx = hash & (t->capacity - 1);
    for (size_t probe = 0; probe < t->capacity; probe++) {
        Slot *slot = &t->slots[idx];
        if (slot->key == NULL) {
            /* First encouter is a NULL means key is not in the table
               Based on the assumption no deletions are allowed, which is the case */
            return false;
        }
        if (slot->hash == hash && strcmp(slot->key, key) == 0) {

            if (value_size != slot->value_size) {
                /* Inserted different types, not allowed */
                return false;
            }
            memcpy(out_value, slot->value, value_size);
            return true;
        }
        /* On collision linear probe next idx */
        idx = (idx + 1) & (t->capacity - 1);
    }
    /* Worst case, full table iterated and no find */
    return false;
}

void *getPointerInHashMap(HashMap *t, const char *key) {
    uint32_t hash = fnv1a32(key);
    size_t idx = hash & (t->capacity - 1);
    for (size_t probe = 0; probe < t->capacity; probe++) {
        Slot *slot = &t->slots[idx];
        if (slot->key == NULL) {
            return NULL;
        }
        if (slot->hash == hash && strcmp(slot->key, key) == 0) {

            return slot->value;
        }
        /* On collision linear probe next idx */
        idx = (idx + 1) & (t->capacity - 1);
    }
    /* Worst case, full table iterated and no find */
    return NULL;
}

void deallocHashMap(HashMap *t) {
    for (size_t idx = 0; idx < t->capacity; idx++) {
        free(t->slots[idx].key);
        free(t->slots[idx].value);
    }
    free(t->slots);
    free(t);
}