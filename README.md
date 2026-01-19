# TimeIt – Header-Only C / C++ Scope Timer

TimeIt is a lightweight, header-only library for logging function and scope execution times in **C and C++**.

---

## Features

* **Header-only** – just `#include "timeit.h"`
* **C++ RAII** and **C GCC/Clang cleanup**
* **Nested scope timing** with proper indentation and depth tracking
* **Tree (`.log`)** and **CSV (`.csv`)** output files
* **Optional compile-time disable** (`LOG_ENABLE_TIMING=0`)

---

## Installation

Just copy `timeit.h` into your project and include it:

```cpp
#include "timeit.h"
```

No CMake or build system required. Works with any C++ compiler or GCC/Clang C compiler.

---

## Usage

### 1. Set output files

```cpp
SET_TIME_IT_OUTPUT_FILE_BASENAME("timings");
```

This creates:

* `timings.log` – tree-style indented output
* `timings.csv` – CSV output

---

### 2. Time a scope

```cpp
TIME_IT("category name");
```

* Mandatory category string.
* Automatically logs elapsed time when the scope ends.
* Nested scopes are handled automatically.

---

### Example C++ Program

```cpp
#include <stdio.h>
#include <thread>
#include <chrono>
#include "timeit.h"

void sub_task() {
    TIME_IT("sub_task");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

int main() {
    SET_TIME_IT_OUTPUT_FILE_BASENAME("timings");

    TIME_IT("main scope");
    printf("Hello World\n");

    sub_task();

    {
        TIME_IT("extra work");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}
```

---

### Example C Program (GCC / Clang)

```c
#include <stdio.h>
#include <unistd.h>
#include "timeit.h"

void sub_task() {
    TIME_IT("sub_task");
    usleep(10000); // 10 ms
}

int main() {
    SET_TIME_IT_OUTPUT_FILE_BASENAME("timings");

    TIME_IT("main scope");
    printf("Hello World\n");

    sub_task();

    {
        TIME_IT("extra work");
        usleep(5000); // 5 ms
    }

    return 0;
}
```

---

## Output

### Tree (`timings.log`)

```
  void sub_task() [sub_task]: 1.007e7
  int main() [extra work]: 5.065e6
int main() [main scope]: 1.517e7
```

### CSV (`timings.csv`)

```
1,void sub_task(),sub_task,1.007e7
1,int main(),extra work,5.065e6
0,int main(),main scope,1.517e7
```

* All elapsed times are in **scientific notation**
* Exponents are multiples of 3 (aligned with SI units)
* Nested scope depth is recorded in CSV
* Outer scopes appear last, inner scopes first (due to RAII destruction order)

---

## Configuration

You can override defaults **before including the header**:

```cpp
#define LOG_ENABLE_TIMING 0      // Disable all timing
#define LOG_OUTPUT_TREE 0        // Disable tree output
#define LOG_OUTPUT_CSV 1         // Enable CSV output
#include "timeit.h"
```

---

## License

MIT License — free to use, modify, and distribute.
