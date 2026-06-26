CREATE OR REPLACE FUNCTION printf(format text, VARIADIC args anyarray) RETURNS text
LANGUAGE plpgsql AS $$
DECLARE
    text_args text[];
BEGIN
    SELECT array_agg(elem::text) INTO text_args
    FROM unnest(args) AS elem;
    RETURN _printf_core(format, text_args);
END;
$$;

