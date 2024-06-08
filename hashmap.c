#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef HM_DEBUG
#include <stdarg.h>
static void debug_print(char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
}
#else
/* compiler should just optimize this away */
static void debug_print()
{
}
#endif

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

/* hash a single byte */
static uint32_t fnv1a(unsigned char byte, uint32_t hash)
{
        return (byte ^ hash) * HM_FNV_PRIME;
}

static uint32_t hm_fnv1a_hash(const char *str)
{
        uint32_t hash = HM_FNV_SEED;
        while (*str)
                hash = fnv1a((unsigned char)*str++, hash);
        return hash;
}

/* will duplicate key */
struct bucket *new_bucket(const char *key, void *val, void (*val_free_func)(void *))
{
        if (key == NULL || val == NULL)
                return NULL;

        struct bucket *b = HM_CALLOC_FUNC(1, sizeof(struct bucket));
        if (hm_check_mem(b))
                return NULL;

        b->key = HM_STRDUP_FUNC(key);
        if (hm_check_mem(b->key)) {
                free(b);
                return NULL;
        }

        b->val = val;
        b->val_free_func = val_free_func;
        b->next = NULL;

        b->hash = hm_fnv1a_hash(key);

        debug_print("creating bucket with key %s, hash of this key is %" PRIu32 "\n",
                        key, b->hash);

        return b;
}

struct hashmap *new_hashmap(void)
{
        /* Any smaller and we'd pretty much instantly have to resize */
        size_t capacity = HM_DEFAULT_HASHMAP_SIZE;

        debug_print("creating hashmap with capacity %ld\n", capacity);

        struct hashmap *hm = HM_CALLOC_FUNC(1, sizeof(struct hashmap));
        if (hm_check_mem(hm))
                return NULL;

        hm->capacity = capacity;
        hm->n_buckets = 0;
        hm->buckets = HM_CALLOC_FUNC(capacity, sizeof(struct bucket *));
        if (hm_check_mem(hm->buckets)) {
                free(hm);
                return NULL;
        }

        return hm;
}

void free_bucket(struct bucket *b)
{
        if (b == NULL)
                return;

        struct bucket *curr;
        while (b != NULL) {
                curr = b;
                b = b->next;

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
                if (hm->buckets[i] == NULL)
                        continue;
                free_bucket(hm->buckets[i]);
        }
        free(hm->buckets);
        free(hm);
}

int hashmap_resize(struct hashmap *hm)
{
        if (hm == NULL)
                return -1;

        size_t new_size = hm->capacity * HM_RESIZE_SCALE_FACTOR;
        debug_print("resizing hashmap from size %ld to %ld...\n", hm->capacity,
                        new_size);
        struct bucket **buckets = HM_CALLOC_FUNC(new_size,
                        sizeof(struct bucket *));
        if (hm_check_mem(buckets))
                return 1;

        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->buckets[i] == NULL)
                        continue;

                size_t new_idx = hm->buckets[i]->hash % (uint32_t)new_size;
                debug_print("buckets at idx %ld move to idx %ld\n", i, new_idx);
                buckets[new_idx] = hm->buckets[i];
        }

        free(hm->buckets);
        hm->buckets = buckets;
        hm->capacity = new_size;

        return 0;
}

static int bucket_append(struct bucket *head, struct bucket *to_append)
{
        if (head == NULL || to_append == NULL)
                return 1;

        while (head->next != NULL)
                head = head->next;

        head->next = to_append;
        to_append->next = NULL;

        return 0;
}

int hashmap_insert(struct hashmap *hm, struct bucket *b)
{
        if ((float)(hm->n_buckets + 1) >
                        (HM_HASHMAP_MAX_LOAD * (float)hm->capacity)) {
                debug_print("resize required... calling resize function\n");
                if (hashmap_resize(hm))
                        return -1;
        }

        size_t idx = (b->hash) % (uint32_t)(hm->capacity);
        debug_print("insert: bucket with key %s goes to idx %ld\n", b->key, idx);

        hm->n_buckets++;

        struct bucket *target_bucket = hm->buckets[idx];
        if (target_bucket == NULL) {
                debug_print("no collision - inserting\n");
                hm->buckets[idx] = b;
                return 0;
        }

        /* changing value of something already in the hashmap */
        struct bucket *curr;
        while (target_bucket != NULL) {
                curr = target_bucket;
                target_bucket = target_bucket->next;
                if (strcmp(curr->key, b->key) == 0) {
                        debug_print("key already exists, reassigning value\n");
                        if (curr->val_free_func != NULL)
                                curr->val_free_func(curr->val);
                        curr->val = b->val;
                        b->val_free_func = NULL;
                        free_bucket(b);
                        return 0;
                }
        }

        debug_print("collision. appending to linked list\n");

        target_bucket = hm->buckets[idx];
        bucket_append(target_bucket, b);

        return 0;
}

