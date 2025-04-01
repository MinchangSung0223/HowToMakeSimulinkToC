# How to Convert a Simulink Model to C Code and Build with CMake on Ubuntu

## 1. Install Simulink and Simulink Coder
Ensure that both Simulink and Simulink Coder are installed on your system.

---

## 2. Create a Simple Simulink Model
Design a basic Simulink model that adds two inputs.

![image](https://github.com/user-attachments/assets/e91cdce5-9120-4105-97c7-9054230ba017)

---

## 3. Modify Configuration Parameters
Update the model settings to enable code generation.

- Set System target file to `grt.tlc`
- Language: `C`
- Hardware Board: `Intel->x86-64 (Linux 64)`

![image](https://github.com/user-attachments/assets/b2c46966-8760-4ee7-98bf-d648b0e1aabb)
![image](https://github.com/user-attachments/assets/f669d867-2d6f-49d5-aaa5-b6a61dfb5b60)
![image](https://github.com/user-attachments/assets/b06e9d84-4d2b-4ade-9ef4-550d90fff02d)
![image](https://github.com/user-attachments/assets/b01655e1-4a88-4b64-9188-40cb5bce04bf)

---

## 4. Generate Code
Click the "Build" button in the Simulink model to generate C code.

![image](https://github.com/user-attachments/assets/031c3481-b9fc-4bff-b24f-307adedd99b3)

---

## 5. Confirm Generated Code Folder
Locate the folder (e.g., `sum_test_grt_rtw`) that contains the generated code.

![image](https://github.com/user-attachments/assets/2f95cd1b-6542-4fe3-ad9c-19d52803b0d7)

---

## 6. Copy Required MATLAB Files
Copy the following folders from your MATLAB installation into the generated code folder:

```bash
C:\Program Files\MATLAB\R2023b\simulink\include  -->  include/
C:\Program Files\MATLAB\R2023b\rtw               -->  rtw/
```

---

## 7. Transfer Files to Ubuntu
Move the entire folder to your Ubuntu system using a method like `scp`, USB drive, or WSL file access.

---

## 8. Create or Modify `CMakeLists.txt`
Place this file in the root of your project:

```cmake
cmake_minimum_required(VERSION 3.10)
project(sum_test_c)

set(CMAKE_C_STANDARD 99)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/rtw/c/src
    ${CMAKE_SOURCE_DIR}/include
)

set(SOURCES
    sum_test.c
    rt_nonfinite.c
    rtGetInf.c
    rtGetNaN.c
    rtw/c/src/rt_logging.c
    main.c
)

add_executable(sum_test ${SOURCES})
target_link_libraries(sum_test m)
```

---

## 9. Write `main.c`
Create a file named `main.c` with the following content to run the model and measure performance:

```c
#include <stdio.h>
#include <time.h>
#include "sum_test.h"

int main() {
    struct timespec start, end;
    double elapsed_ms;

    sum_test_initialize();

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 1000; i++) {
        sum_test_U.Input = 3.0 * i;
        sum_test_U.Input1 = 7.0;

        sum_test_step();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed_ms += (end.tv_nsec - start.tv_nsec) / 1e6;

    printf("Execution time for 1000 iterations: %.3f ms\n", elapsed_ms);

    sum_test_terminate();
    return 0;
}
```

---

## 10. Build and Run
Build and run the executable on Ubuntu:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
./sum_test
```

---

This guide allows you to generate and run Simulink C code outside MATLAB on a Linux system using CMake.
