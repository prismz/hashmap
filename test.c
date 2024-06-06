#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* stolen from stackoverflow for testing */
char *randstring(size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *randomString = NULL;

    if (length) {
        randomString = malloc(sizeof(char) * (length +1));

        if (randomString) {
            for (int n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

int main(void)
{
        struct hashmap *hm = new_hashmap(1);
        srand(0);
        for (int i = 0; i < 64; i++) {
                char *randkey = randstring(16);
                char *randval = randstring(8);

                struct hashmap_item *item = new_hashmap_item(randkey, randval, free);
                if (hashmap_insert(hm, item) < 0)
                        fprintf(stderr, "huh\n");
                free(randkey);
        }
        hashmap_print(hm);
        free_hashmap(hm);

/*
        struct hashmap_item *item1 = new_hashmap_item(
                        "costarring", HM_STRDUP_FUNC("val1"), free);

        struct hashmap_item *item2 = new_hashmap_item(
                        "liquid", HM_STRDUP_FUNC("val2"), free);

        struct hashmap_item *item4 = new_hashmap_item(
                        "testkey", HM_STRDUP_FUNC("val3"), free);

        struct hashmap_item *item5 = new_hashmap_item(
                        "testkey2", HM_STRDUP_FUNC("val4"), free);

        hashmap_insert(hm, item1);
        hashmap_insert(hm, item2);
        hashmap_insert(hm, item4);
        hashmap_insert(hm, item5);

        printf("map n=%ld cap=%ld\n", hm->n, hm->capacity);
        hashmap_print(hm);
        printf("---\n");
        void *val = hashmap_get(hm, "NONEXISTANT");

        if (val == NULL)
                printf("NULL\n");
        else
                printf("%s\n", (char *)val);

        free_hashmap(hm);
*/
}
