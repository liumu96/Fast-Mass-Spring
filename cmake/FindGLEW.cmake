
set(_glew_HEADER_SEARCH_DIRS
"/opt/homebrew/include"
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes"
"C:/Program Files (x86)/glew/include" 
)

set(_glew_LIB_SEARCH_DIRS
"/opt/homebrew/Cellar/glew/2.2.0_1/lib"
"/usr/lib"
"/usr/local/lib"
"${CMAKE_SOURCE_DIR}/lib"
"C:/Program Files (x86)/glew/lib-msvc110")


set( _glew_ENV_ROOT $ENV{GLEW_ROOT} )
if( NOT GLEW_ROOT AND _glew_ENV_ROOT )
	set(GLEW_ROOT ${_glew_ENV_ROOT} )
endif()


if( GLEW_ROOT )
	list( INSERT _glew_HEADER_SEARCH_DIRS 0 "${GLEW_ROOT}/include" )
	list( INSERT _glew_LIB_SEARCH_DIRS 0 "${GLEW_ROOT}/lib" )
endif()

FIND_PATH(GLEW_INCLUDE_DIR "GL/glew.h"
PATHS ${_glew_HEADER_SEARCH_DIRS} )

FIND_LIBRARY(GLEW_LIBRARY NAMES glew glew
PATHS ${_glew_LIB_SEARCH_DIRS})

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG
GLEW_LIBRARY GLEW_INCLUDE_DIR)

