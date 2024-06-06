#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* only for maps with strings as values */
static void hashmap_print(struct hashmap *hm)
{
        for (size_t i = 0; i < hm->capacity; i++) {
                struct hashmap_item *item = hm->items[i];
                printf("%02ld: ", i);
                if (item == NULL) {
                        printf("NULL\n");
                        continue;
                }

                struct hashmap_item *curr;
                while (item != NULL) {
                        curr = item;
                        item = item->next;
                        printf("%s=%s ", curr->key, (char *)curr->val);
                }
                printf("\n");
        }
}

int main(void)
{
        struct hashmap *hm = new_hashmap(1);

        struct hashmap_item *item1 = new_hashmap_item(
                        "costarring", HM_STRDUP_FUNC("val1"), free);

        struct hashmap_item *item2 = new_hashmap_item(
                        "liquid", HM_STRDUP_FUNC("val2"), free);

        struct hashmap_item *item4 = new_hashmap_item(
                        "testkey", HM_STRDUP_FUNC("bruh"), free);

        struct hashmap_item *item5 = new_hashmap_item(
                        "testkey2", HM_STRDUP_FUNC("bruh2"), free);

        hashmap_insert(hm, item1);
        hashmap_insert(hm, item2);
        hashmap_insert(hm, item4);
        hashmap_insert(hm, item5);

        printf("map n=%ld cap=%ld\n", hm->n, hm->capacity);
        hashmap_print(hm);
        printf("---\n");
        char *val = (char *)hashmap_get(hm, "liquid");

        printf("%s\n", val);

        free_hashmap(hm);
}
