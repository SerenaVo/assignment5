
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <debug.h>

// Metadata stored before each allocated memory block.
typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block_t;

// Track all blocks in the order they were obtained.
static block_t *head = NULL;
static block_t *tail = NULL;

// Find the first free block that is large enough.
static block_t *find_free_block(size_t size) {
    block_t *current = head;

    while (current != NULL) {
        assert(current->size > 0);

        if (current->free && current->size >= size) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

// Request a new block from the operating system.
static block_t *request_block(size_t size) {
    if (size > (size_t) INTPTR_MAX - sizeof(block_t)) {
        return NULL;
    }

    size_t total_size = sizeof(block_t) + size;
    block_t *block = sbrk((intptr_t) total_size);

    if (block == (void *) -1) {
        return NULL;
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (head == NULL) {
        head = block;
    } else {
        tail->next = block;
    }

    tail = block;

    return block;
}

// Allocate memory using the first-fit strategy.
void *mymalloc(size_t s) {
    if (s == 0) {
        return NULL;
    }

    debug_printf("Malloc %zu bytes\n", s);

    block_t *block = find_free_block(s);

    if (block != NULL) {
        block->free = 0;
        return (void *) (block + 1);
    }

    block = request_block(s);

    if (block == NULL) {
        return NULL;
    }

    return (void *) (block + 1);
}

// Allocate memory and initialize all bytes to zero.
void *mycalloc(size_t nmemb, size_t s) {
    if (nmemb != 0 && s > (size_t) -1 / nmemb) {
        return NULL;
    }

    size_t total = nmemb * s;

    void *p = mymalloc(total);

    if (p != NULL) {
        memset(p, 0, total);
    }

    debug_printf("Calloc %zu bytes\n", total);

    return p;
}

// Mark an allocated block as free.
void myfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    block_t *block = (block_t *) ptr - 1;

    assert(block->size > 0);

    size_t size = block->size;
    block->free = 1;

    debug_printf("Freed %zu bytes\n", size);
}
