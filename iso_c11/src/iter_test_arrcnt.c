#include <stdio.h>
#include <stdlib.h>

// ---------------- ARRAY OF POINTERS, RETURNS ELEMENT, BUT CNT MUST BE SUPPLIED  --------------------

#define CONCAT_EXPAND(x, y) x ## y
#define CONCAT(x, y)        CONCAT_EXPAND(x, y)
#define UNIQUE_ID(prefix)   CONCAT(prefix, __COUNTER__)

// Внутренний макрос: получает уже готовое arr_name и iter_name
#define                             _foreach_arr_impl(iter, arr_name, cnt) \
    for (typeof_unqual(*(arr_name)) *iter_name = (arr_name), iter = *iter_name; \
         iter_name - arr_name <cnt; \
         iter = *++iter_name)

// Промежуточный макрос: сохраняет arr в arr_name (полученном снаружи) и генерирует iter_name
#define                             _foreach_arr_named(iter, arr, cnt, arr_name) \
    typeof_unqual(*arr) *arr_name = arr; \
    _foreach_arr_impl(iter, arr_name, cnt)

// Внешний макрос: генерирует уникальное имя для arr ОДИН раз и передаёт его вниз
#define                             foreach_arr(iter, arr, cnt) \
    _foreach_arr_named(iter, (arr), (cnt), UNIQUE_ID(_arr_))

// ----------- ARRAY OF POINTERS, RETURNS POINTER TO ELEMENT, BUT CNT MUST BE SUPPLIED  ---------------

// Внутренний макрос: получает уже готовое arr_name и iter_name
#define                             _pforeach_arr_impl(iter, arr_name, cnt) \
    for (typeof_unqual(*(arr_name)) *iter_name = (arr_name), *iter = iter_name; \
         iter_name - arr_name <cnt; \
         iter = ++iter_name)

// Промежуточный макрос: сохраняет arr в arr_name (полученном снаружи) и генерирует iter_name
#define                             _pforeach_arr_named(iter, arr, cnt, arr_name) \
    typeof_unqual(*arr) *arr_name = arr; \
    _pforeach_arr_impl(iter, arr_name, cnt)

// Внешний макрос: генерирует уникальное имя для arr ОДИН раз и передаёт его вниз
#define                             pforeach_arr(iter, arr, cnt) \
    _pforeach_arr_named(iter, (arr), (cnt), UNIQUE_ID(_arr_))

typedef struct {
    int     i;
    char    c;
} TestType;

static TestType   **getparr(TestType   **p){
    static int cnt = 0;
    printf("RUN!!!! %d\n", cnt++);
    return p;
}

int         main(void){


    // TEST1: array of pointers! That's working, OK
    TestType     arr[] = { {1,'a'}, {5, 'b'}, {11, 'c'}, {2, 'd'}, {4, 'e'}, {222, 'f'} };
    int          cnt = sizeof(arr) / sizeof(TestType);
    TestType   **parr = malloc(cnt * sizeof(TestType *) );

    for (int i = 0; i < cnt; i++)
        parr[i] = arr + i;

    // try print normally, to compare
    for (int i = 0; i < cnt; i++)
        printf("%d - %d:%c\n", i, parr[i]->i, parr[i]->c);

    //if (true)
        foreach_arr(item, parr, cnt)
            printf("TestType   **parr: %d - %c\n", item->i, item->c);

    foreach_arr(item, getparr(parr), cnt)
        printf("getparr(): %d - %c\n", item->i, item->c);

    foreach_arr(item, arr, cnt)
        printf("TestType     arr[]: %d - %c\n", item.i, item.c);

    // TEST2: simple array, warning or COMPILATION ERROR must be here!!!!!
    int a[] = {1, 2, 3, 0};
    foreach_arr(y, a, (int) (sizeof(a) / sizeof(int) ) )
        printf("int a[]: %d\n", y);

    int *b = a;
    foreach_arr(y, b, (int) (sizeof(a) / sizeof(int) ) )
        printf("int *b: %d\n", y);

    // test pointers

    TestType     arr2[] = { {100,'a'}, {500, 'b'}, {1100, 'c'}, {200, 'd'}, {400, 'e'}, {222, 'f'} };
    cnt = sizeof(arr2) / sizeof(TestType);
    pforeach_arr(item, arr2, cnt)
        printf("pointer TestType     arr2[]: %d - %c\n", item->i, item->c);

    return 0;
}
