#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int hm_check_mem(void *ptr)
{
        if (ptr != NULL)
                return 0;

        if (HM_EXIT_ON_ALLOC_FAIL) {
                fprintf(stderr, "failed to allocate memory\n");
                exit(1);
        }

        return 1;
}

/* will duplicate key, but not val */
struct hashmap_item *new_hashmap_item(const char *key, void *val,
                void (*val_free_func)(void *))
{
        if (key == NULL || val == NULL)
                return NULL;

        struct hashmap_item *item = HM_CALLOC_FUNC(1,
                        sizeof(struct hashmap_item));

        if (hm_check_mem(item))
                return NULL;

        item->key = HM_STRDUP_FUNC(key);
        if (hm_check_mem(item)) {
                free(item);
                return NULL;
        }

        item->val = val;
        item->val_free_func = val_free_func;

        item->next = NULL;
        item->prev = NULL;

        return item;
}

struct hashmap *new_hashmap(size_t capacity)
{
        if (capacity == 0)
                return NULL;

        struct hashmap *hm = HM_CALLOC_FUNC(1, sizeof(struct hashmap));
        if (hm_check_mem(hm))
                return NULL;

        struct hashmap_item **items = HM_CALLOC_FUNC(capacity,
                        sizeof(struct hashmap_item *));

        if (hm_check_mem(items)) {
                free(hm);
                return NULL;
        }

        hm->capacity = capacity;
        hm->n = 0;
        hm->items = items;

        return hm;
}

static void free_hashmap_item(struct hashmap_item *item)
{
        if (item == NULL)
                return;

        struct hashmap_item *curr;
        while (item != NULL) {
                curr = item;
                item = item->next;
                free(curr->key);
                if (curr->val_free_func != NULL)
                        curr->val_free_func(curr->val);
                free(curr);
        }
}

void free_hashmap(struct hashmap *hm)
{
        if (hm == NULL)
                return;

        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->items[i] == NULL)
                        continue;
                free_hashmap_item(hm->items[i]);
        }
        free(hm->items);
        free(hm);
}

uint64_t hm_fnv1a_hash(const char *str)
{
        if (str == NULL)
                return 0;

        uint64_t hash = HM_FNV_OFFSET;
        for (const char *p = str; *p; p++) {
                hash ^= (uint64_t)(unsigned char)(*p);
                hash *= HM_FNV_PRIME;
        }

        return hash;
}

static int check_hashmap_capacity(struct hashmap *hm)
{
        if (hm == NULL)
                return 1;

        if (hm->n + 1 < hm->capacity)
                return 0;

        int capacity_increase_size = 8;

        struct hashmap_item **items = HM_CALLOC_FUNC(
                        hm->capacity + capacity_increase_size,
                        sizeof(struct hashmap_item *)
        );

        if (hm_check_mem(items))
                return 1;

        for (size_t i = 0; i < hm->capacity; i++) {
                struct hashmap_item *item = hm->items[i];
                if (item == NULL)
                        continue;
                if (item->key == NULL)
                        continue;

                uint64_t hash = hm_fnv1a_hash(item->key);
                size_t idx = (size_t)(hash % (uint64_t)(hm->capacity + capacity_increase_size));
                /* no need to worry about collision
                 * because we implement a linked
                 * list per bucket for this */
                items[idx] = item;
        }

        hm->capacity += capacity_increase_size;
        free(hm->items);
        hm->items = items;
        return 0;
}

int hashmap_insert(struct hashmap *hm, struct hashmap_item *item)
{
        if (hm == NULL || item == NULL)
                return -1;

        if (check_hashmap_capacity(hm))
                return -1;

        uint64_t hash = hm_fnv1a_hash(item->key);
        size_t idx = (size_t)(hash % (uint64_t)(hm->capacity));

        if (hm->items[idx] == NULL) {
                hm->items[idx] = item;
                hm->n++;
                return idx;
        }

        /* collision, handle linked list */
        struct hashmap_item *target_bucket = hm->items[idx];
        while (target_bucket->next != NULL)
                target_bucket = target_bucket->next;

        target_bucket->next = item;
        item->prev = target_bucket;
        item->next = NULL;

        return idx;
}

void *hashmap_get(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return NULL;

        uint64_t hash = hm_fnv1a_hash(key);
        size_t idx = (size_t)(hash % (uint64_t)(hm->capacity));

        struct hashmap_item *item = hm->items[idx];
        if (item == NULL)
                return NULL;

        if (strcmp(item->key, key) == 0)
                return item->val;

        /* not in the first bucket, iterate through linked list */
        struct hashmap_item *curr;
        while (item != NULL) {
                curr = item;
                item = item->next;
                if (strcmp(curr->key, key) == 0)
                        return curr->val;
        }

        return NULL;
}
