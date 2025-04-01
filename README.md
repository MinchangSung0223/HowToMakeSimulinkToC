
1. Simulink 및 Simulink Coder설치
2. 간단한 Simulink 모델 생성
![image](https://github.com/user-attachments/assets/e91cdce5-9120-4105-97c7-9054230ba017)
3. 세팅 변경
   ![image](https://github.com/user-attachments/assets/b2c46966-8760-4ee7-98bf-d648b0e1aabb)
  ![image](https://github.com/user-attachments/assets/f669d867-2d6f-49d5-aaa5-b6a61dfb5b60)
![image](https://github.com/user-attachments/assets/b06e9d84-4d2b-4ade-9ef4-550d90fff02d)
![image](https://github.com/user-attachments/assets/b01655e1-4a88-4b64-9188-40cb5bce04bf)

4. Generate Code
![image](https://github.com/user-attachments/assets/031c3481-b9fc-4bff-b24f-307adedd99b3)

5. 생성된 코드 폴더 확인
![image](https://github.com/user-attachments/assets/2f95cd1b-6542-4fe3-ad9c-19d52803b0d7)

6.아래 두 폴더를 생성된 코드 폴더에 복사.
```bash
    C:\Program Files\MATLAB\R2023b\simulink\include  

    C:\Program Files\MATLAB\R2023b\rtw
```

7. Ubuntu환경으로 복사

8. CMakeLists.txt 수정
```bash
cmake_minimum_required(VERSION 3.10)
project(sum_test_c)

set(CMAKE_C_STANDARD 99)

# include 디렉토리
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/rtw/c/src    
      ${CMAKE_SOURCE_DIR}/include     # 복사한 rtw 내부 경로
)


# 소스 목록 (ert_main.c는 직접 추가하거나 따로 작성해야 함)
set(SOURCES
    sum_test.c
    rt_nonfinite.c
    rtGetInf.c
    rtGetNaN.c
rtw/c/src/rt_logging.c     # <-- 이 줄 추가
    main.c  # 직접 작성한 main 파일 (예: ert_main.c 역할)
)

add_executable(sum_test ${SOURCES})
target_link_libraries(sum_test m)

```
9. main.c 작성
```bash
#include <stdio.h>
#include <time.h>
#include "sum_test.h"

int main() {
    struct timespec start, end;
    double elapsed_ms;

    // 모델 초기화
    sum_test_initialize();

    // 시간 측정 시작
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 1000; i++) {
        sum_test_U.Input = 3.0 * i;
        sum_test_U.Input1 = 7.0;

        sum_test_step();

        // 결과 확인 (옵션)
        // printf("Output: %f\n", sum_test_Y.Out1);
    }

    // 시간 측정 끝
    clock_gettime(CLOCK_MONOTONIC, &end);

    // 경과 시간(ms) 계산
    elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed_ms += (end.tv_nsec - start.tv_nsec) / 1e6;

    printf("Execution time for 1000 iterations: %.3f ms\n", elapsed_ms);

    // 모델 종료
    sum_test_terminate();

    return 0;
}

```
