SET (SEARCH_HEADER_PATHS_DIRS
  "/opt/homebrew/include"
  # "${CMAKE_SOURCE_DIR}/OpenMesh/src/OpenMesh"
  # "${CMAKE_SOURCE_DIR}/libs_required/OpenMesh/src/OpenMesh"
  # "${CMAKE_SOURCE_DIR}/../OpenMesh/src/OpenMesh"
  # "${OPENMESH_LIBRARY_DIR}"
)
SET (SEARCH_LIB_PATHS_DIRS
  "/opt/homebrew/Cellar/open-mesh/9.0/lib"
)

FIND_PATH (OPENMESH_INCLUDE_DIR OpenMesh/Core/Mesh/PolyMeshT.hh
  PATHS ${SEARCH_HEADER_PATHS_DIRS}
  PATH_SUFFIXES include)

FIND_LIBRARY(OPENMESH_CORE_LIBRARY_RELEASE NAMES OpenMeshCore
  PATHS ${SEARCH_LIB_PATHS_DIRS}
  PATH_SUFFIXES lib lib64)

FIND_LIBRARY(OPENMESH_TOOLS_LIBRARY_RELEASE NAMES OpenMeshTools
  PATHS ${SEARCH_LIB_PATHS_DIRS}
  PATH_SUFFIXES lib lib64)


#select configuration depending on platform (optimized... on windows)
include(SelectLibraryConfigurations)
select_library_configurations( OPENMESH_TOOLS )
select_library_configurations( OPENMESH_CORE )

set(OPENMESH_LIBRARIES ${OPENMESH_CORE_LIBRARY} ${OPENMESH_TOOLS_LIBRARY} )
set(OPENMESH_INCLUDE_DIRS ${OPENMESH_INCLUDE_DIR} )

#checks, if OPENMESH was found and sets OPENMESH_FOUND if so
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenMesh  DEFAULT_MSG
                                  OPENMESH_CORE_LIBRARY OPENMESH_TOOLS_LIBRARY OPENMESH_INCLUDE_DIR)

#sets the library dir 
# if ( OPENMESH_CORE_LIBRARY_RELEASE )
#   get_filename_component(_OPENMESH_LIBRARY_DIR ${OPENMESH_CORE_LIBRARY_RELEASE} PATH)
# else( OPENMESH_CORE_LIBRARY_RELEASE )
#   get_filename_component(_OPENMESH_LIBRARY_DIR ${OPENMESH_CORE_LIBRARY_DEBUG} PATH)
# endif( OPENMESH_CORE_LIBRARY_RELEASE )
# set (OPENMESH_LIBRARY_DIR "${_OPENMESH_LIBRARY_DIR}" CACHE PATH "The directory where the OpenMesh libraries can be found.")


# mark_as_advanced(OPENMESH_INCLUDE_DIR OPENMESH_CORE_LIBRARY_RELEASE OPENMESH_CORE_LIBRARY_DEBUG OPENMESH_TOOLS_LIBRARY_RELEASE OPENMESH_TOOLS_LIBRARY_DEBUG OPENMESH_LIBRARY_DIR)