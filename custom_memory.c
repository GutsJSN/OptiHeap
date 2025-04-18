// custom_memory.c

#include "custom_memory.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEMORY_POOL_SIZE (64 * 1024) // 64KB
#define ALIGNMENT 8 // Align to 8-byte boundaries

// Metadata stored at the beginning of each allocated block
typedef struct BlockHeader {
    size_t size;          // Size of the user data
    int ref_count;        // Reference count
    struct BlockHeader* next_free; // Pointer to next free block (used when in free list)
} BlockHeader;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static BlockHeader* free_list = NULL;
static BlockHeader* allocated_blocks = NULL;

// Returns the total size needed for a block including its header
static size_t total_size(size_t user_size) {
    return ((sizeof(BlockHeader) + user_size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1));
}

// Returns a pointer to user data from a block header
static void* block_to_ptr(BlockHeader* block) {
    return (void*)((char*)block + sizeof(BlockHeader));
}

// Returns a pointer to block header from user data
static BlockHeader* ptr_to_block(void* ptr) {
    return (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
}

// Function to initialize the memory pool
void init_memory_pool(size_t size) {
    if (memory_pool == NULL) {
        memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory_pool == MAP_FAILED) {
            perror("mmap");
            memory_pool = NULL;
        } else {
            memory_pool_size = size;
            memset(memory_pool, 0, size);
            
            // Initialize free list with the entire pool
            free_list = (BlockHeader*)memory_pool;
            free_list->size = size - sizeof(BlockHeader);
            free_list->ref_count = 0;
            free_list->next_free = NULL;
        }
    }
}

// Function to allocate memory from the pool
void* custom_malloc(size_t size) {
    if (size == 0 || memory_pool == NULL) {
        return NULL;
    }
    
    // Calculate the total size needed including header
    size_t block_size = total_size(size);
    
    // Find a suitable free block
    BlockHeader** current = &free_list;
    BlockHeader* prev = NULL;
    
    while (*current) {
        if ((*current)->size >= block_size) {
            // Found a suitable block
            BlockHeader* found_block = *current;
            
            // Remove from free list
            *current = found_block->next_free;
            
            // Split the block if it's significantly larger
            if (found_block->size >= block_size + sizeof(BlockHeader) + ALIGNMENT) {
                // Calculate the size for the new free block
                size_t remaining_size = found_block->size - block_size;
                
                // Create a new free block in the remaining space
                BlockHeader* new_free = (BlockHeader*)((char*)found_block + block_size);
                new_free->size = remaining_size - sizeof(BlockHeader);
                new_free->ref_count = 0;
                new_free->next_free = free_list;
                free_list = new_free;
                
                // Adjust the size of the found block
                found_block->size = block_size - sizeof(BlockHeader);
            }
            
            // Initialize the block
            found_block->ref_count = 1;
            
            // Add to allocated blocks list
            found_block->next_free = (BlockHeader*)allocated_blocks;
            allocated_blocks = found_block;
            
            void* user_ptr = block_to_ptr(found_block);
            printf("Allocated memory at %p of size %zu\n", user_ptr, size);
            return user_ptr;
        }
        prev = *current;
        current = &(*current)->next_free;
    }
    
    printf("Memory allocation failed: Not enough memory in pool\n");
    return NULL;
}

// Function to increment reference count
void increment_ref(void* ptr) {
    if (ptr == NULL) {
        printf("Cannot increment reference count of NULL pointer\n");
        return;
    }
    
    BlockHeader* block = ptr_to_block(ptr);
    block->ref_count++;
    printf("Incrementing ref count for %p, new count: %d\n", ptr, block->ref_count);
}

// Function to decrement reference count and free memory if needed
void decrement_ref(void* ptr) {
    if (ptr == NULL) {
        printf("Cannot decrement reference count of NULL pointer\n");
        return;
    }
    
    BlockHeader* block = ptr_to_block(ptr);
    block->ref_count--;
    printf("Decrementing ref count for %p, new count: %d\n", ptr, block->ref_count);
    
    if (block->ref_count <= 0) {
        // Remove from allocated blocks list
        BlockHeader** current = (BlockHeader**)&allocated_blocks;
        while (*current && *current != block) {
            current = (BlockHeader**)&(*current)->next_free;
        }
        
        if (*current) {
            *current = (BlockHeader*)block->next_free;
        }
        
        // Add to free list
        block->next_free = free_list;
        free_list = block;
        
        printf("Added memory at %p of size %zu to free list\n", ptr, block->size);
    }
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
        allocated_blocks = NULL;
        free_list = NULL;
    }
} 