#
# Copyright (C) 2015-2018 Virgil Security Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     (1) Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#     (2) Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#     (3) Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
#

cmake_minimum_required (VERSION 3.10 FATAL_ERROR)

project (virgil_crypto VERSION ${VIRGIL_VERSION})

# Configure sources
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/src/VirgilVersion.cxx.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/VirgilVersion.cxx"
    @ONLY
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/src/VirgilConfig.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/VirgilConfig.h"
    @ONLY
)

# Define sources list
file (GLOB_RECURSE inc_src "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file (GLOB_RECURSE inc_bin "${CMAKE_CURRENT_BINARY_DIR}/include/*.h")

aux_source_directory ("src" src)
aux_source_directory ("src/internal" src)
aux_source_directory ("src/primitive" src)
aux_source_directory ("src/pfs" src)
aux_source_directory ("src/stream" src)
aux_source_directory ("src/pythia" src)

aux_source_directory("${CMAKE_CURRENT_BINARY_DIR}/src" src)

if (BUILD_LIBRARY_FRAMEWORK)
    add_library (${PROJECT_NAME} SHARED ${inc_src} ${inc_bin} ${src})
else()
    add_library (${PROJECT_NAME} ${inc_src} ${inc_bin} ${src})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    POSITION_INDEPENDENT_CODE ON
    INTERFACE_POSITION_INDEPENDENT_CODE ON
    DEBUG_POSTFIX "_d"
)

if (BUILD_SHARED_LIBS OR BUILD_LIBRARY_FRAMEWORK)
    set_target_properties (${PROJECT_NAME} PROPERTIES
        VERSION ${VIRGIL_VERSION}
        SOVERSION ${VIRGIL_SOVERSION}
    )
endif ()

target_include_directories (${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries (${PROJECT_NAME} PUBLIC mbedtls::mbedcrypto mbedtls::ed25519)

target_compile_definitions (${PROJECT_NAME}
    PUBLIC
        "VIRGIL_CRYPTO_FEATURE_STREAM_IMPL=$<BOOL:${VIRGIL_CRYPTO_FEATURE_STREAM_IMPL}>"
        "VIRGIL_CRYPTO_FEATURE_PYTHIA=$<BOOL:${VIRGIL_CRYPTO_FEATURE_PYTHIA}>"
        "UCLIBC=$<BOOL:${UCLIBC}>"
    PRIVATE
        "FMT_HEADER_ONLY"
)

if (VIRGIL_CRYPTO_FEATURE_PYTHIA)
    target_link_libraries (${PROJECT_NAME} PUBLIC pythia)

    get_target_property (include_directories pythia INTERFACE_INCLUDE_DIRECTORIES)

    foreach(include_dir ${include_directories})
        target_include_directories (${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${include_dir}/pythia>")
    endforeach(include_dir)
endif ()


# ---------------------------------------------------------------------------
#   Apple specifc library configuration
# ---------------------------------------------------------------------------
if (BUILD_LIBRARY_FRAMEWORK)
    # Add includes to the framework
    foreach (_inc_file ${inc_src} ${inc_bin})
        get_filename_component (_inc_file_path "${_inc_file}" DIRECTORY)

        string (REGEX REPLACE
            "${CMAKE_CURRENT_SOURCE_DIR}/include/virgil/crypto" ""
            _inc_file_path "${_inc_file_path}"
        )

        string (REGEX REPLACE
            "${CMAKE_CURRENT_BINARY_DIR}/include" ""
            _inc_file_path "${_inc_file_path}"
        )

        set_property (
            SOURCE ${_inc_file}
            PROPERTY MACOSX_PACKAGE_LOCATION "Headers${_inc_file_path}"
        )
    endforeach ()

    if (VIRGIL_CRYPTO_FEATURE_PYTHIA)
        find_file (pythia_buf_header_file_path
                NAMES "pythia_buf.h" PATHS ${include_directories} PATH_SUFFIXES pythia
                NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

        find_file (pythia_buf_sizes_header_file_path
                NAMES "pythia_buf_sizes.h" PATHS ${include_directories} PATH_SUFFIXES pythia
                NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

        if (NOT pythia_buf_header_file_path)
            message (FATAL_ERROR "Header file 'pythia_buf.h' is not found within paths $${include_directories}")
        endif ()

        if (NOT pythia_buf_sizes_header_file_path)
            message (FATAL_ERROR "Header file 'pythia_buf_sizes.h' is not found within paths $${include_directories}")
        endif ()

        target_sources (${PROJECT_NAME} PRIVATE "${pythia_buf_header_file_path}" "${pythia_buf_sizes_header_file_path}")

        set_property (
            SOURCE ${pythia_buf_header_file_path}
            PROPERTY MACOSX_PACKAGE_LOCATION "Headers/pythia"
        )

        set_property (
            SOURCE ${pythia_buf_sizes_header_file_path}
            PROPERTY MACOSX_PACKAGE_LOCATION "Headers/pythia"
        )


    endif ()

    # Convert target to framework
    target_apple_framework (${PROJECT_NAME}
        NAME "VSCCrypto"
        IDENTIFIER "com.virgilsecurity.VSCCrypto"
        MODULE_MAP "${CMAKE_CURRENT_LIST_DIR}/module.modulemap"
    )

endif ()

if (INSTALL_CORE_LIBS)
    # Define names for configuration files
    set (INSTALL_CFG_DIR_NAME
        "${INSTALL_LIB_DIR_NAME}/cmake/${PROJECT_NAME}" CACHE STRING
        "Path to the CMake configuration files be installed"
    )
    set (generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set (version_config "${generated_dir}/${PROJECT_NAME}-config-version.cmake")
    set (project_config "${generated_dir}/${PROJECT_NAME}-config.cmake")
    set (targets_export_name "${PROJECT_NAME}-targets")
    set (namespace "virgil::security::")

    # Create configuration files
    include (CMakePackageConfigHelpers)

    # Write Version Config
    write_basic_package_version_file (
        "${version_config}" COMPATIBILITY SameMajorVersion
    )

    # Write  Project Config
    configure_package_config_file (
        "cmake/config.cmake.in"
        "${project_config}"
        INSTALL_DESTINATION "${INSTALL_CFG_DIR_NAME}"
    )

    # Install targets
    install (TARGETS ${PROJECT_NAME}
        EXPORT "${targets_export_name}"
        LIBRARY   DESTINATION "${INSTALL_LIB_DIR_NAME}"
        ARCHIVE   DESTINATION "${INSTALL_LIB_DIR_NAME}"
        RUNTIME   DESTINATION "${INSTALL_BIN_DIR_NAME}"
        FRAMEWORK DESTINATION "${INSTALL_LIB_DIR_NAME}"
    )

    # Install headers
    if (INSTALL_CORE_HEADERS)
        install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION "${INSTALL_INC_DIR_NAME}"
        )

        install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/"
            DESTINATION "${INSTALL_INC_DIR_NAME}/virgil/crypto"
        )
    endif ()

    if (VIRGIL_CRYPTO_FEATURE_PYTHIA)
        get_target_property (PYTHIA_INCLUDE_DIRECTORIES pythia INTERFACE_INCLUDE_DIRECTORIES)
        foreach(pythia_install_dir ${PYTHIA_INCLUDE_DIRECTORIES})
            install (
                DIRECTORY "${pythia_install_dir}/pythia" DESTINATION "${INSTALL_INC_DIR_NAME}/virgil/crypto"
                FILES_MATCHING PATTERN "*buf*"
            )
        endforeach()
    endif ()

    if (INSTALL_EXT_LIBS)
        install (DIRECTORY "${VIRGIL_DEPENDS_PREFIX}/lib/" DESTINATION "${INSTALL_LIB_DIR_NAME}")
    endif (INSTALL_EXT_LIBS)

    if (INSTALL_EXT_HEADERS)
        install (DIRECTORY "${VIRGIL_DEPENDS_PREFIX}/include/" DESTINATION "${INSTALL_INC_DIR_NAME}")
    endif ()

    # Install configurations
    install (
        FILES "${project_config}" "${version_config}"
        DESTINATION "${INSTALL_CFG_DIR_NAME}"
    )

    install (
        EXPORT "${targets_export_name}"
        NAMESPACE "${namespace}"
        DESTINATION "${INSTALL_CFG_DIR_NAME}"
    )
endif ()

# add a target to generate API documentation with Doxygen
find_host_program (DOT_PROGRAM NAMES dot)
if (DOT_PROGRAM)
    set (DOT_FOUND "YES")
else ()
    set (DOT_FOUND "NO")
endif (DOT_PROGRAM)

find_host_package (Doxygen)
if (DOXYGEN_FOUND)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target (doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif (DOXYGEN_FOUND)
