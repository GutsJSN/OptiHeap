// custom_memory.h

#ifndef CUSTOM_MEMORY_H
#define CUSTOM_MEMORY_H

#include <stddef.h>

// Define the memory pool size
#define MEMORY_POOL_SIZE (64 * 1024) // 64KB

// Function to initialize the memory pool
void init_memory_pool(size_t size);

// Function to allocate memory
void* custom_malloc(size_t size);

// Function to free memory
void custom_free(void* ptr);

// Function to assign one pointer to another
void assign(void** dest, void* src);

// Function to increment reference count
void increment_ref(void* ptr);

// Function to decrement reference count
void decrement_ref(void* ptr);

// Function to free the entire memory pool
void free_memory_pool(void);

#endif // CUSTOM_MEMORY_H 