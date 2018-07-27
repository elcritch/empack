/**
 * Copyright (C) 2018 Jaremy Creechley
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "em_buffer.h"
#include "empack.h"

empack_type_t empack_next_type(buffer_t* b)
{
  int16_t msgpack_type = buffer_peek(b);

  switch (msgpack_type) {
  case -1:
    return EMPACK_EMPTY;

  case 0xc0:
    return EMPACK_NIL;

  case 0xc2:
  case 0xc3:
    return EMPACK_BOOL;

  case 0xc4:
  case 0xc5:
  case 0xc6:
    return EMPACK_BIN;

  case 0xCA:
  case 0xCB:
    return EMPACK_FLOAT;

  case 0xCC:
  case 0xCD:
  case 0xCE:
  case 0xCF:
    return EMPACK_UINT;

  case 0xD0:
  case 0xD1:
  case 0xD2:
  case 0xD3:
    return EMPACK_SINT;

  case 0xD4:
  case 0xD5:
  case 0xD6:
  case 0xD7:
  case 0xD8:
    return EMPACK_EXT;

  case 0xD9:
  case 0xDA:
  case 0xDB:
    return EMPACK_STRING;

  case 0xDC:
  case 0xDD:
    return EMPACK_ARRAY;

  case 0xDE:
  case 0xDF:
    return EMPACK_MAP;

  default:
    break;
  }

  if (msgpack_type >= 0xE0)
    return EMPACK_UINT;
  else if (msgpack_type < 0x80)
    return EMPACK_SINT;
  else if (msgpack_type & 0xA0)
    return EMPACK_STRING;
  else if (msgpack_type & 0x80)
    return EMPACK_MAP;
  else if (msgpack_type & 0x90)
    return EMPACK_ARRAY;

  return EMPACK_UNKNOWN;
}


#define CONST(c) ((em_byte_t)c)

bool empack_read_nil(buffer_t* s)
{
  int32_t ret = buffer_read_byte(s);
  if (ret < 0)
    return false;

  em_byte_t mpack_byte = ret & 0xFF;
  return mpack_byte == CONST(0xC0);
}

bool empack_read_bool(buffer_t* s, bool* value)
{
  em_byte_t mpack_byte;
  int read = buffer_read(s, &mpack_byte, 1);

  if (read != 1)
    return false;

  *value = mpack_byte == CONST(0xC3);

  return mpack_byte == CONST(0xC3) || mpack_byte == CONST(0xC2);
}

bool empack_read_sint(buffer_t* s, em_byte_t* b, uint8_t count_bytes)
{
  em_byte_t mpack_byte;
  uint8_t read_size;
  if (buffer_read(s, &mpack_byte, 1) != 1) {
    return false;
  }
  if (mpack_byte < CONST(0x80)) {
    return false;
  } else if (mpack_byte >= CONST(0xE0)) {
    b[0] = mpack_byte;
    int8_t i;
    for (i = count_bytes - 1; i >= 1; i--) {
      b[i] = 0xff;
    }
    return true;
  } else if (mpack_byte == CONST(0xD0)) {
    read_size = 1;
  } else if (mpack_byte == CONST(0xD1)) {
    read_size = 2;
  } else if (mpack_byte == CONST(0xD2)) {
    read_size = 4;
  } else if (mpack_byte == CONST(0xD3)) {
    read_size = 8;
  } else {
    return false;
  }
  if (read_size > count_bytes)
    return false;

  bool res = true;
  int8_t i;
  for (i = read_size - 1; i >= 0; i--) {
    res &= buffer_read(s, &b[i], 1);
  }

  uint8_t prefix = (b[read_size - 1] >> 7) == 1 ? 0xFF : 0x00;
  for (i = count_bytes - 1; i >= read_size; i--) {
    b[i] = prefix;
  }
  return res;
}

bool empack_read_uint(buffer_t* s, em_byte_t* b, uint8_t count_bytes)
{
  em_byte_t mpack_byte;
  uint8_t read_size;
  if (buffer_read(s, &mpack_byte, 1) != 1) {
    return false;
  }

  if (mpack_byte < CONST(0x80)) {
    b[0] = mpack_byte;
    int8_t i;
    for (i = count_bytes - 1; i >= 1; i--) {
      b[i] = 0x0;
    }
    return true;
  } else if (mpack_byte >= CONST(0xE0)) {
    return false;
  } else if (mpack_byte == CONST(0xCC)) {
    read_size = 1;
  } else if (mpack_byte == CONST(0xCD)) {
    read_size = 2;
  } else if (mpack_byte == CONST(0xCE)) {
    read_size = 4;
  } else if (mpack_byte == CONST(0xCF)) {
    read_size = 8;
  } else {
    return false;
  }

  if (read_size > count_bytes)
    return false;

  bool res = true;
  int8_t i;

  for (i = read_size - 1; i >= 0; i--) {
    res &= buffer_read(s, &b[i], 1);
  }

  for (i = count_bytes - 1; i >= read_size; i--) {
    b[i] = 0x00;
  }
  return res;
}

bool empack_next_skip(buffer_t* s, empack_type_t* skip_type)
{
  empack_type_t type = empack_next_type(s);
  *skip_type = type;

  uint64_t n = 0;
  uint32_t l = 0;
  em_byte_t ignore[2];
  bool b = true, r = true;

  switch (type) {

  case EMPACK_EMPTY:
    return false;

  case EMPACK_NIL:
    return empack_read_nil(s);

  case EMPACK_BOOL:
    return empack_read_bool(s, &b);


  case EMPACK_BIN:
    r = empack_read_bin_sz(s, ignore, 0, &l);
    if (r) {
      l += s->pos;
      s->pos = l;
      s->max = l;
    }
    return r;

  case EMPACK_STRING:
    r = empack_read_string_sz(s, (char*)ignore, 0, &l);
    if (r) {
      l += s->pos;
      s->pos = l;
      s->max = l;
    }
    return r;

  case EMPACK_SINT:
    return empack_read_sint(s, (em_byte_t*)&n, 8);

  case EMPACK_UINT:
    return empack_read_uint(s, (em_byte_t*)&n, 8);

  case EMPACK_ARRAY:
    r = empack_read_array_size(s, &l);
    for (int c = 0; c < l && r; ++c)
      r &= empack_next_skip(s, (empack_type_t*)&n);
    return r;

  case EMPACK_MAP:
    r = empack_read_map_size(s, &l);
    for (int c = 0; c < l && r; ++c)
      r &= empack_next_skip(s, (empack_type_t*)&n);
    return r;

  default:
    break;
  }

  return EMPACK_UNKNOWN;
}

bool empack_next_copy(buffer_t* s, buffer_t* out, empack_type_t* skip_type)
{
  bool status;

  em_size_t pos_start = s->pos;
  status = empack_next_skip(s, skip_type);
  em_size_t pos_next = s->pos;
  em_size_t skip_size = pos_next - pos_start;

  if (skip_size > out->len)
    return false;

  for (em_size_t i = 0; i < skip_size; i++) {
    buffer_write_byte(out, s->buf[i]);
  }

  return status;
}

#define BUFFER_READ_X8(b, s, p) \
  (b &= buffer_read(s, &p[0], 1));

#define BUFFER_READ_X16(b, s, p)     \
  (b &= buffer_read(s, &p[1], 1));  \
  (b &= buffer_read(s, &p[0], 1));


#define BUFFER_READ_X32(b, s, p)     \
  (b &= buffer_read(s, &p[3], 1));      \
  (b &= buffer_read(s, &p[2], 1));        \
  (b &= buffer_read(s, &p[1], 1));        \
  (b &= buffer_read(s, &p[0], 1));

bool empack_read_string_sz(buffer_t* s, char* str, uint32_t count_bytes, uint32_t* str_size)
{
  em_byte_t* str_buff = (em_byte_t*)str;
  *str_size = 0;
  em_byte_t mpack_byte;
  bool b = true;
  uint32_t read_size = 0;
  em_byte_t* p = (em_byte_t*)&read_size;

  if (buffer_read(s, &mpack_byte, 1)) {
    if ((mpack_byte >> 5) == 5) {
      read_size = mpack_byte & 0x1F;
    } else if (mpack_byte == CONST(0xD9)) {
      BUFFER_READ_X8(b, s, p);
    } else if (mpack_byte == CONST(0xDA)) {
      BUFFER_READ_X16(b, s, p);
    } else if (mpack_byte == CONST(0xDB)) {
      BUFFER_READ_X32(b, s, p);
    } else {
      return false;
    }

    *str_size = read_size;

    if (read_size > count_bytes)
      return false;

    uint32_t i;
    for (i = 0; i < read_size; i++) {
      b &= buffer_read(s, &(str_buff[i]), 1);
    }
    return b;
  }
  return false;
}

bool empack_read_string(buffer_t* s, char* str, uint32_t count_bytes)
{
  uint32_t read_size;
  return empack_read_string_sz(s, str, count_bytes, &read_size);
}


bool empack_read_bin_sz(buffer_t* s, em_byte_t* bin, uint32_t count_bytes, uint32_t* bin_size)
{
  em_byte_t mpack_byte;
  bool b = true;
  uint32_t read_size = 0;
  em_byte_t* p = (em_byte_t*)&read_size;
  if (buffer_read(s, &mpack_byte, 1)) {
    if (mpack_byte == CONST(0xc4)) {
      b &= buffer_read(s, &p[0], 1);
      BUFFER_READ_X8(b, s, p);
    } else if (mpack_byte == CONST(0xC5)) {
      BUFFER_READ_X16(b, s, p);
    } else if (mpack_byte == CONST(0xC6)) {
      BUFFER_READ_X32(b, s, p);
    } else {
      return false;
    }

    *bin_size = read_size;

    if (read_size > count_bytes)
      return false;

    return b && buffer_read(s, bin, read_size) == read_size;
  }
  return false;
}

bool empack_read_bin(buffer_t* s, em_byte_t* bin, uint32_t count_bytes)
{
  uint32_t read_size;
  return empack_read_bin_sz(s, bin, count_bytes, &read_size);
}

bool empack_read_array_size(buffer_t* s, uint32_t* array_size)
{
  em_byte_t mpack_byte;
  bool b = true;
  em_byte_t* p = (em_byte_t*)array_size;
  if (buffer_read(s, &mpack_byte, 1)) {
    *array_size = 0;
    if ((mpack_byte >> 4) == CONST(0x09)) {
      *array_size = mpack_byte & 0x0F;
    } else if (mpack_byte == CONST(0xDC)) {
      BUFFER_READ_X16(b, s, p);
      return b;
    } else if (mpack_byte == CONST(0xDD)) {
      BUFFER_READ_X32(b, s, p);
    } else {
      return false;
    }
    return b;
  }
  return false;
}

bool empack_read_map_size(buffer_t* s, uint32_t* map_size)
{
  em_byte_t mpack_byte;
  bool b = true;
  em_byte_t* p = (em_byte_t*)map_size;
  if (buffer_read(s, &mpack_byte, 1)) {
    *map_size = 0;
    if ((mpack_byte >> 4) == CONST(0x08)) {
      *map_size = mpack_byte & 0x0F;
    } else if (mpack_byte == CONST(0xDE)) {
      BUFFER_READ_X16(b, s, p);
    } else if (mpack_byte == CONST(0xDF)) {
      BUFFER_READ_X32(b, s, p);
    } else {
      return false;
    }
    return b;
  }
  return false;
}

void empack_write_nil(buffer_t* s)
{
  buffer_write_byte(s, 0xC0);
}

void empack_write_bool(buffer_t* s, bool b)
{
  b ? buffer_write_byte(s, 0xC3) : buffer_write_byte(s, 0xC2);
}

void empack_write_u8(buffer_t* s, uint8_t u)
{
  if (u < 0x80) {
    buffer_write_byte(s, u);
  } else {
    buffer_write_byte(s, 0xCC);
    buffer_write_byte(s, u);
  }
}

void empack_write_u16(buffer_t* s, uint16_t u)
{
  if (u < 256) {
    empack_write_u8(s, (uint8_t)u);
  } else {
    buffer_write_byte(s, 0xCD);
    buffer_write_byte(s, u >> 8);
    buffer_write_byte(s, u & 0xff);
  }
}

void empack_write_u32(buffer_t* s, uint32_t u)
{
  if (u < 65536) {
    empack_write_u16(s, (uint16_t)u);
  } else {
    buffer_write_byte(s, 0xCE);
    for (uint8_t i = 24; i >= 8; i = i - 8)
      buffer_write_byte(s, u >> i);
    buffer_write_byte(s, u & 0xFF);
  }
}

void empack_write_u64(buffer_t* s, uint64_t u)
{
  if (u < 4294967296) {
    empack_write_u32(s, (uint32_t)u);
  } else {
    buffer_write_byte(s, 0xcd);
    for (uint8_t i = 56; i >= 0; i = i - 8)
      buffer_write_byte(s, u >> i & 0xFF);
  }
}

void empack_write_i8(buffer_t* s, int8_t i)
{
  if ((i < -32) || (i > 0x7F)) {
    buffer_write_byte(s, 0xD0);
    buffer_write_byte(s, i);
  } else {
    buffer_write_byte(s, i);
  }
}

void empack_write_i16(buffer_t* s, int16_t i)
{
  if ((i < SCHAR_MIN + 1) || (i > UCHAR_MAX + 1)) {
    buffer_write_byte(s, 0xd1);
    for (uint8_t i = 8; i >= 0; i = i - 8)
      buffer_write_byte(s, i >> i);
  } else {
    empack_write_i8(s, (int8_t)i);
  }
}

void empack_write_i32(buffer_t* s, int32_t i)
{
  if ((i < SHRT_MIN) || (i > SHRT_MAX)) {
    buffer_write_byte(s, 0xd2);
    for (uint8_t i = 24; i >= 0; i = i - 8)
      buffer_write_byte(s, i >> i & 0xFF);
  } else {
    empack_write_i16(s, (int16_t)i);
  }
}

void empack_write_i64(buffer_t* s, int64_t i)
{
  if ((i < INT_MIN) || (i > INT_MAX)) {
    buffer_write_byte(s, 0xD3);
    for (uint8_t i = 56; i >= 0; i = i - 8)
      buffer_write_byte(s, i >> i & 0xFF);
  } else {
    empack_write_i32(s, (int32_t)i);
  }
}

void empack_write_float(buffer_t* s, float f)
{
  union float_to_byte {
    float f;
    em_byte_t b[4];
  } f2b;
  f2b.f = f;
  buffer_write_byte(s, 0xCA);
  for (uint8_t i = 3; i >= 0; i--)
    buffer_write_byte(s, f2b.b[i]);
}

static bool empack_write_size(buffer_t *s, uint8_t xa, uint8_t xb, uint8_t xc, uint32_t x_size)
{
  if (x_size > USHRT_MAX) {
    buffer_write_byte(s, xa);
    for (uint8_t i = 24; i >= 0; i = i - 8)
      buffer_write_byte(s, (x_size >> i) & 0xFF);
  } else if (x_size > UCHAR_MAX) {
    buffer_write_byte(s, xb);
    for (uint8_t i = 8; i >= 0; i = i - 8)
      buffer_write_byte(s, (x_size >> i) & 0xFF);
  } else {
    buffer_write_byte(s, xc);
    buffer_write_byte(s, x_size & 0xFF);
  }

  return true;
}

void empack_write_string(buffer_t* s, em_byte_t* str, uint32_t str_size)
{
  if (str_size <= 31) {
    buffer_write_byte(s, 0xA0 + str_size);
  } else {
    empack_write_size(s, 0xDB, 0xDA, 0xD9, str_size);
  }

  buffer_write(s, str, str_size);
}

void empack_write_bin(buffer_t* s, em_byte_t* bin, uint32_t bin_size)
{
  empack_write_size(s, 0xC6, 0xC5, 0xC4, bin_size);
  buffer_write(s, bin, bin_size);
}

static bool empack_write_header_size(buffer_t *s, uint8_t xa, uint8_t xb, uint8_t xc, uint32_t x_size)
{
  bool b = true;
  if (x_size > USHRT_MAX) {
    b &= buffer_write_byte(s, xa);
    for (uint8_t i = 24; i >= 0; i = i - 8)
      b &= buffer_write_byte(s, (x_size >> i) & 0xFF);
  } else if (x_size > 15) {
    b &= buffer_write_byte(s, xb);
    for (uint8_t i = 8; i >= 0; i = i - 8)
      b &= buffer_write_byte(s, (x_size >> i) & 0xFF);
  } else {
    b &= buffer_write_byte(s, xc + x_size);
  }

  return b;
}

void empack_write_array_start(buffer_t* s, uint32_t array_size)
{
  empack_write_header_size(s, 0xDD, 0xDC, 0x90, array_size);
}

void empack_write_map_start(buffer_t* s, uint32_t map_size)
{
  empack_write_header_size(s, 0xDE, 0xDF, 0x80, map_size);
}


#ifdef EMPACK_PRINTF

void buffer_flush(char* buf, uint16_t buf_size)
{
  uint16_t i;
  for (i = 0; i < buf_size; i++) {
    buf[i] = ' ';
  }
  buf[buf_size] = '\0';
}

void print_string(buffer_t* output, char* str, uint16_t str_size)
{
  uint16_t i;
  for (i = 0; i < str_size; i++) {
    buffer_write(output, str[i], 1);
  }
}

void print_bin(buffer_t* output, em_byte_t* str, uint16_t str_size)
{
  uint16_t i;
  for (i = 0; i < str_size; i++) {
    buffer_write(output, sprintf(" 0x%02X", str[i]), 5);
  }
}

void empack_to_json(buffer_t* output, buffer_t* input, em_size_t BUFFER_SIZE)
{
  uint8_t i;
  uint16_t buf_size = BUFFER_SIZE;
  char buf[BUFFER_SIZE + 1];

  empack_type_t next = empack_next_type(input);

  switch (next) {

  case EMPACK_MAP:
    uint32_t map_size;
    empack_read_map_size(input, &map_size);

    buffer_write_byte(output, F("{"));
    for (i = 0; i < map_size; i++) {

      if (i != 0)
        buffer_write_byte(output, F(", "));

      flush_buf(buf, buf_size);
      uint32_t r_size;
      empack_read_string(input, buf, buf_size, &r_size);
      buffer_write_byte(output, F("\""));
      print_string(output, buf, r_size);
      buffer_write_byte(output, F("\": "));
      empack_to_json(output, input, BUFFER_SIZE);
    }
    buffer_write_byte(output, F("}"));
    break;

  case EMPACK_ARRAY:
    uint32_t array_size;
    empack_read_array_size(input, &array_size);
    buffer_write_byte(output, F("["));
    for (i = 0; i < array_size; i++) {
      if (i != 0)
        buffer_write_byte(output, F(", "));
      empack_to_json(output, input, BUFFER_SIZE);
    }
    buffer_write_byte(output, F("]"));
    break;

  case EMPACK_NIL:
    empack_read_nil(input);
    buffer_write_byte(output, F("nil"));
    break;

  case EMPACK_BOOL:
    bool b;
    empack_read_bool(input, &b);
    if (b) {
      buffer_write_byte(output, F("true"));
    } else {
      buffer_write_byte(output, F("false"));
    }

    break;

  case EMPACK_SINT:
    int32_t i = 0;
    uint8_t* p = (uint8_t*)&i;
    empack_read_integer(input, p, 4);
    buffer_write_byte(output, i);
    break;

  case EMPACK_UINT:
    uint32_t u = 0;
    uint8_t* p = (uint8_t*)&u;
    empack_read_integer(input, p, 4);
    buffer_write_byte(output, u);
    break;

  case EMPACK_STRING:
    flush_buf(buf, buf_size);
    uint32_t r_size;
    empack_read_string(input, buf, buf_size, &r_size);
    buffer_write_byte(output, F("\""));
    print_string(output, buf, r_size);
    buffer_write_byte(output, F("\""));
    break;

  case EMPACK_BIN:
    flush_buf(buf, buf_size);
    uint32_t r_size;
    empack_read_bin(input, (unsigned char*)buf, buf_size, &r_size);
    print_bin(output, (unsigned char*)buf, r_size);
    buffer_write_byte(output, F("'"));
    break;
  }
}
#endif
