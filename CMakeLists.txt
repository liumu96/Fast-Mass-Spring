cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(fast-mass-spring)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(Sources 
    ClothSimulation/main.cpp
    # ClothSimulation/MassSpringSolver.cpp
    # ClothSimulation/Mesh.cpp
    ClothSimulation/Renderer.cpp
    ClothSimulation/Shader.cpp
    # ClothSimulation/UserInteraction.cpp
)

INCLUDE_DIRECTORIES(/System/Library/Frameworks)
# find OpenGL, GLUT, GLEW
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLEW REQUIRED)

# find glm
set(glm_DIR /opt/homebrew/Cellar/glm/0.9.9.8/lib/cmake/glm) # if necessary
find_package(glm REQUIRED)

set(glm_DIR /opt/homebrew/Cellar/eigen/3.4.0_1/share/eigen3/cmake)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# set(OpenMesh_DIR /opt/homebrew/Cellar/open-mesh/9.0/lib) # if necessary
find_package(OpenMesh REQUIRED)

include_directories( /opt/homebrew/include)

# include_directories(
#     ${OPENGL_INCLUDE_DIRS} 
#     ${GLFW3_INCLUDE_DIR} 
#     ${OPENMESH_INCLUDE_DIR}
#     glew
#     glm
#     eigen3
#     )

add_executable(fast-mass-spring ${Sources})

add_library(GLAD "ClothSimulation/glad.c")

target_link_libraries(fast-mass-spring 
    ${OPENGL_LIBRARIES} 
    ${GLFW3_LIBRARY} 
    ${GLEW_LIBRARY} 
    ${OPENMESH_CORE_LIBRARY}
    GLAD
    glm::glm
    Eigen3::Eigen)