void *hashmap_get(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return NULL;

        uint32_t hash = hm_fnv1a_hash(key);
        size_t idx = hash % (uint32_t)(hm->capacity);
        debug_print(
                "retreiving object with key %s - hashes to %" PRIu32 " giving idx %ld\n",
                key, hash, idx
        );

        struct bucket *target_bucket = hm->buckets[idx];
        if (target_bucket == NULL)
                return NULL;

        if (strcmp(target_bucket->key, key) == 0)
                return target_bucket->val;

        /* collision */
        struct bucket *curr;
        while (target_bucket != NULL) {
                curr = target_bucket;
                target_bucket = target_bucket->next;
                if (strcmp(curr->key, key) == 0)
                        return curr->val;
        }

        /* search all items as a last resort */
        for (size_t i = 0; i < hm->capacity; i++) {
                struct bucket *b = hm->buckets[i];
                struct bucket *curr;
                while (b != NULL) {
                        curr = b;
                        b = b->next;

                        if (strcmp(curr->key, key) == 0)
                                return curr->val;
                }
        }

        return NULL;
}

/*
 * wrapper function, so that if we don't find it with normal
 * methods, we can easily re-call the function with a custom index, allowing
 * us to iterate through the hashmap to find the item if for some
 * reason the hash gives an incorrect index
 */
static int _hashmap_remove(struct hashmap *hm, const char *key, uint32_t hash,
                size_t idx)
{
        if (hm == NULL || key == NULL)
                return 1;

        //uint32_t hash = hm_fnv1a_hash(key);
        //size_t idx = hash % (uint32_t)(hm->capacity);

        debug_print(
                "removing object with key %s which hashes to %" PRIu32
                ". Supplied index to search is %ld\n",
                key, hash, idx
        );

        struct bucket *target = hm->buckets[idx];
        if (target == NULL)
                return 1;

        if (strcmp(target->key, key) == 0) {
                /* bucket with single item */
                if (target->next == NULL) {
                        debug_print("simple removal of bucket with single node\n");
                        free_bucket(hm->buckets[idx]);
                        hm->buckets[idx] = NULL;
                        hm->n_buckets--;
                        return 0;
                }

                /* bucket where first item is to be removed,
                 * but there are other items in the linked list */

                debug_print("removing first node and preserving the rest of the linked list\n");

                struct bucket **next_addr = &(hm->buckets[idx]->next);
                hm->buckets[idx] = *next_addr;
                target->next = NULL;
                free_bucket(target);
                return 0;
        }

        /* the bucket to be removed is not the first
         * bucket in the linked list */
        debug_print("bucket to be removed is within the linked list - searching...\n");
        struct bucket *curr = target;
        struct bucket *prev = NULL;

        while (target != NULL) {
                curr = target;
                target = target->next;

                if (strcmp(curr->key, key) != 0) {
                        prev = curr;
                        continue;
                }

                if (prev == NULL) {
                        debug_print("this should never happen...\n");
                        return 1;
                }

                prev->next = curr->next;
                curr->next = NULL;
                free_bucket(curr);

                return 0;
        }

        /* if by this point we're still executing,
         * that means the item couldn't be found at the
         * correct index - we must iterate. */

        return 1;
}

int hashmap_remove(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return 1;

        uint32_t hash = hm_fnv1a_hash(key);
        size_t idx = hash % (uint32_t)(hm->capacity);

        if (_hashmap_remove(hm, key, hash, idx) == 0)
                return 0;

        debug_print("couldn't find item to remove by hashing, must search entire hashmap\n");
        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->buckets[i] == NULL)
                        continue;

                if (_hashmap_remove(hm, key, hash, i) == 0) {
                        debug_print("successfully found and removed item at idx %ld\n", i);
                        return 0;
                }
        }

        debug_print("couldn't find item to remove anywhere, returning 1\n");
        return 1;
}

#ifdef HM_DEBUG
/* only works for hashmap with strings as values */
void print_hashmap(struct hashmap *hm)
{
        if (hm == NULL) {
                printf("NULL\n");
                return;
        }

        printf("Printing hashmap containing %ld buckets with capacity %ld:\n",
                        hm->n_buckets, hm->capacity);

        for (size_t i = 0; i < hm->capacity; i++) {
                struct bucket *target = hm->buckets[i];
                printf("bucket %02ld:\n", i);
                if (target == NULL)
                        continue;

                struct bucket *curr;
                int n = 0;
                while (target != NULL) {
                        curr = target;
                        target = target->next;
                        printf("    %02d: %s=%s\n", n,
                                        curr->key, (char *)curr->val);

                        n++;
                }

        }
}
#endif
