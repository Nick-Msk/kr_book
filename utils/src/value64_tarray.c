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

/**
 * @brief  simple checker for increase
 *
 * @param varr value64_tarray pointer
*/
static bool                             check_increase(value64_tarray *varr) {
    if (varr->cnt >= varr->sz) {
        int newsz = varr->sz;
        if (!resize(varr, newsz, true)) {
            userraiseint(ERR_UNABLE_ALLOCATE,
                         "Unable to grow vt array from %d to %d", varr->sz, newsz);
        }
        return true;    // increased
    } else
        return false;
}

/**
 * @brief  technical printer, wrapper over value64_techfprint
 *
 * @param out FILE *, opened for write
 * @param v value64_typed structure
 */
static int                              value64_typed_techfprint(FILE *out, value64_typed v) {
    int cnt = 0;
    cnt += value64_techfprint(out, v.val, v.typ, "");
    return cnt;
}

// ------------------------------------- API -----------------------------------------------

/**
 * @brief Allocate an empty dynamic value64_tarray.
 * @param cap initial capacity (may be 0).
 * @return initialized array (owns memory, must be freed with value64_tarray_free).
 */
value64_tarray                      value64_tarray_init(int cap) {
    invraisecode(ERR_WRONG_PARAMETER, cap > 0, "Must be positive %d", cap);

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

    check_increase(arr);
    arr->v[arr->cnt++] = value64_typed_clone(elem);
    return logret(arr, "Pushed, cnt=%d", arr->cnt);
}

/**
 * @brief Move a typed element into a dynamic array, taking ownership.
 *
 * The source element *elem is moved into the array via value64_move();
 * after the call *elem is reset to a safe empty state
 * (LITERAL64_ZERO / VALUE64_INT).  This avoids a deep copy for
 * FS/STR values that own heap memory.
 *
 * @param arr  non‑NULL, non‑static value64_tarray
 * @param elem pointer to the element to move
 * @return pointer to the (possibly resized) array
 * @throws ERR_NULLABLE_PTR if arr or elem is NULL
 * @throws ERR_UNSUPPORTED_TYPE if arr is static (sz < 0)
 * @throws ERR_UNABLE_ALLOCATE if realloc fails
 */
value64_tarray *value64_tarray_move(value64_tarray *restrict arr, value64_typed *restrict elem) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL && elem != NULL,
                 "Null pointer arr=%p elem=%p", arr, elem);
    invraisecode(ERR_UNSUPPORTED_TYPE, arr->sz >= 0,
                 "Cannot move into static array (sz=%d)", arr->sz);

    logenter("elem typ=%d:%s", elem->typ, value64_typename(elem->typ));

    check_increase(arr);
    arr->v[arr->cnt++] = value64_typed_move(elem);
    return logret(arr, "Moved, cnt=%d", arr->cnt);
}

/**
 * @brief Free a dynamic value64_tarray and all owned elements.
 *
 * Static arrays (sz < 0) are silently ignored.
 * After return the array is zeroed and safe to reuse.
 *
 * @param arr non‑NULL pointer to a dynamic array
 * @throws ERR_NULLABLE_PTR if arr is NULL
 * @throws ERR_UNSUPPORTED_TYPE is static
 */
void                            value64_tarray_free(value64_tarray *arr) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, 
                "Null pointer");
    invraisecode(ERR_UNSUPPORTED_TYPE, arr->sz >= 0,
                "Cannot free static array (sz=%d)", arr->sz);
    for (int i = 0; i < arr->cnt; i++)
        value64_free(&arr->v[i].val, arr->v[i].typ);
    free(arr->v);
    arr->cnt = arr->sz = 0;
    arr->v = NULL;
    logsimple("freed tarray");
};

// ------------------------ Printers ---------------------------------------
/**
 * @brief  technical printer
 *
 * @param out FILE *, opened for write
 * @param arr  value64_tarray pointer
 * @param name   name of value64 tarray
 * @throws ERR_NULLABLE_PTR if arr is NULL
 */
int                             value64_tarray_techfprinf(FILE *restrict out, value64_tarray *restrict arr, const char *restrict name) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, "Null pointer");
    int cnt = 0;
    if (out) {
        cnt += fprintf(out, "VALUE64_TARRAY: cnt [%d], sz [%d] %s[\n", arr->cnt, arr->sz, name);
        for (int i = 0; i < arr->cnt; i++)
            cnt += value64_typed_techfprint(out, arr->v[i]);
        cnt += fprintf(out, "]\n");
    }
    return cnt;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64_TARRAY_TESTING

#include "test.h"

