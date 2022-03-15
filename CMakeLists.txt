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


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

if(WIN32)
  include_directories(${HYDRA_API_ROOT_DIR}/dependencies/include)
  link_directories(${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_win)
else()
  find_package(glfw3 REQUIRED)
  include_directories(${GLFW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${GLFW_LIBRARIES})
  find_package (Threads REQUIRED)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(OpenGL REQUIRED)
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})

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

find_library(HYDRA_API_LIB hydra_api HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_IES_PARSER_LIB ies_parser HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE mikktspace HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO corto HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CLEW clew HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)


if(WIN32)
  target_link_libraries(scene-converter PUBLIC
          ${HYDRA_API_LIB}
          ${HYDRA_API_IES_PARSER_LIB} ${HYDRA_API_MIKKTSPACE} ${HYDRA_API_CORTO} ${HYDRA_API_CLEW}
          ${OPENGL_gl_LIBRARY} ${LIBS} glfw3dll)
else()
  target_link_libraries(scene-converter LINK_PUBLIC
          ${HYDRA_API_LIB}
          ${HYDRA_API_IES_PARSER_LIB} ${HYDRA_API_MIKKTSPACE} ${HYDRA_API_CORTO}
          ${LIBS}
          Threads::Threads rt dl OpenCL glfw)
endif()


