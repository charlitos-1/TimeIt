# TimeIt – Header-Only C / C++ Scope Timer

TimeIt is a lightweight, header-only library for logging function and scope execution times in **C and C++**.

---

## Features

* **Header-only** – just `#include "time_it.h"`
* **C++ RAII** and **C GCC/Clang cleanup**
* **Nested scope timing** with proper indentation and depth tracking
* **Tree (`.log`)** and **CSV (`.csv`)** output files
* **Optional compile-time disable** (`LOG_ENABLE_TIMING=0`)

---

## Installation

Just copy `time_it.h` into your project and include it:

```cpp
#include "time_it.h"
```

No CMake or build system required.

### C Compatibility

**Linking requirement:** When compiling for a C project, you must link against the math library using the `-lm` flag:

```bash
gcc -o program program.c -lm
```

**C99 Compatibility:** TimeIt automatically defines `_POSIX_C_SOURCE` when compiling C code (non-C++) to ensure POSIX functions like `clock_gettime()` are available. This works seamlessly with `-std=gnu99` or newer C standards.

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
#include "time_it.h"

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
#include "time_it.h"

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
1,sub_task,sub_task,1.007e7
1,main,extra work,5.065e6
0,main,main scope,1.517e7
```

* All elapsed times are in **scientific notation**
* Exponents are multiples of 3 (aligned with SI units)
* Function names are simple (`__func__`), not full signatures, avoiding comma conflicts
* Nested scope depth is recorded in CSV
* Outer scopes appear last, inner scopes first (due to RAII destruction order)

---

## Configuration

You can override defaults **before including the header**:

```cpp
#define LOG_ENABLE_TIMING 0      // Disable all timing
#define LOG_OUTPUT_TREE 0        // Disable tree output
#define LOG_OUTPUT_CSV 1         // Enable CSV output
#include "time_it.h"
```

---

## License

MIT License — free to use, modify, and distribute.
