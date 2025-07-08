# OptiHeap
Optiheap is a custom memory management system that also implements a garbage collector based on the principle of reference counting

## Building and Running
To compile and run the program, execute the following commands in your project directory

```bash
gcc -g -o test main.c custom_memory.c -Wall -Wextra
```

Then, run the executable:

```bash
./test
```

## Testing with Valgrind

Check for memory leaks by running:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./test
```