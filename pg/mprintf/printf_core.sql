CREATE FUNCTION _printf_core(format text, args text[]) RETURNS text
AS 'MODULE_PATHNAME', '_printf_core'
LANGUAGE C IMMUTABLE STRICT;

