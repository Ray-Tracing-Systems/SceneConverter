cmake_minimum_required(VERSION 3.7)
project(scene-converter)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(HYDRA_API_ROOT_DIR ${CMAKE_SOURCE_DIR}/../HydraAPI CACHE PATH "Path to HydraAPI") #
set(HYDRA_API_SRC_DIR ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR ${HYDRA_API_ROOT_DIR}/bin)
set(HYDRA_API_UTILS_SRC_DIR ${HYDRA_API_ROOT_DIR}/utils)

set(USE_GL OFF CACHE BOOL "HydraAPI with OpenGL support")

set(ADDITIONAL_INCLUDE_DIRS
        ${HYDRA_API_ROOT_DIR}/dependencies/include CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
if(WIN32)
  set(ADDITIONAL_LIBRARY_DIRS
          ${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_win CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
else()
  set(ADDITIONAL_LIBRARY_DIRS
          ${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_linux CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
endif()


include_directories(${ADDITIONAL_INCLUDE_DIRS})
link_directories(${ADDITIONAL_LIBRARY_DIRS})

find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(USE_GL)
    find_package(OpenGL REQUIRED)
endif()

set(SOURCE_FILES
        convert_common.cpp
        convert_gltf.cpp
        convert_obj.cpp
        image_loader.cpp
        gltf_utils.cpp
        main.cpp)

add_executable(scene-converter ${SOURCE_FILES})

include_directories(${HYDRA_API_SRC_DIR} ${HYDRA_API_UTILS_SRC_DIR})
include_directories("external")
include_directories("external/stb_image")

find_library(HYDRA_API_LIB_DEBUG            hydra_api  HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_IES_PARSER_LIB_DEBUG ies_parser HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE_DEBUG     mikktspace HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO_DEBUG          corto      HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)


find_library(HYDRA_API_LIB_RELEASE            hydra_api  HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_IES_PARSER_LIB_RELEASE ies_parser HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE_RELEASE     mikktspace HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO_RELEASE          corto      HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)

if(WIN32)
    find_library(HYDRA_API_CLEW_DEBUG   clew HINTS ${HYDRA_API_LIB_DIR}/Debug   ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_CLEW_RELEASE clew HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
else()
    find_package(OpenCL REQUIRED)
endif()

target_link_libraries(scene-converter LINK_PUBLIC debug     
	${HYDRA_API_LIB_DEBUG}
	${HYDRA_API_IES_PARSER_LIB_DEBUG}
	${HYDRA_API_MIKKTSPACE_DEBUG}
	${HYDRA_API_CORTO_DEBUG})
target_link_libraries(scene-converter LINK_PUBLIC optimized 
	${HYDRA_API_LIB_RELEASE}
	${HYDRA_API_IES_PARSER_LIB_RELEASE}
	${HYDRA_API_MIKKTSPACE_RELEASE}
	${HYDRA_API_CORTO_RELEASE})
target_link_libraries(scene-converter LINK_PUBLIC freeimage)
          
if(WIN32) 
  if(USE_GL)
  	target_link_libraries(scene-converter LINK_PUBLIC debug ${HYDRA_API_CLEW_DEBUG})
	target_link_libraries(scene-converter LINK_PUBLIC optimized ${HYDRA_API_CLEW_RELEASE})
    target_link_libraries(scene-converter LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw3dll)
  endif()
else()
  target_link_libraries(scene-converter LINK_PUBLIC Threads::Threads rt dl OpenCL::OpenCL)
  if(USE_GL)
    target_link_libraries(scene-converter LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw)
  endif()
endif()


