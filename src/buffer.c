#include "buffer.h"
#include <sys/param.h>

size_t Buffer_Write(BufferWriter *bw, void *data, size_t len) {
  Buffer *buf = bw->buf;
  if (buf->offset + len > buf->cap) {
    do {
      buf->cap = MAX(buf->cap * 4 / 5, 1024 * 1024);
    } while (buf->offset + len > buf->cap);

    buf->data = realloc(buf->data, buf->cap);
    bw->pos = buf->data + buf->offset;
  }
  memcpy(bw->pos, data, len);
  bw->pos += len;
  buf->offset += len;
  return len;
}

/**
Truncate the buffer to newlen. If newlen is 0 - trunacte capacity
*/
size_t Buffer_Truncate(Buffer *b, size_t newlen) {
  if (newlen == 0) {
    newlen = Buffer_Offset(b);
  }

  b->data = realloc(b->data, newlen);
  b->cap = newlen;
  return newlen;
}

BufferWriter NewBufferWriter(Buffer *b) {
  BufferWriter ret = {.buf = b, .pos = b->data};
  return ret;
}

BufferReader NewBufferReader(Buffer *b) {
  BufferReader ret = {.buf = b, .pos = b->data};
  return ret;
}

/* Initialize a static buffer and fill its data */
void Buffer_Init(Buffer *b, size_t cap) {
  b->cap = cap;
  b->offset = 0;
  b->data = malloc(cap);
}

/**
Allocate a new buffer around data. If type is BUFFER_WRITE, freeing this buffer
will also free the underlying data
*/
Buffer *NewBuffer(char *data, size_t len) {
  Buffer *buf = malloc(sizeof(Buffer));
  buf->cap = len;
  buf->data = data;
  buf->offset = 0;

  return buf;
}

/**
Read len bytes from the buffer into data. If offset + len are over capacity
- we do not read and return 0
@return the number of bytes consumed
*/
inline size_t Buffer_Read(BufferReader *br, void *data, size_t len) {
  // no capacity - return 0
  Buffer *b = br->buf;
  if (b->offset + len > b->cap) {
    return 0;
  }

  data = memcpy(data, br->pos, len);
  br->pos += len;
  b->offset += len;

  return len;
}

/**
Consme one byte from the buffer
@return 0 if at end, 1 if consumed
*/
inline size_t Buffer_ReadByte(BufferReader *b, char *c) {
  // if (BufferAtEnd(b)) {
  //     return 0;
  // }
  *c = *b->pos++;
  ++b->buf->offset;
  return 1;
}

/**
Skip forward N bytes, returning the resulting offset on success or the end
position if where is outside bounds
*/
inline size_t Buffer_Skip(BufferReader *br, int bytes) {
  // if overflow - just skip to the end
  Buffer *b = br->buf;
  if (b->offset + bytes > b->cap) {
    br->pos = b->data + b->cap;
    b->offset = b->cap;
    return b->cap;
  }

  br->pos += bytes;
  b->offset += bytes;
  return b->offset;
}

/**
Seek to a specific offset. If offset is out of bounds we seek to the end.
@return the effective seek position
*/
inline size_t Buffer_Seek(BufferReader *br, size_t where) {
  Buffer *b = br->buf;

  where = MIN(where, b->cap);
  br->pos = b->data + where;
  b->offset = where;
  return where;
}

inline size_t BufferOffset(Buffer *ctx) {
  return ctx->offset;
}

inline size_t Buffer_Capacity(Buffer *ctx) {
  return ctx->cap;
}

inline int Buffer_AtEnd(Buffer *ctx) {
  return ctx->offset >= ctx->cap;
}
