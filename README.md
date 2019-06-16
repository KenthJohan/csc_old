# str_fmt
|task|csc|std|
|---|---|---|
|int32_t Decimal|`str_printf ("%i32", a)`|`printf("%" PRId32 "\n", a)`|
|int32_t Hexadecimal|`str_printf ("%i32_16", a)`|`printf("%" PRIX32 "\n", a)`|
|int32_t Binary|`str_printf ("%i32_2", a)`|Not supporterd|
|int32_t Negabinary|`str_printf ("%i32_-2", a)`|Not supporterd|
|int32_t Negadecimal|`str_printf ("%i32_-10", a)`|Not supporterd|
|uint8_t Trivigesimal|`str_printf ("%u8_23", a)`|Not supporterd|


```c
int32_t a = 56;
int32_t b = -56;
str_printf ("%i32", a); // +56
str_printf ("%i32", b); // -56
```
