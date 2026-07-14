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
    struct value64_tarray;
} value64_static_tarray;

// TODO: value64_tarray4 {value64_tarray v[4];}

// ------------------------- CONSTRUCTORS / DESTRUCTORS -------------------

/** @brief Создать пустой массив с начальной ёмкостью cap */
value64_tarray                  value64_tarray_init(int cap);

/** @brief Освободить массив и все владеющие памятью элементы (FS/STR) */
void                            value64_tarray_free(value64_tarray *arr);

// ------------------------- ДОБАВЛЕНИЕ -----------------------------------

/** @brief Добавить элемент в конец (автоматически расширяет массив) */
void                            value64_tarray_push(value64_tarray *arr, value64_typed elem);

// ------------------------- ДОСТУП ---------------------------------------

/** @brief Получить i-й элемент (возвращает копию value64_typed) */
value64_typed                   value64_tarray_get(const value64_tarray *arr, int i);

/** @brief Получить указатель на i-й элемент */
value64_typed                  *value64_tarray_get_ptr(value64_tarray *arr, int i);

// ------------------------- ИНИЦИАЛИЗАЦИЯ ЛИТЕРАЛАМИ --------------------

/** @brief Создать статический массив из литералов */
#define VALUE64_TARRAY(...) \
    (value64_tarray){ .v = (value64_typed[]){__VA_ARGS__}, \
                      .cnt = COUNT((value64_typed[]){__VA_ARGS__}), \
                      .sz = COUNT((value64_typed[]){__VA_ARGS__}) }

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
    return (value64_typed){ .val = LITERAL64_FS(s), .typ = VALUE64_FS };
}

#endif /* _VALUE64_TARRAY_H */

