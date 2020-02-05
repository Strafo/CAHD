cmake_minimum_required(VERSION 3.14)
project(CAHDInputData)

configure_file(originalData.csv ${CMAKE_CURRENT_BINARY_DIR}/originalData.csv COPYONLY)
configure_file(100_000Data.csv ${CMAKE_CURRENT_BINARY_DIR}/100_000Data.csv COPYONLY)
configure_file(synteticData.csv ${CMAKE_CURRENT_BINARY_DIR}/synteticData.csv COPYONLY)
configure_file(80_000Data.csv ${CMAKE_CURRENT_BINARY_DIR}/80_000Data.csv COPYONLY)
configure_file(shuf80_000Data.csv ${CMAKE_CURRENT_BINARY_DIR}/shuf80_000Data.csv COPYONLY)
configure_file(BMS1formatted.csv ${CMAKE_CURRENT_BINARY_DIR}/BMS1formatted.csv COPYONLY)
configure_file(shuf1000Data.csv ${CMAKE_CURRENT_BINARY_DIR}/shuf1000Data.csv COPYONLY)
configure_file(shufData.csv ${CMAKE_CURRENT_BINARY_DIR}/shufData.csv COPYONLY)
configure_file(BMS1formattedshuf.csv ${CMAKE_CURRENT_BINARY_DIR}/BMS1formattedshuf.csv COPYONLY)
configure_file(BMS1formatted6000.csv ${CMAKE_CURRENT_BINARY_DIR}/BMS1formatted6000.csv COPYONLY)
configure_file(BMS1formatted1000.csv ${CMAKE_CURRENT_BINARY_DIR}/BMS1formatted1000.csv COPYONLY)



