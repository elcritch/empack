/**
 * Copyright (C) 2018 Jaremy Creechley
 *
 */

#ifndef __EMPACK_HEADER__
#define __EMPACK_HEADER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "em_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EMPACK_JSON_BUFF_SIZE
#define EMPACK_JSON_BUFF_SIZE 128
#endif


// ====================== TYPES ============== //

enum empack_types {
  EMPACK_EMPTY = 0xFF,
  EMPACK_NIL = 0xC0,
  EMPACK_BOOL = 0xC2,
  EMPACK_UINT = 0xCC,
  EMPACK_SINT = 0xD0,
  EMPACK_FLOAT = 0xCA,
  EMPACK_STRING = 0xD9,
  EMPACK_BIN = 0xC4,
  EMPACK_EXT = 0xC7,
  EMPACK_ARRAY = 0xDC,
  EMPACK_MAP = 0xDE,
  EMPACK_UNKNOWN = 0x00,
};

typedef enum empack_types empack_type_t;
struct byte_buff;

#define EMPACK_UINT_SMALL_MAX 224
#define EMPACK_SINT_SMALL_MAX 128

// ====================== API ============== //

empack_type_t empack_next_type(buffer_t* s);
bool empack_next_skip(buffer_t* s, empack_type_t* skip_type);
bool empack_next_copy(buffer_t* s, buffer_t* out, empack_type_t* skip_type);

bool empack_read_nil(buffer_t* s);
bool empack_read_bool(buffer_t* s, bool* b);
bool empack_read_sint(buffer_t* s, em_byte_t* b, uint8_t count_bytes);
bool empack_read_uint(buffer_t* s, em_byte_t* b, uint8_t count_bytes);
bool empack_read_float(buffer_t* s, float* f);

bool empack_read_string_sz(buffer_t* s, char* str, uint32_t count_bytes, uint32_t* str_size);
bool empack_read_bin_sz(buffer_t* s, em_byte_t* bin, uint32_t count_bytes, uint32_t* bin_size);

bool empack_read_array_size(buffer_t* s, uint32_t* array_size);
bool empack_read_map_size(buffer_t* s, uint32_t* map_size);

// ======= Basic Types ===== //
void empack_write_nil(buffer_t* s);
void empack_write_bool(buffer_t* s, bool b);

void empack_write_u8(buffer_t* s, uint8_t u);
void empack_write_u16(buffer_t* s, uint16_t u);
void empack_write_u32(buffer_t* s, uint32_t u);
void empack_write_u64(buffer_t* s, uint64_t u);
void empack_write_i8(buffer_t* s, int8_t i);
void empack_write_i16(buffer_t* s, int16_t i);
void empack_write_i32(buffer_t* s, int32_t i);
void empack_write_i64(buffer_t* s, int64_t i);

void empack_write_float(buffer_t* s, float f);

// ======= Data Types ===== //
void empack_write_string(buffer_t* s, em_byte_t* str, uint32_t str_size);
void empack_write_bin(buffer_t* s, em_byte_t* b, uint32_t bin_size);

// ======= String Types ===== //
void empack_write_array_start(buffer_t* s, uint32_t array_size);
void empack_write_map_start(buffer_t* s, uint32_t map_size);

#ifdef EMPACK_JSON
void empack_to_json(buffer_t* output, buffer_t* input, emsize_t buffer_size);
#endif // EMPACK_JSON


#ifdef __cplusplus
}
#endif

#endif
