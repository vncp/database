PROJECT(DATABASES_PA1)
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "{PROJECT_SOURCE_DIR}/CMakeModules")
SET(CMAKE_CXX_STANDARD 17)
SET(CXX17_FLAGS -std=gnu++17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX16_FLAGS}")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

INCLUDE(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/2fe3bd994b3189899d93f1d5a881e725e046fdc2.zip
)
SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

IF(UNIX)
  ADD_DEFINITIONS(-DUNIX)
ENDIF(UNIX)

INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/include"
)

SET(INCLUDES ${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${INCLUDES})

FILE(GLOB_RECURSE SOURCES "src/*.cpp")

# Build test binary
enable_testing()
ADD_EXECUTABLE(
  lexer_test
  ${SOURCES}
)

# Link to GoogleTest
TARGET_LINK_LIBRARIES(
  lexer_test
  gtest_main
)

# Discover and run tests in binary
include(GoogleTest)
gtest_discover_tests(lexer_test)

#Build project binary
#FILE(GLOB_RECURSE SOURCES "src/*.cpp")
#ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})

#TARGET_LINK_LIBRARIES(${PROJECT_NAME})