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
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
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
    src/client/Camera.cpp
    src/client/Camera.hpp
    src/client/ClientModel.cpp
    src/client/ClientModel.hpp
    src/client/ClientObject.cpp
    src/client/ClientObject.hpp
    src/client/LuaLight.cpp
    src/client/LuaObject.cpp
    src/client/ResourceCache.cpp
    src/client/ResourceCache.hpp
    src/client/ShaderCache.cpp
    src/client/ShaderCache.hpp
    src/client/TextureCache.cpp
    src/client/TextureCache.hpp
    src/client/MusicCache.cpp
    src/client/MusicCache.hpp
    src/client/Transformable3D.cpp
    src/client/Transformable3D.hpp
    src/client/client.cpp
)

SET(SHADERS
    data/shaders/default.330
)

SET(SERVER_SRCS
    src/server/server.cpp
)

SET(TEST_SRCS
    test/catch.hpp
    test/test_physics.cpp
    test/testmain.cpp
)

################################################################################

ADD_DEFINITIONS(-DGLEW_STATIC=1)
INCLUDE_DIRECTORIES(lib/luajit/src ${CMAKE_CURRENT_BINARY_DIR}/lib/luajit)
INCLUDE_DIRECTORIES(src lib/glm ${GLEW_INCLUDE_DIR} ${SFML_INCLUDE_DIR})

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

IF(WIN32)
    SET(GLSLANG_URL "https://cvs.khronos.org/svn/repos/ogl/trunk/ecosystem/public/sdk/tools/glslang/Install/Windows")
    SET(GLSLANG_FILE "glslangValidator.exe")
ELSE()
    SET(GLSLANG_URL "https://cvs.khronos.org/svn/repos/ogl/trunk/ecosystem/public/sdk/tools/glslang/Install/Linux")
    SET(GLSLANG_FILE "glslangValidator")
ENDIF()

SET(GLSLANG_TOOL "${CMAKE_CURRENT_BINARY_DIR}/tools/${GLSLANG_FILE}")

IF(NOT EXISTS "${GLSLANG_TOOL}")
    FILE(DOWNLOAD "${GLSLANG_URL}/${GLSLANG_FILE}" "${GLSLANG_TOOL}" SHOW_PROGRESS)
ENDIF()

ADD_CUSTOM_TARGET(check-shaders)

ADD_DEPENDENCIES(run-client check-shaders)

FOREACH(SHADER ${SHADERS})
    SET(VERT "${SHADER}.vert")
    SET(FRAG "${SHADER}.frag")

    IF(EXISTS ${VERT} AND EXISTS ${FRAG})
        ADD_CUSTOM_COMMAND(
            TARGET check-shaders
            COMMAND "${GLSLANG_TOOL}" ${VERT} ${FRAG}
            DEPENDS ${VERT} ${FRAG}
        )
    ELSE()
        MESSAGE(WARNING "Shader component(s) missing for: ${SHADER}")
    ENDIF()
ENDFOREACH()

################################################################################
##  EOF
################################################################################

