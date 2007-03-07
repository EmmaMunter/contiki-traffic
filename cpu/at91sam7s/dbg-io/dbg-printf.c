#include <stdio.h>
#include <debug-uart.h>
#include <string.h>
#include <strformat.h>

StrFormatResult
write_str(void *user_data, const char *data, unsigned int len)
{
  dbg_send_bytes((unsigned char*)data, len);
  return STRFORMAT_OK;
}


static StrFormatContext ctxt =
  {
    write_str,
    NULL
  };
int
printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  return format_str_v(&ctxt, fmt, ap);
  va_end(ap);
}


