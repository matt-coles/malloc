#include <stdint.h>

struct header_t {
  size_t size;
  unsigned int is_free;
  struct header_t* next;
};

struct header_t* reuse_block(size_t size);
void* malloc(size_t size);
void free(void* ptr);
