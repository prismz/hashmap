#include "../hashmap.h"

#include <stdio.h>

int main(void)
{
        struct hashmap *hm = new_hashmap();

        char *ptr1 = HM_STRDUP_FUNC("value1");
        char *ptr2 = HM_STRDUP_FUNC("value2");

        struct bucket *b1 = new_bucket("costarring", ptr1, free);
        struct bucket *b2 = new_bucket("liquid", ptr2, free);
        struct bucket *b3 = new_bucket("ll3", HM_STRDUP_FUNC(
                                "this should be the final value"), free);
        hashmap_insert(hm, b1);
        hashmap_insert(hm, b2);
        hm->buckets[3] = b3;

        //struct bucket *dup = duplicate_bucket(hm->buckets[13]);

        printf("----\n");
        print_hashmap(hm);
        printf("----\n");

        hashmap_remove(hm, "ll3");

        printf("----\n");
        print_hashmap(hm);

        printf("----\n");
        //print_bucket(dup);


        free_hashmap(hm);
}
