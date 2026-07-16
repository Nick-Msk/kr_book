#ifndef _VALUE64_TARRAY_H
#define _VALUE64_TARRAY_H

#include "value64.h"

// ------------------------------- TYPES ---------------------------------

/** @brief Типизированное значение value64 (значение + тип) */
typedef struct {
    value64      val;   ///< само значение
    value64_type typ;   ///< его тип (VALUE64_INT, VALUE64_FS, ...)
} value64_typed;

/** @brief Динамический массив типизированных value64 */
typedef struct {
    value64_typed *v;   ///< элементы
    int            cnt; ///< занято
    int            sz;  ///< выделено
} value64_tarray;

// for static init (no free)
typedef struct {
    value64_tarray base;
} value64_static_tarray;

// TODO: value64_tarray4 {value64_tarray v[4];}

// ------------------------- CONSTRUCTORS / DESTRUCTORS -------------------

/** @brief Создать пустой массив с начальной ёмкостью cap */
extern value64_tarray                  value64_tarray_init(int cap);

/** @brief Освободить массив и все владеющие памятью элементы (FS/STR) */
extern void                            value64_tarray_free(value64_tarray *arr);

// ------------------------- ДОБАВЛЕНИЕ -----------------------------------

/** @brief Добавить элемент в конец (автоматически расширяет массив) */
extern value64_tarray                 *value64_tarray_push(value64_tarray *arr, value64_typed elem);

extern value64_tarray                 *value64_tarray_move(value64_tarray *restrict arr, value64_typed *restrict elem);

// ------------------------- ДОСТУП ---------------------------------------

/** @brief Получить i-й элемент (возвращает копию value64_typed) */
extern value64_typed                   value64_tarray_get(const value64_tarray *arr, int i);

/** @brief Получить указатель на i-й элемент */
extern value64_typed                  *value64_tarray_get_ptr(value64_tarray *arr, int i);

// ------------------------ Printers ---------------------------------------

/**
 * @brief  technical printer
 *
 * @param out FILE *, opened for write
 * @param arr  value64_tarray pointer
 * @param name   name of value64 tarray
*/
extern int                             value64_tarray_techfprinf(FILE *restrict out, value64_tarray *restrict arr, const char *restrict name);
/**
 * @brief  technical printer to stdout
 *
 * @param arr  value64_tarray pointer
 * @param name   name of value64 tarray
*/
static inline int                      value64_tarray_techprinf(value64_tarray *restrict arr, const char *restrict name) {
    return value64_tarray_techfprinf(stdout, arr, name);
}

#define VALUE64_TARRAY_TECHFPRINF(out, arr) value64_tarray_techfprinf(out, arr, #arr)
#define VALUE64_TARRAY_TECHPRINF(arr) value64_tarray_techfprinf(arr, #arr)

/**
 * @brief  technical printer, wrapper over value64_techfprint
 *
 * @param out FILE *, opened for write
 * @param v value64_typed structure
 */
int                                    value64_typed_techfprint(FILE *out, value64_typed v);

// ------------------------- ИНИЦИАЛИЗАЦИЯ ЛИТЕРАЛАМИ --------------------

/** @brief Создать статический массив из литералов */
#define VALUE64_TSTATIC_ARRAY(...) \
    (value64_static_tarray){ .base = { \
                      .v = (value64_typed[]){__VA_ARGS__}, \
                      .cnt = COUNT((value64_typed[]){__VA_ARGS__}), \
                      .sz = -1 } \
                           }

// ------------------------- ВСПОМОГАТЕЛЬНЫЕ КОНСТРУКТОРЫ ----------------

static inline value64_typed     value64_typedint(int x) {
    return (value64_typed){ .val = LITERAL64_INT(x), .typ = VALUE64_INT };
}
static inline value64_typed     value64_typedlong(long x) {
    return (value64_typed){ .val = LITERAL64_LNG(x), .typ = VALUE64_LNG };
}
static inline value64_typed     value64_typeddbl(double x) {
    return (value64_typed){ .val = LITERAL64_DBL(x), .typ = VALUE64_DBL };
}
static inline value64_typed     value64_typedstr(const char *s) {
    // Внимание: строка не копируется, ожидается, что она существует всё время использования массива
    return (value64_typed){ .val = LITERAL64_STR(s), .typ = VALUE64_STR };
}
static inline value64_typed     value64_typedfs(fs *s) {
    return (value64_typed){ .val = LITERAL64_PFS(s), .typ = VALUE64_FS };
}
static inline value64_typed     value64_typedunk(void) {
    return (value64_typed){ .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN };
}
static inline value64_typed     value64_typed_clone(value64_typed v) {
    return (value64_typed) { .val = value64_clone(v.val, v.typ), .typ = v.typ };
}
static inline value64_typed     value64_typed_move(value64_typed *v) {
    value64_typed   res = { .val = value64_move(&v->val, v->typ), .typ = v->typ};
    v->typ = VALUE64_UNKNOWN;
    return res;
}

#endif /* _VALUE64_TARRAY_H */

