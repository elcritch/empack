/*
 * Based on testing library from mpack:
 *
 * Copyright (c) 2015-2016 Nicholas Fraser
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "empack.h"

// enable this to exit at the first error
#define TEST_EARLY_EXIT 1

// runs the given expression, causing a unit test failure with the
// given printf format string if the expression is not true.
#define TEST_TRUE(expr, ...) \
  test_true_impl((expr), __FILE__, __LINE__, " " __VA_ARGS__)

void test_true_impl(bool result, const char* file, int line, const char* format, ...);


int tests;
int passes;

// in release mode there are no asserts or break functions, so
// TEST_BREAK() just runs the expr. it is usually used to test
// that something flags empack_error_bug.
#define TEST_BREAK(expr, ...)       \
  do {                              \
    TEST_TRUE(expr, ##__VA_ARGS__); \
  } while (0)

// initializes a reader from a literal string
#define TEST_READER_INIT_STR(reader, buf) \
  empack_reader_init_data(reader, buf, sizeof(buf) - 1)

void print_hexdump(char* str, int len)
{
  if (str == NULL) {
    printf("(null)");
  } else {
    for (int i = 0; i < len; i++) {
      uint8_t x = str[i];
      printf(" %02X", x);
    }
  }
}

#define TEST_DESTROY_MATCH_IMPL(expect)                                         \
  do {                                                                          \
    static const char buf[] = expect;                                           \
    size_t used = stream.max;                                                   \
    bool t = sizeof(buf) - 1 == used && memcmp(buf, stream.buf, used) == 0;     \
    if (!t) {                                                                   \
      printf("\n\tlhs: ");                                                      \
      print_hexdump((char*)buf, sizeof(buf) - 1);                               \
      printf("\n\trhs: ");                                                      \
      print_hexdump(stream.buf, stream.max);                                    \
      printf("\n");                                                             \
    }                                                                           \
    TEST_TRUE(t,                                                                \
        "written buf (of length %i) does not match expected (of length %i):  ", \
        (int)used, (int)(sizeof(buf) - 1));                                     \
  } while (0)

// runs a simple reader test, ensuring the expression is true and no errors occur
#define TEST_SIMPLE_READ(buf, read_expr)                                  \
  do {                                                                    \
    stream_t stream;                                                      \
    TEST_READER_INIT_STR(&stream, buf);                                   \
    TEST_TRUE((read_expr), "simple read test did not pass: " #read_expr); \
  } while (0)

// runs a simple stream test, ensuring it matches the given buf
#define TEST_SIMPLE_WRITE(expect, write_op)  \
  do {                                       \
    stream_t stream;                         \
    bool r;                                  \
    stream_setup(&stream, buf, sizeof(buf)); \
    write_op;                                \
    TEST_DESTROY_MATCH_IMPL(expect);         \
  } while (0)

#define MAX_TEST_BUFF 4096
void empack_stream_init(stream_t* reader)
{
  char* buf = malloc(MAX_TEST_BUFF);
  reader->buf = buf;
  reader->len = MAX_TEST_BUFF;
}

void empack_reader_init_data(stream_t* reader, char* buf, size_t len)
{
  memcpy(reader->buf, buf, len);
}

void test_true_impl(bool result, const char* file, int line, const char* format, ...)
{
  ++tests;
  if (result) {
    ++passes;
  } else {
    printf("TEST FAILED AT %s:%i --", file, line);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    if (TEST_EARLY_EXIT)
      abort();
  }
}

// writes ints using the auto int()/uint() functions
static void test_write_simple_auto_int()
{
  char buf[4096];

  // positive fixnums
  TEST_SIMPLE_WRITE("\x00", empack_write_u64(&stream, 0));
  TEST_SIMPLE_WRITE("\x01", empack_write_u64(&stream, 1));
  TEST_SIMPLE_WRITE("\x02", empack_write_u64(&stream, 2));
  TEST_SIMPLE_WRITE("\x0f", empack_write_u64(&stream, 0x0f));
  TEST_SIMPLE_WRITE("\x10", empack_write_u64(&stream, 0x10));
  TEST_SIMPLE_WRITE("\x7e", empack_write_u64(&stream, 0x7e));
  TEST_SIMPLE_WRITE("\x7f", empack_write_u64(&stream, 0x7f));

  // positive fixnums with signed int functions
  TEST_SIMPLE_WRITE("\x00", empack_write_i64(&stream, 0));
  TEST_SIMPLE_WRITE("\x01", empack_write_i64(&stream, 1));
  TEST_SIMPLE_WRITE("\x02", empack_write_i64(&stream, 2));
  TEST_SIMPLE_WRITE("\x0f", empack_write_i64(&stream, 0x0f));
  TEST_SIMPLE_WRITE("\x10", empack_write_i64(&stream, 0x10));
  TEST_SIMPLE_WRITE("\x7e", empack_write_i64(&stream, 0x7e));
  TEST_SIMPLE_WRITE("\x7f", empack_write_i64(&stream, 0x7f));

  // negative fixnums
  TEST_SIMPLE_WRITE("\xff", empack_write_i64(&stream, -1));
  TEST_SIMPLE_WRITE("\xfe", empack_write_i64(&stream, -2));
  TEST_SIMPLE_WRITE("\xf0", empack_write_i64(&stream, -16));
  TEST_SIMPLE_WRITE("\xe1", empack_write_i64(&stream, -31));
  TEST_SIMPLE_WRITE("\xe0", empack_write_i64(&stream, -32));

  // uints
  TEST_SIMPLE_WRITE("\xcc\x80", empack_write_u64(&stream, 0x80));
  TEST_SIMPLE_WRITE("\xcc\xff", empack_write_u64(&stream, 0xff));
  TEST_SIMPLE_WRITE("\xcd\x01\x00", empack_write_u64(&stream, 0x100));
  TEST_SIMPLE_WRITE("\xcd\xff\xff", empack_write_u64(&stream, 0xffff));
  TEST_SIMPLE_WRITE("\xce\x00\x01\x00\x00", empack_write_u64(&stream, 0x10000));
  TEST_SIMPLE_WRITE("\xce\xff\xff\xff\xff", empack_write_u64(&stream, 0xffffffff));
  /* TEST_SIMPLE_WRITE("\xcf\x00\x00\x00\x01\x00\x00\x00\x00", empack_write_u64(&stream, UINT64_C(0x100000000))); */
  /* TEST_SIMPLE_WRITE("\xcf\xff\xff\xff\xff\xff\xff\xff\xff", empack_write_u64(&stream, UINT64_C(0xffffffffffffffff))); */

  // positive ints with signed value
  TEST_SIMPLE_WRITE("\xcc\x80", empack_write_u8(&stream, 0x80));
  TEST_SIMPLE_WRITE("\xcc\xff", empack_write_u8(&stream, 0xff));
  TEST_SIMPLE_WRITE("\xcd\x01\x00", empack_write_u64(&stream, 0x100));
  TEST_SIMPLE_WRITE("\xcd\xff\xff", empack_write_u64(&stream, 0xffff));
  TEST_SIMPLE_WRITE("\xce\x00\x01\x00\x00", empack_write_u64(&stream, 0x10000));
  TEST_SIMPLE_WRITE("\xce\xff\xff\xff\xff", empack_write_u64(&stream, INT64_C(0xffffffff)));
  /* TEST_SIMPLE_WRITE("\xcf\x00\x00\x00\x01\x00\x00\x00\x00", empack_write_u64(&stream, INT64_C(0x100000000))); */
  /* TEST_SIMPLE_WRITE("\xcf\x7f\xff\xff\xff\xff\xff\xff\xff", empack_write_u64(&stream, INT64_C(0x7fffffffffffffff))); */

  // ints
  TEST_SIMPLE_WRITE("\xd0\xdf", empack_write_i64(&stream, -33));
  /* TEST_SIMPLE_WRITE("\xd0\x80", empack_write_i64(&stream, -128)); */
  TEST_SIMPLE_WRITE("\xd1\xff\x7f", empack_write_i64(&stream, -129));
  TEST_SIMPLE_WRITE("\xd1\x80\x00", empack_write_i64(&stream, -32768));
  TEST_SIMPLE_WRITE("\xd2\xff\xff\x7f\xff", empack_write_i64(&stream, -32769));

  // when using INT32_C() and compiling the test suite as c++, gcc complains:
  // error: this decimal constant is unsigned only in ISO C90 [-Werror]
  /* TEST_SIMPLE_WRITE("\xd2\x80\x00\x00\x00", empack_write_i64(&stream, INT64_C(-2147483648))); */

  /* TEST_SIMPLE_WRITE("\xd3\xff\xff\xff\xff\x7f\xff\xff\xff", empack_write_i64(&stream, INT64_C(-2147483649))); */
  /* TEST_SIMPLE_WRITE("\xd3\x80\x00\x00\x00\x00\x00\x00\x00", empack_write_i64(&stream, INT64_MIN)); */
}

