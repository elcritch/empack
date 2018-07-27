
void buffer_init(buffer_t * sb, uint8_t *data_buffer, ssize_t data_len)
 {
   sb->data = data_buffer;
   sb->len = data_len;
   sb->pos = 0;
 }

int buffer_available(buffer_t * sb) {
  return sb->len - sb->pos;
}

int buffer_read_bytes(buffer_t * sb, uint8_t * buffer, int length) {
  if (buffer_available(sb) < length)
        return -1;

    uint32_t i;
    for(i=0; i < length; i++) {
      *(buffer+i) = *(sb->data + sb->pos++);
    }

    sb->max = sb->pos + 1;

    return length;
  }

ssize_t buffer_write(buffer_t * sb, uint8_t d) {
    if (buffer_available(sb) <= 0)
      return -1;

    sb->max = sb->pos+1;
    sb->data[sb->pos++] = d;

    return 1;
};

ssize_t buffer_write_bytes(buffer_t * sb, uint8_t d) {
  if (buffer_available(sb) <= 0)
    return -1;

  sb->max = sb->pos+1;
  sb->data[sb->pos++] = d;

  return 1;
};

int buffer_peek(buffer_t * sb) {
  return sb->data[sb->pos];
}

void buffer_flush(buffer_t * sb) {
  ssize_t i;
  for (i = 0; i < sb->len; ++i) {
    sb->data[i] = 0;
  }
  sb->pos = 0;
  sb->max = 0;
}

void buffer_clear(buffer_t * sb) {
  buffer_flush(sb);
}

void buffer_reset(buffer_t * sb) {
  sb->pos = 0;
}

void buffer_reset_all(buffer_t * sb) {
        sb->pos = 0;
        sb->max = 0;
}

