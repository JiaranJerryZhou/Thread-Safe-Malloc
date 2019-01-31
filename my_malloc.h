#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct space {
  struct space *next;
  struct space *prev;
  size_t size;
  // this variable record a segment of space is used or not
  int used;
  struct space *free_next;
  struct space *free_prev;
};
typedef struct space space_t;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

size_t node_size = sizeof(struct space);

space_t *head = NULL;

space_t *free_head = NULL;

space_t *tail = NULL;

// no lock version
__thread space_t *head_nolock = NULL;

__thread space_t *free_head_nolock = NULL;

__thread space_t *tail_nolock = NULL;

// Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

// Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

space_t *first_malloc(size_t size);
space_t *first_malloc_nolock(size_t size);

space_t *malloc_end(size_t size);
space_t *malloc_end_nolock(size_t size);

void fit_in(space_t *curr);
void fit_in_nolock(space_t *curr);

void free_space(void *ptr);
void free_space_nolock(void *ptr);

void add_free(space_t *curr);
void remove_free(space_t *curr);

void add_free_nolock(space_t *curr);
void remove_free_nolock(space_t *curr);

unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
#endif
