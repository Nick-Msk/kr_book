#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PG_MODULE_MAGIC;

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
    /* Аргумент 0 – форматная строка */
    char *fmt = text_to_cstring(PG_GETARG_TEXT_PP(0));
    int nargs = PG_NARGS();            /* общее число аргументов, включая формат */
    StringInfoData buf;
    const char *p = fmt;
    const char *lit = p;
    int idx = 1;                       /* следующий аргумент после формата */

    initStringInfo(&buf);

    while (*p)
    {
        if (*p == '%')
        {
            appendBinaryStringInfo(&buf, lit, p - lit);

            if (*(p + 1) == '%')
            {
                appendStringInfoChar(&buf, '%');
                p += 2;
                lit = p;
                continue;
            }

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

            if (idx < nargs)
            {
                Oid argtypid = get_fn_expr_argtype(fcinfo->flinfo, idx);
                Datum d = PG_GETARG_DATUM(idx);
                char *str = DatumGetCString(DirectFunctionCall1(textout, d));
                char tmp[256];
                format_one_arg(spec, str, tmp, sizeof(tmp));
                appendStringInfoString(&buf, tmp);
                pfree(str);
                idx++;
            }
            else
            {
                appendStringInfoString(&buf, spec);
            }

            lit = p;
        }
        else
            p++;
    }

    appendBinaryStringInfo(&buf, lit, p - lit);

    PG_RETURN_TEXT_P(cstring_to_text(buf.data));
}

