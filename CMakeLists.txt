# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" FILESYSTEMS_VERSION)
string(STRIP "${FILESYSTEMS_VERSION}" FILESYSTEMS_VERSION)
if(NOT FILESYSTEMS_VERSION MATCHES "^[0-9]+[.][0-9]+[.][0-9]+$")
    message(FATAL_ERROR "VERSION must contain major.minor.patch")
endif()

project(Filesystems VERSION "${FILESYSTEMS_VERSION}" LANGUAGES C CXX)
include(CTest)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FILESYSTEMS_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

# Application-owned executable and resources live in the native namespace.
# Linux may keep a minimal /usr/bin launcher and distro-specific integration,
# but neither the application nor its selected format build sources live
# under /usr/lib/filesystems. One configure-time root sets all app paths.
set(FILESYSTEMS_APPLICATION_ROOT "/Programs/Filesystems" CACHE PATH
    "Native Filesystems application directory")

# Native deployment policy owns only filesystem/module membership. Desktop
# integration has an independent authority in data/desktop-filesystems.tsv and
# must never influence which kernel modules are built, packaged or managed.
set(FILESYSTEMS_NATIVE_MANIFEST
    "${CMAKE_CURRENT_SOURCE_DIR}/data/native-filesystems.tsv")
file(STRINGS "${FILESYSTEMS_NATIVE_MANIFEST}"
    FILESYSTEMS_NATIVE_MANIFEST_LINES)
set(FILESYSTEMS_NATIVE_FILESYSTEMS)
set(FILESYSTEMS_NATIVE_MODULES)
foreach(native_line IN LISTS FILESYSTEMS_NATIVE_MANIFEST_LINES)
    string(STRIP "${native_line}" native_line)
    if(native_line STREQUAL "" OR native_line MATCHES "^#")
        continue()
    endif()

    string(REPLACE "\t" ";" native_fields "${native_line}")
    list(LENGTH native_fields native_field_count)
    if(NOT native_field_count EQUAL 2)
        message(FATAL_ERROR
            "Invalid native filesystem manifest row: ${native_line}")
    endif()

    list(GET native_fields 0 native_fs)
    list(GET native_fields 1 native_module)
    if(NOT native_fs MATCHES "^[A-Za-z0-9_.+-]+$" OR
       NOT native_module MATCHES "^[A-Za-z0-9_.+-]+$")
        message(FATAL_ERROR
            "Invalid native filesystem/module token: ${native_line}")
    endif()

    list(FIND FILESYSTEMS_NATIVE_FILESYSTEMS
        "${native_fs}" native_duplicate_index)
    if(NOT native_duplicate_index EQUAL -1)
        message(FATAL_ERROR
            "Duplicate native filesystem in manifest: ${native_fs}")
    endif()
    list(FIND FILESYSTEMS_NATIVE_MODULES
        "${native_module}" module_duplicate_index)
    if(NOT module_duplicate_index EQUAL -1)
        message(FATAL_ERROR
            "Duplicate native module in manifest: ${native_module}")
    endif()

    list(APPEND FILESYSTEMS_NATIVE_FILESYSTEMS "${native_fs}")
    list(APPEND FILESYSTEMS_NATIVE_MODULES "${native_module}")
    set("FILESYSTEMS_NATIVE_MODULE_${native_fs}" "${native_module}")
endforeach()
if(NOT FILESYSTEMS_NATIVE_FILESYSTEMS)
    message(FATAL_ERROR "Native filesystem deployment manifest is empty")
endif()

