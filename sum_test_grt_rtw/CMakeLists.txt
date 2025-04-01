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

