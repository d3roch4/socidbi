cmake_minimum_required(VERSION 2.8.10)

project(soci_dbi)

file(GLOB CPP_FILES src/*.cpp)

set(INCLUDES_DIR    ${PROJECT_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS}
                    ${CMAKE_INSTALL_PREFIX}/include)

include_directories(${INCLUDES_DIR})
link_directories(${LINK_DIRECTORIES}
                    ${CMAKE_INSTALL_PREFIX}/lib
                    ${CMAKE_INSTALL_PREFIX}/lib64 )
                    
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(${PROJECT_NAME} SHARED ${CPP_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_dependencies(${PROJECT_NAME} jsoncpp_)

target_link_libraries(${PROJECT_NAME} jsoncpp soci_core ${Boost_LIBRARIES})

install ( DIRECTORY src/
  DESTINATION include/
  FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

# Expose B's public includes (including Boost transitively) to other
# subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${INCLUDES_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBRARY_DIRS
                    ${CMAKE_INSTALL_PREFIX}/lib
                    ${CMAKE_INSTALL_PREFIX}/lib64
    CACHE INTERNAL "${PROJECT_NAME}: Library Directories" FORCE)
  
  
include(ExternalProject)
ExternalProject_Add (
    soci_
    URL  "https://github.com/SOCI/soci/archive/master.zip"
    CMAKE_ARGS 
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DSOCI_CORE_TARGET_OUTPUT_NAME=soci_core"
        "-DSOCI_TESTS=OFF"
        "-DSOCI_STATIC=OFF")
set_target_properties(soci_ PROPERTIES POSITION_INDEPENDENT_CODE ON)    
add_dependencies(${PROJECT_NAME} soci_) 
add_dependencies(${PROJECT_NAME} jsoncpp_)
