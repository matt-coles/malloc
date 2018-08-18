#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "bmalloc.h"

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct header_t *head, *tail = NULL;

void* malloc(size_t size) {
  void* new;
  struct header_t *header;
  size_t sbrk_size;
  if (size > SIZE_MAX - sizeof(struct header_t))
    return NULL;
  sbrk_size = size + sizeof(struct header_t);
  header = reuse_block(size);
  if (header) {
    header->is_free = 0;
    return (void*)(header+1);
  }
  new = sbrk(sbrk_size);
  if (new == (void*) -1)
    return NULL;
  header = new;
  header->size = size;
  header->is_free = 0;
  header->next = NULL;
  if (!head)
    head = header;
  if (tail)
    tail->next = header;
  tail = header;
  return (void*)(header+1);
}

void free(void* ptr) {
  struct header_t *header, *tmp;
  void *pbrk;

  if (!ptr)
    return;
  header = ((struct header_t*) ptr)-1; 
  pbrk = sbrk(0);
  if ((char*)ptr + header->size == pbrk) {
    if (head == tail)
      head = tail = NULL;
    else {
      tmp = head;
      while (tmp) {
        if (tmp->next == tail) {
          tmp->next = NULL;
          tail = tmp;
        }
        tmp = tmp->next;
      }
    }
    sbrk(0 - sizeof(struct header_t) - header->size);
  } else {
    header->is_free = 1;
  }
}

void *calloc(size_t num, size_t nsize) {
  size_t size;
  void *ptr;
  if (!num || !nsize)
    return NULL;
  size = num * nsize;
  if (nsize != size / num)
    return NULL;
  ptr = malloc(size);
  if (!ptr)
    return NULL;
  memset(ptr, 0, size);
  return ptr;
}

void *realloc(void *ptr, size_t size) {
  struct header_t *header;
  void *ret;
  if (!ptr || !size)
    return malloc(size);
  header = (struct header_t*)ptr - 1;
  if (header->size >= size)
    return ptr;
  ret = malloc(size);
  if (ret) {
    memcpy(ret, ptr, header->size);
    free(ptr);
  }
  return ret;
}

struct header_t* reuse_block(size_t size) {
  struct header_t* iter = head;
  if (size > SIZE_MAX - sizeof(struct header_t))
    return NULL;
  while (iter) {
    if (iter->is_free) {
      if (iter->size == size) {
        iter->is_free = 0;
        return iter;
      }
      if (iter->size > size - sizeof(struct header_t)) {
        struct header_t header;
        char* block = (char*) (((struct header_t*) iter)+1);
        struct header_t* tmp = (struct header_t*) block + size;
        header.size = iter->size - size - sizeof(struct header_t);
        header.is_free = 1;
        header.next = iter->next;
        *tmp = header;
        iter->size = size;
        iter->is_free = 0;
        iter->next = tmp;
        return iter;
      }
    }
    iter = iter->next;
  }
  return NULL;
}
