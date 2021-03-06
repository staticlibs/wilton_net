# Copyright 2017, alex at staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# project
project ( wilton_net CXX )

if ( STATICLIB_TOOLCHAIN MATCHES "windows_.+" )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_asio )
elseif ( WILTON_BUILD_FLAVOUR MATCHES "wheezy" )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_asio )
endif ( )
set ( ${PROJECT_NAME}_DEPS
        staticlib_config
        staticlib_support
        staticlib_io
        staticlib_ranges
        staticlib_utils
        staticlib_json
        staticlib_pimpl
        asio )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# library
set ( ${PROJECT_NAME}_PLATFORM_SRC ) 
set ( ${PROJECT_NAME}_RESFILE )
set ( ${PROJECT_NAME}_DEFFILE )
set ( ${PROJECT_NAME}_DEFINITIONS )
if ( STATICLIB_TOOLCHAIN MATCHES "windows_.+" )
    configure_file ( ${WILTON_DIR}/resources/buildres/wilton_module.rc
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_RESFILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_DEFFILE ${CMAKE_CURRENT_LIST_DIR}/resources/${PROJECT_NAME}.def )
    # http://think-async.com/Asio/asio-1.10.6/doc/asio/reference/basic_stream_socket/cancel/overload1.html#asio.reference.basic_stream_socket.cancel.overload1.remarks
    set ( ${PROJECT_NAME}_DEFINITIONS -DASIO_DISABLE_IOCP )
endif ( )

add_library ( ${PROJECT_NAME} SHARED
        ${CMAKE_CURRENT_LIST_DIR}/src/tcp_client_socket.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/tcp_operations.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/tcp_server_socket.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/wilton_net.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/wilton_socket.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/wiltoncall_net.cpp
        ${${PROJECT_NAME}_RESFILE}
        ${${PROJECT_NAME}_DEFFILE} )
        
target_link_libraries ( ${PROJECT_NAME} PRIVATE
        wilton_core
        wilton_logging
        ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )

target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${WILTON_DIR}/core/include
        ${WILTON_DIR}/modules/wilton_logging/include
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
        
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )

target_compile_definitions( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS} )

if ( STATICLIB_TOOLCHAIN MATCHES "android_.+" )
    set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-soname,lib${PROJECT_NAME}.so" )
elseif ( STATICLIB_TOOLCHAIN MATCHES "windows_.+" )
    target_link_libraries ( ${PROJECT_NAME} PRIVATE wtsapi32 )
    set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "/manifest:no" )
endif ( )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS )
configure_file ( ${WILTON_DIR}/resources/buildres/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )
