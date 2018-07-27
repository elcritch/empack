
#ifndef __EMPACK_BUFFER__
#define __EMPACK_BUFFER__

#include <stdint.h>

#ifndef EM_SIZE_TYPE
#define EM_SIZE_TYPE
typedef int em_size_t;
#else
typedef EM_SIZE_TYPE em_size_t;
#endif

#ifndef EM_BYTE_TYPE
#define EM_BYTE_TYPE
typedef char em_byte_t;
#else
typedef EM_BYTE_TYPE em_byte_t;
#endif

// ====================== Buffer ============== //

#ifdef __cplusplus
extern "C" {
#endif

struct byte_buff {
  em_byte_t* buf;
  em_size_t pos;
  em_size_t max;
  em_size_t len;
};

typedef struct byte_buff buffer_t;

void buffer_init(buffer_t* data, em_byte_t* data_buffer, em_size_t data_len);

int buffer_available(buffer_t* data);

int buffer_read(buffer_t* data, em_byte_t* buffer, int length);

int16_t buffer_read_byte(buffer_t* data);

em_size_t buffer_write(buffer_t* data, em_byte_t* buffer, em_size_t data_len);

em_size_t buffer_write_byte(buffer_t* data, em_byte_t byte);

int buffer_peek(buffer_t* data);

void buffer_flush(buffer_t* data);

void buffer_clear(buffer_t* data);

void buffer_reset(buffer_t* data);

void buffer_reset_all(buffer_t* data);

#endif
