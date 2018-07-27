
#ifndef __EMPACK_BUFFER__
#define __EMPACK_BUFFER__

#ifndef em_size_t
typedef int em_size_t;
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

void buffer_setup(buffer_t* sb, char* data_buffer, em_size_t data_len);

int buffer_available(buffer_t* sb);

int buffer_read_bytes(buffer_t* sb, uint8_t* buffer, int length);

uint16_t buffer_read_byte(buffer_t* sb);

em_size_t buffer_write_bytes(buffer_t* sb, uint8_t* buffer, uint32_t data_len);

int buffer_peek(buffer_t* sb);

void buffer_flush(buffer_t* sb);

void buffer_clear(buffer_t* sb);

void buffer_reset(buffer_t* sb);

void buffer_reset_all(buffer_t* sb);

#endif
