// main.c

#include <stdio.h>
#include "custom_memory.h"

int main() {
    // Initialize memory pool
    init_memory_pool(MEMORY_POOL_SIZE);

    // Test 1: Allocate and free memory
    printf("\nTest 1: Allocate and free memory\n");
    int* ptr1 = (int*)custom_malloc(sizeof(int));
    if (ptr1) {
        *ptr1 = 42;
        printf("Allocated memory at %p with value %d\n", ptr1, *ptr1);
        custom_free(ptr1);
    }

    // Test 2: Allocate memory, assign to another pointer, and free
    printf("\nTest 2: Allocate, assign, and free memory\n");
    int* ptr2 = (int*)custom_malloc(sizeof(int));
    int* ptr3 = NULL;
    if (ptr2) {
        *ptr2 = 100;
        printf("Allocated memory at %p with value %d\n", ptr2, *ptr2);
        assign((void**)&ptr3, ptr2);
        printf("Assigned memory to another pointer: %p\n", ptr3);
        printf("Value through ptr3: %d\n", *ptr3);
        custom_free(ptr2);  // This should not free the memory yet
        printf("After freeing ptr2, value through ptr3: %d\n", *ptr3);
        custom_free(ptr3);  // This should free the memory
    }

    // Test 3: Reuse memory from free list
    printf("\nTest 3: Reuse memory from free list\n");
    int* ptr4 = (int*)custom_malloc(sizeof(int));
    if (ptr4) {
        *ptr4 = 200;
        printf("Allocated memory at %p with value %d\n", ptr4, *ptr4);
        printf("This should reuse memory from the free list\n");
    }

    // Test 4: Array allocation
    printf("\nTest 4: Array allocation\n");
    size_t array_size = 5;
    int* array = (int*)custom_malloc(sizeof(int) * array_size);
    if (array) {
        printf("Allocated array at %p\n", array);
        for (size_t i = 0; i < array_size; i++) {
            array[i] = i * 10;
            printf("array[%zu] = %d\n", i, array[i]);
        }
        custom_free(array);
    }

    // Free the entire memory pool
    // printf("\nFreeing the entire memory pool\n");
    // free_memory_pool();

    return 0;
}