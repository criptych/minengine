################################################################################
##
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(minengine VERSION 0.1 LANGUAGES CXX)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
ENDIF()

################################################################################

FIND_PACKAGE(SFML 2.2 REQUIRED COMPONENTS system window graphics audio network)
ADD_SUBDIRECTORY(lib/glew-1.13.0 EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(lib/luajit EXCLUDE_FROM_ALL)

################################################################################

SET(ENGINE_SRCS
    src/engine/engine.cpp
    src/engine/engine.hpp
    src/engine/entity.cpp
    src/engine/entity.hpp
    src/engine/math.cpp
    src/engine/math.hpp
    src/engine/model.cpp
    src/engine/model.hpp
    src/engine/network.cpp
    src/engine/network.hpp
    src/engine/physics.cpp
    src/engine/physics.hpp
    src/engine/types.cpp
    src/engine/types.hpp
    src/engine/world.cpp
    src/engine/world.hpp
)

SET(CLIENT_SRCS
    src/client/client.cpp
    src/client/Camera.cpp
    src/client/Camera.hpp
    src/client/ClientModel.cpp
    src/client/ClientModel.hpp
    src/client/ClientObject.cpp
    src/client/ClientObject.hpp
    src/client/ResourceCache.cpp
    src/client/ResourceCache.hpp
    src/client/Transformable3D.cpp
    src/client/Transformable3D.hpp
)

SET(SERVER_SRCS
    src/server/server.cpp
)

SET(TEST_SRCS
    test/catch.hpp
    test/testmain.cpp
    test/test_physics.cpp
)

################################################################################

ADD_DEFINITIONS(-DGLEW_STATIC=1)
INCLUDE_DIRECTORIES(src ${GLEW_INCLUDE_DIR} ${SFML_INCLUDE_DIR})
INCLUDE_DIRECTORIES(lib/luajit/src ${CMAKE_CURRENT_BINARY_DIR}/lib/luajit)

ADD_LIBRARY(engine ${ENGINE_SRCS})
SET_TARGET_PROPERTIES(engine PROPERTIES VERSION ${PROJECT_VERSION})
TARGET_LINK_LIBRARIES(engine ${SFML_LIBRARIES} liblua)

ADD_EXECUTABLE(client ${CLIENT_SRCS})
SET_TARGET_PROPERTIES(client PROPERTIES VERSION ${PROJECT_VERSION})
TARGET_LINK_LIBRARIES(client engine ${SFML_LIBRARIES} ${GLEW_LIBRARIES})
ADD_CUSTOM_TARGET(run-client COMMAND client DEPENDS client)

ADD_EXECUTABLE(server ${SERVER_SRCS})
SET_TARGET_PROPERTIES(server PROPERTIES VERSION ${PROJECT_VERSION})
TARGET_LINK_LIBRARIES(server engine ${SFML_LIBRARIES})
ADD_CUSTOM_TARGET(run-server COMMAND server DEPENDS server)

################################################################################

ADD_EXECUTABLE(testmain ${TEST_SRCS})
TARGET_LINK_LIBRARIES(testmain engine ${SFML_LIBRARIES})
SET_TARGET_PROPERTIES(testmain PROPERTIES OUTPUT_NAME test)
ADD_TEST(testmain testmain)

################################################################################
##  EOF
################################################################################

