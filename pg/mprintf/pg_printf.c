#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PG_MODULE_MAGIC;

/* Форматирует один аргумент, преобразуя строку arg_text в нужный тип по спецификатору */
static void format_one_arg(const char *spec, const char *arg_text, char *out_buf, size_t out_size)
{
    char spec_type = spec[strlen(spec) - 1];
    switch (spec_type)
    {
        case 'd': case 'i':
        {
            long v = strtol(arg_text, NULL, 10);
            snprintf(out_buf, out_size, spec, v);
            break;
        }
        case 'u': case 'o': case 'x': case 'X':
        {
            unsigned long v = strtoul(arg_text, NULL, 10);
            snprintf(out_buf, out_size, spec, v);
            break;
        }
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
        {
            double v = strtod(arg_text, NULL);
            snprintf(out_buf, out_size, spec, v);
            break;
        }
        case 's':
            snprintf(out_buf, out_size, spec, arg_text);
            break;
        case 'c':
            snprintf(out_buf, out_size, spec, arg_text[0]);
            break;
        default:
            snprintf(out_buf, out_size, "%s", arg_text);
    }
}

PG_FUNCTION_INFO_V1(_printf_core);

Datum
_printf_core(PG_FUNCTION_ARGS)
{
    /* 0-й аргумент — форматная строка */
    char *format_str = text_to_cstring(PG_GETARG_TEXT_PP(0));
    int nargs = PG_NARGS();  // общее количество аргументов, включая формат
    StringInfoData buf;
    const char *p = format_str;
    const char *literal_start = p;
    int arg_idx = 1;  // начинаем с первого аргумента после формата

    initStringInfo(&buf);

    while (*p)
    {
        if (*p == '%')
        {
            /* Копируем предшествующий литерал */
            appendBinaryStringInfo(&buf, literal_start, p - literal_start);

            /* Обрабатываем %% */
            if (*(p + 1) == '%')
            {
                appendStringInfoChar(&buf, '%');
                p += 2;
                literal_start = p;
                continue;
            }

            /* Выделяем спецификатор */
            const char *spec_start = p;
            p++;
            while (*p && !strchr("diouxXeEfFgGaAcsCpmn", *p))
                p++;
            if (*p) p++;

            char spec[64];
            int spec_len = p - spec_start;
            if (spec_len >= sizeof(spec)) spec_len = sizeof(spec) - 1;
            memcpy(spec, spec_start, spec_len);
            spec[spec_len] = '\0';

            /* Если есть аргумент, форматируем его */
            if (arg_idx < nargs)
            {
                /* Получаем тип аргумента */
                Oid argtypid = get_fn_expr_argtype(fcinfo->flinfo, arg_idx);
                Datum d = PG_GETARG_DATUM(arg_idx);
                /* Преобразуем значение в строку (textout) */
                char *str = DatumGetCString(DirectFunctionCall1(textout, d));
                char formatted[256];
                format_one_arg(spec, str, formatted, sizeof(formatted));
                appendStringInfoString(&buf, formatted);
                pfree(str);
                arg_idx++;
            }
            else
            {
                /* Недостаточно аргументов — вставляем спецификатор как есть */
                appendStringInfoString(&buf, spec);
            }

            literal_start = p;
        }
        else
        {
            p++;
        }
    }

    /* Дописываем остаток строки */
    appendBinaryStringInfo(&buf, literal_start, p - literal_start);

    PG_RETURN_TEXT_P(cstring_to_text(buf.data));
}
