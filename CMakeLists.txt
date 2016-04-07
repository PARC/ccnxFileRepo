cmake_minimum_required(VERSION 3.2)
project(ccnxFileRepo)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if( UNIX )
    link_libraries(m)
endif( UNIX )

include( CTest )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_NOPANTS "${CMAKE_C_FLAGS_NOPANTS} -O3 -DNDEBUG")

link_directories($ENV{CCNX_DEPENDENCIES}/lib)
include_directories($ENV{CCNX_DEPENDENCIES}/include)
set(OPENSSL_ROOT_DIR $ENV{CCNX_DEPENDENCIES})

find_package( LongBow REQUIRED )
include_directories(${LONGBOW_INCLUDE_DIRS})

find_package( LibEvent REQUIRED )
include_directories(${LIBEVENT_INCLUDE_DIRS})

find_package( Libparc REQUIRED )
include_directories(${LIBPARC_INCLUDE_DIRS})

find_package( CCNX_Common REQUIRED )
include_directories(${CCNX_COMMON_INCLUDE_DIRS})

find_package( CCNX_Transport_Rta REQUIRED )
include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

find_package( CCNX_Portal REQUIRED )
include_directories(${CCNX_PORTAL_INCLUDE_DIRS})

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH false)

# when building, use the install RPATH already
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

# Build libraries location
#set(CMAKE_INSTALL_RPATH "${CCNX_HOME}/lib")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

set(REPO_LIBRARIES
    ${LONGBOW_LIBRARIES}
    ${LIBEVENT_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CCNX_PORTAL_LIBRARIES}
    ${CCNX_TRANSPORT_RTA_LIBRARIES}
    ${CCNX_COMMON_LIBRARIES}
    ${LIBPARC_LIBRARIES}
    -ldl)

add_executable(ccnxFileRepo_Server
               ccnxFileRepo_Server.c
               ccnxFileRepo_Common.c
               ccnxFileRepo_ManifestBuilder.c
               ccnxFileRepo_Cache.c)

add_executable(ccnxFileRepo_Client
               ccnxFileRepo_Client.c
               ccnxFileRepo_ManifestFetcher.c
               ccnxFileRepo_Common.c)

target_link_libraries(ccnxFileRepo_Client ${REPO_LIBRARIES})
target_link_libraries(ccnxFileRepo_Server ${REPO_LIBRARIES})

install(TARGETS ccnxFileRepo_Client RUNTIME DESTINATION bin)
install(TARGETS ccnxFileRepo_Server RUNTIME DESTINATION bin)

add_test(EmptyTest, echo "OK")
