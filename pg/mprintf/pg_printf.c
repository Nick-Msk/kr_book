#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PG_MODULE_MAGIC;

static void format_one_arg(const char *spec, const char *arg_text, char *out_buf, size_t out_size)
{
    // ... без изменений
}

PG_FUNCTION_INFO_V1(_printf_core);

Datum
_printf_core(PG_FUNCTION_ARGS)
{
    char *fmt = text_to_cstring(PG_GETARG_TEXT_PP(0));
    int nargs = PG_NARGS();
    StringInfoData buf;
    const char *p = fmt;
    const char *lit = p;
    int idx = 1;

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
                Oid typoutput;
                bool typisvarlena;
                getTypeOutputInfo(argtypid, &typoutput, &typisvarlena);
                char *str = OidOutputFunctionCall(typoutput, d);
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

