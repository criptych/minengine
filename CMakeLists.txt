################################################################################
##
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(minengine CXX)

SET(CMAKE_BUILD_TYPE Debug)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
ENDIF()

################################################################################

FIND_PACKAGE(SFML 2.2 REQUIRED COMPONENTS system window graphics audio network)

INCLUDE_DIRECTORIES(${SFML_INCLUDE_DIR})

################################################################################

ADD_LIBRARY(engine src/engine.cpp)

ADD_EXECUTABLE(client src/client.cpp)
TARGET_LINK_LIBRARIES(client engine ${SFML_LIBRARIES})

ADD_EXECUTABLE(server src/server.cpp)
TARGET_LINK_LIBRARIES(server engine ${SFML_LIBRARIES})

ADD_EXECUTABLE(testmain test/testmain.cpp)
TARGET_LINK_LIBRARIES(testmain engine)
SET_TARGET_PROPERTIES(testmain PROPERTIES OUTPUT_NAME test)

################################################################################
##  EOF
################################################################################

