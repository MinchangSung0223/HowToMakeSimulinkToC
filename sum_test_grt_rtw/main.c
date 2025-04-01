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

