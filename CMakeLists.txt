cmake_minimum_required(VERSION 3.30)
project(computacao_grafica)
#set(CMAKE_FIND_DEBUG_MODE TRUE)

# C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Origin files
add_executable(${PROJECT_NAME} module1/main.cpp glad.c)

# Output file name
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "app")

# Includes directory
include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)

# Find system compiled libraries
find_library(GLFW_LIBRARY NAMES glfw)
find_library(GLEW_LIBRARY NAMES GLEW)

# Verify added libraries
if (NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW not found!")
endif()

if (NOT GLEW_LIBRARY)
    message(FATAL_ERROR "GLEW not found!")
endif()

# Link libraries found
target_link_libraries(${PROJECT_NAME}
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        ${CMAKE_DL_LIBS}
)

# Search and link OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    message(STATUS "OpenGL found: ${OPENGL_INCLUDE_DIR}")
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
else()
    message(FATAL_ERROR "OpenGL not found!")
endif()

# Define output directory
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
message(${PROJECT_SOURCE_DIR}/bin)
message(STATUS "CMake configuration completed successfully!")