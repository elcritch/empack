
#include "em_buffer.h"


void buffer_init(buffer_t * data, em_byte_t *data_buffer, em_size_t data_len)
 {
   data->buf = data_buffer;
   data->len = data_len;
   data->pos = 0;
 }

int buffer_available(buffer_t * data) {
  return data->len - data->pos;
}

int16_t buffer_read_byte(buffer_t * data) {
  if (buffer_available(data) < 1)
    return -1;

  data->max = data->pos+1;
  return data->buf[data->pos++];
}

int buffer_read(buffer_t * data, em_byte_t * buffer, em_size_t length) {
  if (buffer_available(data) < length)
        return -1;

    for (em_size_t i=0; i < length; i++) {
      *(buffer+i) = *(data->buf + data->pos++);
    }

    data->max = data->pos + 1;

    return length;
}

em_size_t buffer_write_byte(buffer_t * data, em_byte_t d) {
    if (buffer_available(data) <= 0)
      return -1;

    data->max = data->pos+1;
    data->buf[data->pos++] = d;

    return 1;
};

em_size_t buffer_write(buffer_t * data, em_byte_t* buffer, em_size_t data_len) {
  if (buffer_available(data) <= 0)
    return -1;

  for(em_size_t i=0; i < data_len; i++) {
    data->buf[data->pos++] = buffer[i];
  }

  data->max = data->pos;

  return 1;
};

int buffer_peek(buffer_t * data) {
  return data->buf[data->pos];
}

void buffer_flush(buffer_t * data) {
  em_size_t i;
  for (i = 0; i < data->len; ++i) {
    data->buf[i] = 0;
  }
  data->pos = 0;
  data->max = 0;
}

void buffer_clear(buffer_t * data) {
  buffer_flush(data);
}

void buffer_reset(buffer_t * data) {
  data->pos = 0;
}

void buffer_reset_all(buffer_t * data) {
        data->pos = 0;
        data->max = 0;
}

