################################################################################
##
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(GLEW C)
SET(GLEW_VERSION 1.13.0)

FIND_PACKAGE(OpenGL REQUIRED)

SET(GLEW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE PATH "")
SET(GLEW_LIBRARY glew-${GLEW_VERSION} CACHE STRING "")
SET(GLEW_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} CACHE PATH "")

INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
ADD_DEFINITIONS(-DGLEW_STATIC=1)
ADD_LIBRARY(${GLEW_LIBRARY} STATIC src/glew.c)

################################################################################
##  EOF
################################################################################

