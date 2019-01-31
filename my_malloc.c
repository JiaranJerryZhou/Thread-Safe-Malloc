#include "my_malloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// this function is called when malloc for the first time
space_t *first_malloc(size_t size) {
  space_t *new = sbrk(size + node_size);
  new->next = NULL;
  new->prev = NULL;
  new->size = size;
  new->used = 1;
  new->free_next = NULL;
  new->free_prev = NULL;
  head = new;
  tail = new;
  return new;
}

// this function is called when we need to increase the heap
space_t *malloc_end(size_t size) {
  space_t *new = sbrk(size + node_size);
  new->next = NULL;
  new->prev = tail;
  new->size = size;
  new->used = 1;
  new->free_next = NULL;
  new->free_prev = NULL;
  tail->next = new;
  tail = new;
  return new;
}

// this function is called when a segment can fit in the amount space
void fit_in(space_t *curr) {
  curr->used = 1;
  remove_free(curr);
}

// Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size) {
  space_t *curr = free_head;
  space_t *best = NULL;
  int max_diff = -1;
  // malloc for the first time
  if (head == NULL) {
    space_t *new = first_malloc(size);
    return new + 1;
  }
  while (curr != NULL) {
    // find the best fit space
    if (curr->size >= size) {
      // current block has the exact space
      if (curr->size == size) {
        fit_in(curr);
        return curr + 1;
      }
      // calculate the difference
      else {
        int diff = curr->size - size;
        if (diff > max_diff) {
          best = curr;
          max_diff = diff;
        }
      }
    }
    curr = curr->free_next;
  }
  // there is a best fit
  if (best != NULL) {
    fit_in(best);
    return best + 1;
  }
  // need to malloc new space
  else {
    space_t *new = malloc_end(size);
    return new + 1;
  }
}

void ts_free_lock(void *ptr) { free_space(ptr); }

// Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size) {
  space_t *curr = free_head;
  space_t *best = NULL;
  int max_diff = -1;
  // malloc for the first time
  if (head == NULL) {
    space_t *new = first_malloc(size);
    return new + 1;
  }
  while (curr != NULL) {
    // find the best fit space
    if (curr->size >= size) {
      // current block has the exact space
      if (curr->size == size) {
        fit_in(curr);
        return curr + 1;
      }
      // calculate the difference
      else {
        int diff = curr->size - size;
        if (diff > max_diff) {
          best = curr;
          max_diff = diff;
        }
      }
    }
    curr = curr->free_next;
  }
  // there is a best fit
  if (best != NULL) {
    fit_in(best);
    return best + 1;
  }
  // need to malloc new space
  else {
    space_t *new = malloc_end(size);
    return new + 1;
  }
}

void ts_free_nolock(void *ptr) { free_space(ptr); }

void free_space(void *ptr) {
  space_t *curr = (space_t *)ptr - 1;
  curr->used = 0;
  // if the next piece of space is freed, merge them together
  space_t *next = curr->next;
  space_t *prev = curr->prev;
  if (next != NULL && next->used == 0) {
    // add_free(curr);
    remove_free(next);
    curr->size = curr->size + node_size + next->size;
    if (next->next != NULL) {
      next->next->prev = curr;
    }
    curr->next = next->next;
  }
  // if the prev piece of space is freed, merge them together
  if (prev != NULL && prev->used == 0) {
    // remove_free(curr);
    prev->next = curr->next;
    prev->size = prev->size + curr->size + node_size;
    if (curr->next != NULL) {
      curr->next->prev = prev;
    }
  } else {
    add_free(curr);
  }
}

// this function is used to sum up all segments
unsigned long get_data_segment_size() {
  space_t *curr = head;
  unsigned long total = 0;
  while (curr != NULL) {
    total += curr->size + node_size;
    curr = curr->next;
  }
  return total;
}

// this function is used to sum up all free segments
unsigned long get_data_segment_free_space_size() {
  space_t *curr = free_head;
  unsigned long total = 0;
  while (curr != NULL) {
    total += curr->size + node_size;
    curr = curr->free_next;
  }
  return total;
}

void add_free(space_t *curr) {
  if (free_head != NULL) {
    curr->free_next = free_head;
    free_head->free_prev = curr;
    free_head = curr;
    curr->free_prev = NULL;
  }
  // add to the head of free list
  else {
    free_head = curr;
    curr->free_next = NULL;
    curr->free_prev = NULL;
  }
}

void remove_free(space_t *curr) {
  // remove the head of free list
  if (free_head == curr) {
    free_head = curr->free_next;
  }
  if (curr->free_next != NULL) {
    curr->free_next->free_prev = curr->free_prev;
  }
  if (curr->free_prev != NULL) {
    curr->free_prev->free_next = curr->free_next;
  }
  curr->free_next = NULL;
  curr->free_prev = NULL;
}
