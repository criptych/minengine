################################################################################
##
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(minengine VERSION 0.1 LANGUAGES C CXX)

SET(CMAKE_BUILD_TYPE Debug)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
ENDIF()

################################################################################

FIND_PACKAGE(SFML 2.2 REQUIRED COMPONENTS system window graphics audio network)
FIND_PACKAGE(OpenGL REQUIRED)

################################################################################

SET(GLEW_INCLUDE_DIR lib/glew-1.13.0/include)
SET(GLEW_LIBRARY glew-1.13.0)
SET(GLEW_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

ADD_DEFINITIONS(-DGLEW_STATIC=1)

INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})

ADD_LIBRARY(${GLEW_LIBRARY} lib/glew-1.13.0/src/glew.c)

################################################################################

INCLUDE_DIRECTORIES(${SFML_INCLUDE_DIR})

ADD_LIBRARY(engine src/engine.cpp)

ADD_EXECUTABLE(client src/client.cpp)
SET_TARGET_PROPERTIES(client PROPERTIES VERSION ${PROJECT_VERSION})
TARGET_LINK_LIBRARIES(client engine ${SFML_LIBRARIES} ${GLEW_LIBRARIES})

ADD_EXECUTABLE(server src/server.cpp)
SET_TARGET_PROPERTIES(server PROPERTIES VERSION ${PROJECT_VERSION})
TARGET_LINK_LIBRARIES(server engine ${SFML_LIBRARIES})

ADD_EXECUTABLE(testmain test/testmain.cpp)
TARGET_LINK_LIBRARIES(testmain engine)
SET_TARGET_PROPERTIES(testmain PROPERTIES OUTPUT_NAME test)

################################################################################
##  EOF
################################################################################

