cmake_minimum_required(VERSION 3.16)
project(Motiv_DualPot C)

set(CMAKE_C_STANDARD 99)

add_executable(Motiv_DualPot main.c DualPot_Drv.c dummy.c DualPot_Drv.h)