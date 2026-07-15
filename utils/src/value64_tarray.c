/********************************************************************
             TYPED BASE VALUE64 ARRAY MODULE IMPLEMENTATION
********************************************************************/

#include "value64_tarray.h"

// ------------------------------------ Utilities ------------------------------------------

/**
 * @brief  simple formatter to VALUE64_UNKNOWN
 *
 * @param vtarr  typed value64 pointer
 * @param cnt    count to fromat
*/
static void                             format(value64_typed *vtarr, int cnt) {
    for (int i = 0; i < cnt; i++)
        vtarr[i] = value64_typedunk();
}

/**
 * @brief Изменяет размер динамического массива.
 *
 * - При расширении (newsz > sz) выделяет новый буфер нужного размера.
 *   Если strict == true, размер буфера будет ровно newsz.
 *   Если strict == false, размер может быть увеличен до ближайшей степени двойки.
 * - При сжатии (newsz < sz) размер буфера становится ровно newsz.
 *   При этом newsz НЕ должен быть меньше cnt – это ошибка.
 * - Если newsz == sz, ничего не делает.
 *
 * @param varr   динамический массив
 * @param newsz  новый размер (количество элементов)
 * @param strict управляет оптимизацией расширения
 * @return varr или NULL при ошибке
 */
static                  value64_tarray *resize(value64_tarray *varr, int newsz, bool strict) {
    invraisecode(ERR_NULLABLE_PTR, varr != NULL && newsz >= 0,
        "Null varr %p or incorrent input %d", varr, newsz);

    // Ничего не делаем, если размер не меняется
    if (newsz == varr->sz) {
        return logsimpleret(varr, "Noting to do, sz %d", newsz);
    }

    value64_typed *new_v = varr->v;
    if (newsz < varr->sz) {
        // Сжатие: строгая проверка
        if (newsz < varr->cnt) {
            // Ошибка: нельзя сжать массив так, чтобы потерять существующие элементы
            return logsimpleerr(NULL, "Unable to shrink %d < cnt %d", newsz, varr->cnt);
        }
        // Сжатие всегда strict (не округляем)
        new_v = realloc(varr->v, newsz * sizeof(value64_typed));
        if (!new_v && newsz > 0) {
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to acclocate %d elems", newsz);  // realloc failed
        }
    } else {
        // Расширение
        if (!strict)
            newsz = calcnewsize(SIZE_POWER2, newsz);
        value64_typed *new_v = realloc(varr->v, newsz * sizeof(value64_typed));
        if (!new_v) {
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to acclocate %d elems", newsz);  // ошибка выделения памяти
        }

        format(new_v + varr->sz, newsz - varr->sz);
    }
    varr->v = new_v;
    varr->sz = newsz;
    return logsimpleret(varr, "Resized to %d", varr->sz);
}

// ------------------------------------- API -----------------------------------------------

/**
 * @brief Allocate an empty dynamic value64_tarray.
 * @param cap initial capacity (may be 0).
 * @return initialized array (owns memory, must be freed with value64_tarray_free).
 */
value64_tarray                      value64_tarray_init(int cap) {
    logenter("%d", cap);
    value64_tarray arr = { .v = NULL, .cnt = 0, .sz = 0 };
    if (!resize(&arr, cap, false) )
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to create vt array %d", cap);

    return logret(arr, "Created with %d", arr.sz);
}

/**
 * @brief Append a typed element to a dynamic array.
 *
 * @param arr  non‑NULL, non‑static value64_tarray
 * @param elem element to append
 * @return pointer to the (possibly resized) array
 * @throws ERR_NULLABLE_PTR if arr is NULL
 * @throws ERR_UNSUPPORTED_TYPE if arr is static (sz < 0)
 * @throws ERR_UNABLE_ALLOCATE if realloc fails
 */
value64_tarray                     *value64_tarray_push(value64_tarray *arr, value64_typed elem) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, 
                "Null pointer");
    invraisecode(ERR_UNSUPPORTED_TYPE, arr->sz >= 0,
                "Cannot push to static array (sz=%d)", arr->sz);

    logenter("elem typ=%d:%s", elem.typ, value64_typename(elem.typ) );
    if (arr->cnt >= arr->sz) {
        // ensure a reasonable minimum capacity
        int newsz = arr->sz > 0 ? arr->sz * 2 : 4;
        if (!resize(arr, newsz, false)) {
            userraiseint(ERR_UNABLE_ALLOCATE,
                         "Unable to grow vt array from %d to %d", arr->sz, newsz);
        }
    }
    arr->v[arr->cnt++] = value64_typed_clone(elem);
    return logret(arr, "Pushed, cnt=%d", arr->cnt);
}

/** @brief Освободить массив и все владеющие памятью элементы (FS/STR) */
void                            value64_tarray_free(value64_tarray *arr) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, 
                "Null pointer");
    invraisecode(ERR_UNSUPPORTED_TYPE, arr->sz >= 0,
                "Cannot free static array (sz=%d)", arr->sz);
    for (int i = 0; i < arr->cnt; i++)
        value64_free(&arr->v[i].val, arr->v[i].typ);
    arr->cnt = arr->sz = 0;
    arr->v = NULL;
    logsimple("freed tarray");
};

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64_TARRAY_TESTING

#include "test.h"

// ------------------------- TEST hset_filtereduce_* (all types) -------------------------
static TestStatus
tf(const char *name)    // TODO:
{
    logenter("%s", name);
    int subnum = 0;

    {

    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        testenginestd_run(num,
            testnew(.f2 =  tf,                                 .num = 1, .name = "Hset_in simple test"                        
                , .desc="", .mandatory=true)
        );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64_TARRAY_TESTING */


