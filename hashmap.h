#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HM_CALLOC_FUNC calloc
#define HM_STRDUP_FUNC strdup
#define HM_EXIT_ON_ALLOC_FAIL 1
#define HM_HASHMAP_MAX_LOAD 0.70f
#define HM_RESIZE_SCALE_FACTOR 2
#define HM_DEFAULT_HASHMAP_SIZE 16

/* enabling this will print something for
 * each internal action, not recommended */
// #define HM_DEBUG

/*
 * if using structs as values, this conveniently
 * typecasts your struct's free function
 * so that it can be passed to new_hashmap_item()
 */
#define hashmap_item_free_func(a) (void (*)(void *))a

/* https://create.stephan-brumme.com/fnv-hash */
#define HM_FNV_PRIME 0x01000193  //   16777619
#define HM_FNV_SEED  0x811C9DC5  // 2166136261

struct bucket;
struct bucket {
        uint32_t hash;
        char *key;
        void *val;
        void (*val_free_func)(void *);

        /* collisions are stored as linked lists */
        struct bucket *next;
};

struct hashmap {
       struct bucket **buckets;
       size_t n_buckets;
       size_t capacity;
};

struct bucket *new_bucket(const char *key, void *val,
                void (*val_free_func)(void *));
struct hashmap *new_hashmap(void);
void free_bucket(struct bucket *b);
void free_hashmap(struct hashmap *hm);
int hashmap_resize(struct hashmap *hm);
int hashmap_insert(struct hashmap *hm, struct bucket *b);
void *hashmap_get(struct hashmap *hm, const char *key);
int hashmap_remove(struct hashmap *hm, const char *key);

#ifdef HM_DEBUG
void print_hashmap(struct hashmap *hm);
#endif

#endif  /* HASHMAP_H */
