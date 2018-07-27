
#ifndef __EMPACK_BUFFER__
#define __EMPACK_BUFFER__

#ifndef em_size_t
typedef int em_size_t;
#endif

#ifndef em_byte_t
typedef const char em_byte_t;
#endif

// ====================== Buffer ============== //

#ifdef __cplusplus
extern "C" {
#endif

struct byte_buff {
  char* buf;
  em_size_t pos;
  em_size_t max;
  em_size_t len;
};

typedef struct byte_buff buffer_t;

void buffer_init(buffer_t* data, char* data_buffer, em_size_t data_len);

int buffer_available(buffer_t* data);

int buffer_read_bytes(buffer_t* data, em_byte_t* buffer, int length);

uint16_t buffer_read_byte(buffer_t* data);

em_size_t buffer_write_bytes(buffer_t* data, em_byte_t* buffer, uint32_t data_len);

int buffer_peek(buffer_t* data);

void buffer_flush(buffer_t* data);

void buffer_clear(buffer_t* data);

void buffer_reset(buffer_t* data);

void buffer_reset_all(buffer_t* data);

#endif