# Generate the C++ native policy directly from the same deployment manifest.
# catalog.cpp therefore cannot acquire a second, independently maintained list
# of native filesystem IDs or kernel-module names.
set(FILESYSTEMS_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${FILESYSTEMS_GENERATED_DIR}")
set(FILESYSTEMS_NATIVE_POLICY_HEADER
    "${FILESYSTEMS_GENERATED_DIR}/native_filesystems.generated.hpp")
list(LENGTH FILESYSTEMS_NATIVE_FILESYSTEMS
    FILESYSTEMS_NATIVE_FILESYSTEM_COUNT)
set(native_policy_header
    "// Generated from data/native-filesystems.tsv; do not edit.\n#pragma once\n\n#include <array>\n#include <string_view>\n\nnamespace filesystems {\n\nstruct NativeFilesystemPolicy {\n    std::string_view filesystem_id;\n    std::string_view module;\n};\n\ninline constexpr std::array<NativeFilesystemPolicy, ${FILESYSTEMS_NATIVE_FILESYSTEM_COUNT}> kNativeFilesystemPolicies = {{\n")
foreach(native_fs IN LISTS FILESYSTEMS_NATIVE_FILESYSTEMS)
    set(native_module "${FILESYSTEMS_NATIVE_MODULE_${native_fs}}")
    string(APPEND native_policy_header
        "    {\"${native_fs}\", \"${native_module}\"},\n")
endforeach()
string(APPEND native_policy_header "}};\n\n} // namespace filesystems\n")
file(WRITE "${FILESYSTEMS_NATIVE_POLICY_HEADER}" "${native_policy_header}")

set(INFILTRATR_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/infiltratr-common")
set(INFILTRATR_COMMON_EXPECTED_VERSION "1.19.46")

if(NOT EXISTS "${INFILTRATR_COMMON_DIR}/VERSION")
    message(FATAL_ERROR "Pinned Libraries submodule is required")
endif()

file(READ "${INFILTRATR_COMMON_DIR}/VERSION" INFILTRATR_COMMON_VERSION)
string(STRIP "${INFILTRATR_COMMON_VERSION}" INFILTRATR_COMMON_VERSION)
if(NOT INFILTRATR_COMMON_VERSION STREQUAL INFILTRATR_COMMON_EXPECTED_VERSION)
    message(FATAL_ERROR
        "Filesystems requires Common ${INFILTRATR_COMMON_EXPECTED_VERSION}; found ${INFILTRATR_COMMON_VERSION}")
endif()

# Git itself owns the exact Common revision through the submodule gitlink. Do
# not duplicate that SHA in CMake: compare the checked-out submodule directly
# with the commit recorded by this Filesystems revision.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
                rev-parse "HEAD:src/infiltratr-common"
        RESULT_VARIABLE COMMON_GITLINK_RESULT
        OUTPUT_VARIABLE INFILTRATR_COMMON_GITLINK
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${INFILTRATR_COMMON_DIR}" rev-parse HEAD
        RESULT_VARIABLE COMMON_CHECKOUT_RESULT
        OUTPUT_VARIABLE INFILTRATR_COMMON_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(NOT COMMON_GITLINK_RESULT EQUAL 0 OR
       NOT COMMON_CHECKOUT_RESULT EQUAL 0 OR
       NOT INFILTRATR_COMMON_COMMIT STREQUAL INFILTRATR_COMMON_GITLINK)
        message(FATAL_ERROR
            "Libraries checkout ${INFILTRATR_COMMON_COMMIT} does not match repository gitlink ${INFILTRATR_COMMON_GITLINK}")
    endif()
endif()

set(INFILTRATR_COMMON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INFILTRATR_COMMON_WARNINGS_AS_ERRORS ON CACHE BOOL "" FORCE)
add_subdirectory("${INFILTRATR_COMMON_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/infiltratr-common" EXCLUDE_FROM_ALL)

set(FILESYSTEMS_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wshadow)

function(filesystems_configure target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(FILESYSTEMS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang" OR
           CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE ${FILESYSTEMS_WARNING_FLAGS})
        if(FILESYSTEMS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# Filesystem-neutral native mechanisms. Platform-specific adapters provide I/O
# and operating-system integration; filesystem semantics must not depend on
# Linux VFS or Windows IFS objects.
add_library(infiltratr-fs-native-core STATIC
    native/core/status.c
    native/core/io.c
    native/core/byte_reader.c
    native/core/filesystem.c)
target_include_directories(infiltratr-fs-native-core
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/native/include")
target_link_libraries(infiltratr-fs-native-core
    PUBLIC InfiltratrCommon::Portable)
filesystems_configure(infiltratr-fs-native-core)

# Stable on-media identity is a platform-neutral authority. Keep it physically
# separate from Linux package/provider policy so non-Linux consumers cannot
# acquire Debian concerns through a target-level dependency.
add_library(filesystems-identity STATIC
    src/platform_support.cpp)
target_include_directories(filesystems-identity
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(filesystems-identity
    PUBLIC InfiltratrCommon::Portable)
target_compile_definitions(filesystems-identity
    PUBLIC FILESYSTEMS_VERSION="${PROJECT_VERSION}")
filesystems_configure(filesystems-identity)

# Linux catalogue materializes stable identity with Linux provider/package and
# native-deployment policy. This target is therefore intentionally Linux-aware.
add_library(filesystems-catalog STATIC
    src/catalog.cpp)
target_include_directories(filesystems-catalog
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE "${FILESYSTEMS_GENERATED_DIR}")
target_link_libraries(filesystems-catalog
    PUBLIC filesystems-identity InfiltratrCommon::Portable)
filesystems_configure(filesystems-catalog)

if(WIN32)
    # Native Windows management shell consumes stable filesystem identity only;
    # Debian package/provider policy is not part of its dependency graph.
    add_executable(filesystems WIN32
        windows/manager/main.cpp)
    target_link_libraries(filesystems
        PRIVATE filesystems-identity Comctl32)
    target_compile_definitions(filesystems
        PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
    filesystems_configure(filesystems)

    install(TARGETS filesystems
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
    pkg_check_modules(GIO2 REQUIRED IMPORTED_TARGET gio-2.0)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0>=3.22)

    add_library(infiltratr-fs-userspace STATIC
        native/platform/userspace/file_io.c)
    target_include_directories(infiltratr-fs-userspace
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/native/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/native/platform/userspace")
    target_link_libraries(infiltratr-fs-userspace
        PUBLIC infiltratr-fs-native-core InfiltratrCommon::Common)
    filesystems_configure(infiltratr-fs-userspace)

    # SCO BFS has contiguous file data and fixed 512-byte blocks. This
    # standalone shared module implements real root traversal and in-place
    # overwrites, independently of the Linux BFS kernel driver.
    add_library(filesystems-bfs-dropin SHARED
        native/filesystems/bfs/module/bfs_module.c)
    set_target_properties(filesystems-bfs-dropin PROPERTIES
        PREFIX "" OUTPUT_NAME "BFS")
    target_include_directories(filesystems-bfs-dropin PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/native/filesystems/bfs/module")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(filesystems-bfs-dropin PRIVATE
            -O2 -ffreestanding -fno-builtin -fno-stack-protector
            -fvisibility=hidden)
        if(CMAKE_C_FLAGS MATCHES "fsanitize")
            target_link_options(filesystems-bfs-dropin PRIVATE
                -Wl,-z,relro -Wl,-z,now)
        else()
            target_link_options(filesystems-bfs-dropin PRIVATE
                -nostdlib -Wl,-z,defs -Wl,-z,relro -Wl,-z,now)
        endif()
    endif()
    filesystems_configure(filesystems-bfs-dropin)
    install(FILES
        native/filesystems/bfs/module/bfs_module.c
        native/filesystems/bfs/module/bfs_module.h
        DESTINATION "${FILESYSTEMS_APPLICATION_ROOT}/Sources/bfs/module")
    install(PROGRAMS packaging/linux/bfs-module-helper.sh
        DESTINATION "${FILESYSTEMS_APPLICATION_ROOT}/Helpers"
        RENAME bfs-module-helper)

    # Portable read-only Minix V1/V2/V3, not Linux minix.ko.
    add_library(filesystems-core STATIC
        src/probe.cpp
        src/platform/linux_host_paths.cpp
        src/action_policy.cpp)
    target_include_directories(filesystems-core
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(filesystems-core
        PUBLIC filesystems-catalog PkgConfig::GLIB2)
    target_compile_definitions(filesystems-core PRIVATE
        FILESYSTEMS_APPLICATION_ROOT="${FILESYSTEMS_APPLICATION_ROOT}")
    filesystems_configure(filesystems-core)

    add_library(filesystems-actions STATIC
        src/installer.cpp
        src/removal_guard.cpp
        src/desktop_actions.cpp)
    target_include_directories(filesystems-actions
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(filesystems-actions
        PUBLIC filesystems-core PkgConfig::GIO2)
    target_compile_definitions(filesystems-actions PRIVATE
        FILESYSTEMS_APPLICATION_ROOT="${FILESYSTEMS_APPLICATION_ROOT}")
    filesystems_configure(filesystems-actions)

    add_executable(filesystems-package-removal-guard
        src/removal_guard_main.cpp)
    target_link_libraries(filesystems-package-removal-guard
        PRIVATE filesystems-actions)
    set_target_properties(filesystems-package-removal-guard PROPERTIES
        OUTPUT_NAME package-removal-guard)
    filesystems_configure(filesystems-package-removal-guard)
    install(TARGETS filesystems-package-removal-guard
        RUNTIME DESTINATION "lib/filesystems")

    add_executable(filesystems
        src/main.cpp)
    # The native program is /Programs/Filesystems/Filesystems; the lowercase
    # 'filesystems' name is reserved for the Linux /usr/bin launcher.
    set_target_properties(filesystems PROPERTIES OUTPUT_NAME "Filesystems")
    target_link_libraries(filesystems
        PRIVATE filesystems-actions InfiltratrCommon::Portable PkgConfig::GTK3)
    filesystems_configure(filesystems)

    install(TARGETS filesystems
        RUNTIME DESTINATION "${FILESYSTEMS_APPLICATION_ROOT}")
    # Debian/Mint-only entrypoint. The actual application binary stays in
    # /Programs/Filesystems; this script is not a second installation.
    configure_file(packaging/linux/filesystems-launcher.sh.in
        "${CMAKE_CURRENT_BINARY_DIR}/filesystems-launcher.sh" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/filesystems-launcher.sh"
        DESTINATION "${CMAKE_INSTALL_BINDIR}" RENAME filesystems)
    install(FILES data/org.infiltrator.Filesystems.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

    # Project-owned native Linux filesystem modules are built against the
    # running kernel at install time. The coordinator and kernel transaction
    # helper are separate installed programs so desktop policy can never leak
    # into the kernel rollback implementation.
    install(PROGRAMS packaging/linux/native-module-helper.sh
        DESTINATION "lib/filesystems"
        RENAME native-module-helper)
    install(PROGRAMS packaging/linux/kernel-module-helper.sh
        DESTINATION "lib/filesystems"
        RENAME kernel-module-helper)
    install(FILES "${FILESYSTEMS_NATIVE_MANIFEST}"
        DESTINATION "lib/filesystems"
        RENAME native-filesystems.tsv)

    # Shared kernel-safe adapter source is compiled directly into each
    # filesystem module that consumes it. It is not installed as a shared
    # runtime module and therefore cannot couple the .ko lifecycles together.
    install(DIRECTORY "native/platform/linux_kernel/"
        DESTINATION "lib/filesystems/native/platform/linux_kernel"
        FILES_MATCHING PATTERN "*.h")

    foreach(native_fs IN LISTS FILESYSTEMS_NATIVE_FILESYSTEMS)
        set(native_source_directories
            "native/filesystems/${native_fs}/linux")
        if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/native/filesystems/${native_fs}/core")
            list(APPEND native_source_directories
                "native/filesystems/${native_fs}/core")
        endif()
        install(DIRECTORY ${native_source_directories}
            DESTINATION "lib/filesystems/native/filesystems/${native_fs}"
            PATTERN "*.o" EXCLUDE
            PATTERN "*.ko" EXCLUDE
            PATTERN "*.mod" EXCLUDE
            PATTERN "*.mod.c" EXCLUDE
            PATTERN "Module.symvers" EXCLUDE
            PATTERN "modules.order" EXCLUDE)
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(filesystems-catalog-test
        tests/catalog_test.cpp)
    target_link_libraries(filesystems-catalog-test
        PRIVATE filesystems-catalog)
    filesystems_configure(filesystems-catalog-test)
    add_test(
        NAME filesystems-catalog
        COMMAND filesystems-catalog-test
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/FILESYSTEMS_MATRIX.md"
                "${FILESYSTEMS_NATIVE_MANIFEST}")

    add_executable(filesystems-native-core-test
        tests/native_core_test.c)
    target_link_libraries(filesystems-native-core-test
        PRIVATE infiltratr-fs-native-core)
    filesystems_configure(filesystems-native-core-test)
    add_test(
        NAME filesystems-native-core
        COMMAND filesystems-native-core-test)

    if(TARGET filesystems-bfs-dropin)
        add_executable(filesystems-bfs-real-mkfs-test
            tests/bfs_real_mkfs_test.c)
        target_link_libraries(filesystems-bfs-real-mkfs-test PRIVATE
            filesystems-bfs-dropin)
        filesystems_configure(filesystems-bfs-real-mkfs-test)

        add_executable(filesystems-bfs-dropin-test
            tests/bfs_dropin_module_test.c)
        target_link_libraries(filesystems-bfs-dropin-test PRIVATE
            filesystems-bfs-dropin)
        filesystems_configure(filesystems-bfs-dropin-test)
        add_test(NAME filesystems-bfs-dropin COMMAND filesystems-bfs-dropin-test)
    endif()

    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME filesystems-native-source-digest
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/native_source_digest_test.py"
                "${CMAKE_CURRENT_SOURCE_DIR}")
        add_test(NAME filesystems-amiga-resize-errors
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/amiga_resize_error_test.py"
                "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_C_COMPILER}")
        add_test(NAME filesystems-package-transactions
            COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/package_transaction_test.py"
                "$<TARGET_FILE:filesystems-package-removal-guard>"
                "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CXX_COMPILER}")
    endif()

    add_executable(filesystems-windows-catalog-test
        tests/windows_catalog_test.cpp)
    target_link_libraries(filesystems-windows-catalog-test
        PRIVATE filesystems-identity)
    filesystems_configure(filesystems-windows-catalog-test)
    add_test(
        NAME filesystems-windows-catalog
        COMMAND filesystems-windows-catalog-test)

    if(UNIX)
        add_executable(filesystems-removal-guard-test
            tests/removal_guard_test.cpp src/removal_guard.cpp)
        target_include_directories(filesystems-removal-guard-test
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        filesystems_configure(filesystems-removal-guard-test)
        add_test(NAME filesystems-removal-guard
            COMMAND filesystems-removal-guard-test)

        add_executable(filesystems-action-policy-test
            tests/action_policy_test.cpp)
        target_link_libraries(filesystems-action-policy-test
            PRIVATE filesystems-actions)
        filesystems_configure(filesystems-action-policy-test)
        add_test(
            NAME filesystems-action-policy
            COMMAND filesystems-action-policy-test)

        add_executable(filesystems-linux-host-paths-test
            tests/linux_host_paths_test.cpp)
        target_link_libraries(filesystems-linux-host-paths-test
            PRIVATE filesystems-core)
        filesystems_configure(filesystems-linux-host-paths-test)
        add_test(NAME filesystems-linux-host-paths
            COMMAND filesystems-linux-host-paths-test)

        add_executable(filesystems-desktop-actions-test
            tests/desktop_actions_test.cpp)
        target_link_libraries(filesystems-desktop-actions-test
            PRIVATE filesystems-actions)
        filesystems_configure(filesystems-desktop-actions-test)
        add_test(
            NAME filesystems-desktop-actions
            COMMAND filesystems-desktop-actions-test)
    endif()
endif()

if(UNIX)
    # Required so CPack includes native absolute /Programs paths in a
    # Linux .deb while respecting DESTDIR staging.
    set(CPACK_SET_DESTDIR ON)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "filesystems")
    set(CPACK_PACKAGE_VENDOR "Infiltrator Projects")
    set(CPACK_PACKAGE_CONTACT "Shannon Smith")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Filesystems - discover and install filesystem support on Debian")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "Filesystems-${PROJECT_VERSION}-amd64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Shannon Smith")
    set(CPACK_DEBIAN_PACKAGE_SECTION "admin")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "policykit-1, apt, dpkg, kmod, mokutil")
    include(CPack)
endif()
