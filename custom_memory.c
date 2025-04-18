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
    int ref_count;
    struct Block* next;
} Block;

typedef struct FreeBlock {
    size_t size;
    struct FreeBlock* next;
} FreeBlock;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static Block* allocated_blocks = NULL;
static FreeBlock* free_list = NULL;

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
    // Align size to ensure proper alignment
    size = (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);

    // Check if there's a suitable free block
    FreeBlock** current = &free_list;
    while (*current) {
        if ((*current)->size >= size) {
            // Found a suitable block
            FreeBlock* block = *current;
            *current = (*current)->next;
            
            // Create a tracking block
            Block* new_block = (Block*)malloc(sizeof(Block));
            new_block->address = (void*)block;
            new_block->size = size;
            new_block->ref_count = 1;
            new_block->next = allocated_blocks;
            allocated_blocks = new_block;
            
            printf("Allocated memory at %p of size %zu from free list\n", new_block->address, new_block->size);
            return new_block->address;
        }
        current = &(*current)->next;
    }

    // No suitable free block found, allocate from the pool
    if (memory_pool == NULL || size > memory_pool_size) {
        return NULL;
    }
    
    void* ptr = memory_pool;
    memory_pool = (char*)memory_pool + size;
    memory_pool_size -= size;

    // Create a tracking block
    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->address = ptr;
    new_block->size = size;
    new_block->ref_count = 1;
    new_block->next = allocated_blocks;
    allocated_blocks = new_block;
    
    printf("Allocated memory at %p of size %zu from pool\n", new_block->address, new_block->size);
    return ptr;
}

// Function to increment reference count
void increment_ref(void* ptr) {
    Block* current = allocated_blocks;
    while (current) {
        if (current->address == ptr) {
            current->ref_count++;
            printf("Incrementing ref count for %p, new count: %d\n", ptr, current->ref_count);
            return;
        }
        current = current->next;
    }
    printf("Pointer not found in allocated blocks\n");
}

// Function to decrement reference count and free memory if needed
void decrement_ref(void* ptr) {
    Block** current = &allocated_blocks;
    while (*current) {
        if ((*current)->address == ptr) {
            (*current)->ref_count--;
            printf("Decrementing ref count for %p, new count: %d\n", ptr, (*current)->ref_count);
            if ((*current)->ref_count <= 0) {
                // Add the block to the free list
                FreeBlock* block = (FreeBlock*)ptr;
                block->size = (*current)->size;
                block->next = free_list;
                free_list = block;
                
                printf("Added memory at %p of size %zu to free list\n", ptr, (*current)->size);

                // Remove the block from the tracking list
                Block* to_free = *current;
                *current = (*current)->next;
                free(to_free);
            }
            return;
        }
        current = &(*current)->next;
    }
    printf("Pointer not found in allocated blocks\n");
}

// Function to free memory explicitly
void custom_free(void* ptr) {
    if (ptr) {
        decrement_ref(ptr);
    }
}

// Function to assign one pointer to another
void assign(void** dest, void* src) {
    if (dest && *dest) {
        decrement_ref(*dest);
    }
    if (dest) {
        *dest = src;
        if (src) {
            increment_ref(src);
        }
    }
}

// Function to free the entire memory pool
void free_memory_pool(void) {
    if (memory_pool) {
        if (munmap(memory_pool, MEMORY_POOL_SIZE) == 0) {
            printf("Memory pool successfully freed\n");
        } else {
            perror("munmap");
        }
        memory_pool = NULL;
        memory_pool_size = 0;
        
        // Free all allocated blocks
        while (allocated_blocks) {
            Block* to_free = allocated_blocks;
            allocated_blocks = allocated_blocks->next;
            free(to_free);
        }
        
        // Clear the free list
        free_list = NULL;
    }
} 