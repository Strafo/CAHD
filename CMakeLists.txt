cmake_minimum_required(VERSION 3.14)
project(CAHD)
set(CMAKE_CXX_STANDARD 14)
find_package( Threads )
find_package(PythonLibs REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_subdirectory(inputData)
add_subdirectory(output)

add_executable(Cahd mainCahd.cpp TransactionsTable.h TransactionsTable.cpp TransactionWrapper.h Cahd.h Cahd.cpp
        Histogram.cpp Histogram.h TransactionGroup.h TransactionGroup.cpp TransactionDataAnonymizationTypes.h )
target_link_libraries( Cahd ${CMAKE_THREAD_LIBS_INIT} ${PYTHON_LIBRARIES}  ${Boost_LIBRARIES})

add_executable(KLDIVERGENCE klDivergence.cpp TransactionsTable.h TransactionsTable.cpp TransactionWrapper.h Cahd.h Cahd.cpp
        Histogram.cpp Histogram.h TransactionGroup.h TransactionGroup.cpp TransactionDataAnonymizationTypes.h )
target_link_libraries( KLDIVERGENCE ${CMAKE_THREAD_LIBS_INIT}  ${Boost_LIBRARIES}  ${PYTHON_LIBRARIES})


