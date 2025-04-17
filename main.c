// main.c

#include <stdio.h>
#include "custom_memory.h"

int main() {
    // Initialize memory pool
    init_memory_pool(MEMORY_POOL_SIZE);

    // Ask the user for the size of the integer array
    size_t array_size;
    printf("Enter the size of the integer array: ");
    scanf("%zu", &array_size);

    // Allocate memory for the integer array
    int* int_array = (int*)custom_malloc(array_size * sizeof(int));
    if (!int_array) {
        printf("Memory allocation failed\n");
        return 1;
    }
    printf("Memory allocated for integer array at %p\n", int_array);

    // Take user input to fill the array
    printf("Enter %zu integers:\n", array_size);
    for (size_t i = 0; i < array_size; ++i) {
        printf("Enter integer %zu: ", i + 1);
        scanf("%d", &int_array[i]);
    }

    // Display the values using pointer dereferencing
    printf("The values in the array are:\n");
    for (size_t i = 0; i < array_size; ++i) {
        printf("Value at index %zu: %d\n", i, *(int_array + i));
    }

    // Free memory (no-op in this simple implementation)
    custom_free(int_array);
    printf("Memory freed\n");

    return 0;
} 