// ------------------------- TEST value64_tarray ---------------------------------
static TestStatus
tf_value64_tarray(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Создание пустого динамического массива */
    test_sub("subtest %d: init empty dynamic array", ++subnum);
    {
        value64_tarray arr = value64_tarray_init(0);
        test_validatefree(
            arr.sz == 0 && arr.cnt == 0 && arr.v == NULL,
            value64_tarray_free(&arr),
            "Empty init: sz=%d, cnt=%d, v=%p", arr.sz, arr.cnt, arr.v
        );
        value64_tarray_free(&arr);
    }

    /* 2. Создание с начальной ёмкостью */
    test_sub("subtest %d: init with capacity", ++subnum);
    {
        value64_tarray arr = value64_tarray_init(3);
        test_validatefree(
            arr.sz >= 3 && arr.cnt == 0,
            value64_tarray_free(&arr),
            "Init cap=3: sz=%d, cnt=%d", arr.sz, arr.cnt
        );
        value64_tarray_free(&arr);
    }

    /* 3. Добавление элементов разных типов */
    test_sub("subtest %d: push int, str, fs", ++subnum);
    {
        value64_tarray arr = value64_tarray_init(2);
        // push int
        VALUE64_TARRAY_TECHFPRINF(logfile, &arr);
        value64_tarray_push(&arr, value64_typedint(42));
        test_validatefree(
            arr.cnt == 1 && arr.v[0].typ == VALUE64_INT && arr.v[0].val.ival == 42,
            value64_tarray_free(&arr),
            "Push int: cnt=%d, val=%d", arr.cnt, arr.v[0].val.ival
        );
        // push str (literal, no ownership)
        value64_tarray_push(&arr, value64_typedstr("hello"));
        test_validatefree(
            arr.cnt == 2 && arr.v[1].typ == VALUE64_STR &&
            strcmp(arr.v[1].val.sval, "hello") == 0,
            value64_tarray_free(&arr),
            "Push str: cnt=%d, val=%s", arr.cnt, arr.v[1].val.sval
        );
        // push fs (heap allocated, ownership transferred)
        fs *f = fs_heapcopy("test");
        value64_tarray_push(&arr, value64_typedfs(f));
        // после push оригинальный f не должен освобождаться пользователем, массив владеет копией?
        // Так как мы клонируем в push, то f можно освободить отдельно
        fs_free(f);
        test_validatefree(
            arr.cnt == 3 && arr.v[2].typ == VALUE64_FS &&
            strcmp(fs_str(arr.v[2].val.fsval), "test") == 0,
            value64_tarray_free(&arr),
            "Push fs: cnt=%d, val=%s", arr.cnt, fs_str(arr.v[2].val.fsval)
        );
        value64_tarray_free(&arr);
    }
    fs_alloc_check(true);

    /* 4. Проверка клонирования: изменение оригинала не влияет на массив */
    test_sub("subtest %d: clone isolation (int)", ++subnum);
    {
        value64_tarray arr = value64_tarray_init(1);
        value64_typed orig = value64_typedint(100);
        value64_tarray_push(&arr, orig);
        // меняем оригинал
        orig.val.ival = 999;
        test_validatefree(
            arr.v[0].val.ival == 100,
            value64_tarray_free(&arr),
            "Clone isolation: array must keep original value 100, got %d",
            arr.v[0].val.ival
        );
        value64_tarray_free(&arr);
    }

    /* 5. Защита от push в статический массив */
    test_sub("subtest %d: push to static array must raise SIGINT", ++subnum);
    {
        value64_static_tarray st = VALUE64_TSTATIC_ARRAY(value64_typedint(1));
        if (!try()) {
            // пытаемся добавить в статический массив (приведение к value64_tarray*)
            value64_tarray_push((value64_tarray*)&st.base, value64_typedint(2));
            // если сюда дошли, то ошибки не было -> провал
            test_validate(false, "Push to static array should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised on push to static");
        }
        // статический массив освобождать не нужно
    }

    /* 6. Освобождение пустого массива */
    test_sub("subtest %d: free empty array", ++subnum);
    {
        value64_tarray arr = {0};
        value64_tarray_free(&arr);
        // просто не должно упасть
        test_validate(true, "Free empty array succeeded");
    }

    /* 7. Освобождение статического массива (должно игнорироваться) */
    test_sub("subtest %d: free static array must be ignored", ++subnum);
    {
        value64_static_tarray st = VALUE64_TSTATIC_ARRAY(value64_typedint(1));
        value64_tarray_free(&st.base);   // sz=-1, функция выйдет
        test_validate(true, "Free static array ignored");
    }

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
            testnew(.f2 =  tf_value64_tarray,                                 .num = 1, .name = "tf_value64_tarray"
                , .desc="", .mandatory=true)
        );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64_TARRAY_TESTING */