static void test_write_basic_structures()
{
  char buf[MAX_TEST_BUFF];
  size_t size = MAX_TEST_BUFF;
  stream_t stream;

  // we use a mix of int writers below to test their tracking.

  // []
  stream_setup(&stream, buf, size);
  empack_write_array_header(&stream, 0);
  TEST_DESTROY_MATCH_IMPL("\x90");

  // [nil]
  stream_setup(&stream, buf, size);
  empack_write_array_header(&stream, 1);
  empack_write_nil(&stream);
  TEST_DESTROY_MATCH_IMPL("\x91\xc0");

  /* // range(15) */
  stream_setup(&stream, buf, size);
  empack_write_array_header(&stream, 15);
  for (int i = 0; i < 15; ++i)
    empack_write_i32(&stream, i);
  TEST_DESTROY_MATCH_IMPL(
      "\x9f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e");

  // range(16) (larger than infix)
  stream_setup(&stream, buf, size);
  empack_write_array_header(&stream, 16);
  for (int i = 0; i < 16; ++i)
    empack_write_u32(&stream, (uint32_t)i);
  TEST_DESTROY_MATCH_IMPL(
      "\xdc\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
      "\x0d\x0e\x0f");

  /* // UINT16_MAX nils */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_array(&stream, UINT16_MAX); */
  /*     for (int i = 0; i < UINT16_MAX; ++i) */
  /*         empack_write_nil(&stream); */
  /* empack_finish_array(&stream); */
  /* { */
  /*     const char prefix[] = "\xdc\xff\xff"; */
  /*     TEST_WRITER_DESTROY_NOERROR(&stream); */
  /*     TEST_TRUE(memcmp(prefix, buf, sizeof(prefix)-1) == 0, "array prefix is incorrect"); */
  /*     TEST_TRUE(size == UINT16_MAX + sizeof(prefix)-1); */
  /* } */

  /* // UINT16_MAX+1 nils (largest category) */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_array(&stream, UINT16_MAX+1); */
  /*     for (int i = 0; i < UINT16_MAX+1; ++i) */
  /*         empack_write_nil(&stream); */
  /* empack_finish_array(&stream); */
  /* { */
  /*     const char prefix[] = "\xdd\x00\x01\x00\x00"; */
  /*     TEST_WRITER_DESTROY_NOERROR(&stream); */
  /*     TEST_TRUE(memcmp(prefix, buf, sizeof(prefix)-1) == 0, "array prefix is incorrect"); */
  /*     TEST_TRUE(size == UINT16_MAX+1 + sizeof(prefix)-1); */
  /* } */
  /* if (buf) */
  /*     EMPACK_FREE(buf); */

  // {}
  stream_setup(&stream, buf, size);
  empack_write_map_header(&stream, 0);
  TEST_DESTROY_MATCH_IMPL("\x80");

  // {nil:nil}
  stream_setup(&stream, buf, size);
  empack_write_map_header(&stream, 1);
  empack_write_nil(&stream);
  empack_write_nil(&stream);
  TEST_DESTROY_MATCH_IMPL("\x81\xc0\xc0");

  // {0:0,1:1}
  stream_setup(&stream, buf, size);
  empack_write_map_header(&stream, 2);
  empack_write_i8(&stream, 0);
  empack_write_i16(&stream, 0);
  empack_write_u8(&stream, 1);
  empack_write_u16(&stream, 1);
  TEST_DESTROY_MATCH_IMPL("\x82\x00\x00\x01\x01");

  /* // {0:1, 2:3, ..., 28:29} */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_map(&stream, 15); */
  /*     for (int i = 0; i < 30; ++i) */
  /*         empack_write_i8(&stream, (int8_t)i); */
  /* empack_finish_map(&stream); */
  /* TEST_DESTROY_MATCH( */
  /*     "\x8f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e" */
  /*     "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d" */
  /*     ); */

  /* // {0:1, 2:3, ..., 28:29, 30:31} (larger than infix) */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_map(&stream, 16); */
  /*     for (int i = 0; i < 32; ++i) */
  /*         empack_write_int(&stream, i); */
  /* empack_finish_map(&stream); */
  /* TEST_DESTROY_MATCH( */
  /*     "\xde\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c" */
  /*     "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c" */
  /*     "\x1d\x1e\x1f" */
  /*     ); */

  /* // UINT16_MAX nil:nils */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_map(&stream, UINT16_MAX); */
  /*     for (int i = 0; i < UINT16_MAX*2; ++i) */
  /*         empack_write_nil(&stream); */
  /* empack_finish_map(&stream); */
  /* { */
  /*     const char prefix[] = "\xde\xff\xff"; */
  /*     TEST_WRITER_DESTROY_NOERROR(&stream); */
  /*     TEST_TRUE(memcmp(prefix, buf, sizeof(prefix)-1) == 0, "map prefix is incorrect"); */
  /*     TEST_TRUE(size == UINT16_MAX*2 + sizeof(prefix)-1); */
  /* } */
  /* if (buf) */
  /*     EMPACK_FREE(buf); */

  /* // UINT16_MAX+1 nil:nils (largest category) */
  /* stream_setup(&stream, buf, size); */
  /* empack_start_map(&stream, UINT16_MAX+1); */
  /*     for (int i = 0; i < (UINT16_MAX+1)*2; ++i) */
  /*         empack_write_nil(&stream); */
  /* empack_finish_map(&stream); */
  /* { */
  /*     const char prefix[] = "\xdf\x00\x01\x00\x00"; */
  /*     TEST_WRITER_DESTROY_NOERROR(&stream); */
  /*     TEST_TRUE(memcmp(prefix, buf, sizeof(prefix)-1) == 0, "map prefix is incorrect"); */
  /*     TEST_TRUE(size == (UINT16_MAX+1)*2 + sizeof(prefix)-1); */
  /* } */
  /* if (buf) */
  /*     EMPACK_FREE(buf); */
}

static void test_next_funcs()
{
  char buf[MAX_TEST_BUFF];
  size_t size = MAX_TEST_BUFF;
  stream_t stream;
}

int main()
{
  test_write_simple_auto_int();
  test_write_basic_structures();

  printf("\n\nUnit testing complete. %i failures in %i checks.\n\n\n", tests - passes, tests);
  return (passes == tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
