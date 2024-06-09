#include "../hashmap.h"

#include <stdio.h>

/* TODO: proper unit tests */
int main(void)
{
        struct hashmap *hm = hashmap_new(free);

        char *ptr1 = HM_STRDUP_FUNC("value1");
        char *ptr2 = HM_STRDUP_FUNC("value2");
        char *ptr3 = HM_STRDUP_FUNC("this should be the final value");

        hashmap_insert(hm, "costarring", ptr1);
        hashmap_insert(hm, "liquid", ptr2);
        hashmap_insert(hm, "liquid", ptr3);

        //struct bucket *dup = duplicate_bucket(hm->buckets[13]);

        printf("----\n");
        hashmap_print(hm);
        printf("----\n");

        //hashmap_remove(hm, "ll3");

        printf("----\n");
        hashmap_print(hm);

        printf("----\n");
        //print_bucket(dup);


        hashmap_free(hm);
}
