cmake_minimum_required(VERSION 3.14)
project(CAHDOutputData)
configure_file(output.csv ${CMAKE_CURRENT_BINARY_DIR}/output.csv COPYONLY)
configure_file(BMS1output1000.csv ${CMAKE_CURRENT_BINARY_DIR}/BMS1output1000.csv COPYONLY)