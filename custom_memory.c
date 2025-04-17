// custom_memory.c

#include "custom_memory.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEMORY_POOL_SIZE (64 * 1024) // 64KB

typedef struct Block {
    void* address;
    size_t size;
    struct Block* next;
} Block;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static Block* allocated_blocks = NULL;

// Function to initialize the memory pool
void init_memory_pool(size_t size) {
    if (memory_pool == NULL) {
        memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory_pool == MAP_FAILED) {
            perror("mmap");
            memory_pool = NULL;
        } else {
            memory_pool_size = size;
            memset(memory_pool, 0, size); // Initialize memory to zero
        }
    }
}

// Function to allocate memory from the pool
void* custom_malloc(size_t size) {
    if (memory_pool == NULL || size > memory_pool_size) {
        return NULL;
    }
    void* ptr = memory_pool;
    memory_pool = (char*)memory_pool + size;
    memory_pool_size -= size;

    // Track the allocated block
    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->address = ptr;
    new_block->size = size;
    new_block->next = allocated_blocks;
    allocated_blocks = new_block;

    return ptr;
}

// Function to free memory
void custom_free(void* ptr) {
    printf("Attempting to free memory at %p\n", ptr);
    Block** current = &allocated_blocks;
    while (*current) {
        if ((*current)->address == ptr) {
            // Free the memory using munmap
            if (munmap(ptr, (*current)->size) == 0) {
                printf("Memory at %p successfully freed\n", ptr);
            } else {
                perror("munmap");
            }

            // Remove the block from the list
            Block* to_free = *current;
            *current = (*current)->next;
            free(to_free);
            return;
        }
        current = &(*current)->next;
    }
    printf("Pointer not found in allocated blocks\n");
}

// Function to assign one pointer to another
void assign(void** dest, void* src) {
    if (dest) {
        *dest = src;
    }
} 