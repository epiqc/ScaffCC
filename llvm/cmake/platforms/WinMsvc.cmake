# Cross toolchain configuration for using clang-cl on non-Windows hosts to
# target MSVC.
#
# Usage:
# cmake -G Ninja
#    -DCMAKE_TOOLCHAIN_FILE=/path/to/this/file
#    -DLLVM_NATIVE_TOOLCHAIN=/path/to/llvm/installation
#    -DMSVC_BASE=/path/to/MSVC/system/libraries/and/includes
#    -DWINSDK_BASE=/path/to/windows-sdk
#    -DWINSDK_VER=windows sdk version folder name
#
# LLVM_NATIVE_TOOLCHAIN:
#   *Absolute path* to a folder containing the toolchain which will be used to
#   build.  At a minimum, this folder should have a bin directory with a
#   copy of clang-cl, clang, clang++, and lld-link, as well as a lib directory
#   containing clang's system resource directory.
#
# MSVC_BASE:
#   *Absolute path* to the folder containing MSVC headers and system libraries.
#   The layout of the folder matches that which is intalled by MSVC 2017 on
#   Windows, and should look like this:
#
# ${MSVC_BASE}
#   include
#     vector
#     stdint.h
#     etc...
#   lib
#     x64
#       libcmt.lib
#       msvcrt.lib
#       etc...
#     x86
#       libcmt.lib
#       msvcrt.lib
#       etc...
#
# For versions of MSVC < 2017, or where you have a hermetic toolchain in a
# custom format, you must use symlinks or restructure it to look like the above.
#
# WINSDK_BASE:
#   Together with WINSDK_VER, determines the location of Windows SDK headers
#   and libraries.
#
# WINSDK_VER:
#   Together with WINSDK_BASE, determines the locations of Windows SDK headers
#   and libraries.
#
# WINSDK_BASE and WINSDK_VER work together to define a folder layout that matches
# that of the Windows SDK installation on a standard Windows machine.  It should
# match the layout described below.
#
# Note that if you install Windows SDK to a windows machine and simply copy the
# files, it will already be in the correct layout.
#
# ${WINSDK_BASE}
#   Include
#     ${WINSDK_VER}
#       shared
#       ucrt
#       um
#         windows.h
#         etc...
#   Lib
#     ${WINSDK_VER}
#       ucrt
#         x64
#         x86
#           ucrt.lib
#           etc...
#       um
#         x64
#         x86
#           kernel32.lib
#           etc
#
# IMPORTANT: In order for this to work, you will need a valid copy of the Windows
# SDK and C++ STL headers and libraries on your host.  Additionally, since the
# Windows libraries and headers are not case-correct, you will need to have these
# mounted in a case-insensitive mount.  This requires one command to set up.
#
# ~/src: mkdir winsdk
# ~/src: mkdir winsdk.icase
# ~/src: ciopfs winsdk/ winsdk.icase
#
# Now copy or otherwise install your headers and libraries to the winsdk.icase folder
# and use *that* folder as the path when configuring CMake.
#
# TODO: We could also provide a CMake option -DUSE_ICASE_VFS_OVERLAY=ON/OFF that would
# make this optional.  For now, we require ciopfs.


# When configuring CMake with a toolchain file against a top-level CMakeLists.txt,
# it will actually run CMake many times, once for each small test program used to
# determine what features a compiler supports.  Unfortunately, none of these
# invocations share a CMakeCache.txt with the top-level invocation, meaning they
# won't see the value of any arguments the user passed via -D.  Since these are
# necessary to properly configure MSVC in both the top-level configuration as well as
# all feature-test invocations, we set environment variables with the values so that
# these environments get inherited by child invocations.
function(init_user_prop prop)
  if(${prop})
    set(ENV{_${prop}} "${${prop}}")
  else()
    set(${prop} "$ENV{_${prop}}" PARENT_SCOPE)
  endif()
endfunction()

# FIXME: We should support target architectures other than x64
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

init_user_prop(LLVM_NATIVE_TOOLCHAIN)
init_user_prop(MSVC_BASE)
init_user_prop(WINSDK_BASE)
init_user_prop(WINSDK_VER)

set(MSVC_INCLUDE "${MSVC_BASE}/include")
set(MSVC_LIB "${MSVC_BASE}/lib")
set(WINSDK_INCLUDE "${WINSDK_BASE}/Include/${WINSDK_VER}")
set(WINSDK_LIB "${WINSDK_BASE}/Lib/${WINSDK_VER}")

# Do some sanity checking to make sure we can find a native toolchain and
# that the Windows SDK / MSVC STL directories look kosher.
if(NOT EXISTS "${LLVM_NATIVE_TOOLCHAIN}/bin/clang-cl" OR
   NOT EXISTS "${LLVM_NATIVE_TOOLCHAIN}/bin/lld-link")
  message(SEND_ERROR
          "LLVM_NATIVE_TOOLCHAIN folder '${LLVM_NATIVE_TOOLCHAIN}' does not "
          "point to a valid directory containing bin/clang-cl and bin/lld-link "
          "binaries")
endif()

if(NOT EXISTS "${MSVC_BASE}" OR
   NOT EXISTS "${MSVC_INCLUDE}" OR
   NOT EXISTS "${MSVC_LIB}")
  message(SEND_ERROR
          "CMake variable MSVC_BASE must point to a folder containing MSVC "
          "system headers and libraries")
endif()

if(NOT EXISTS "${WINSDK_BASE}" OR
   NOT EXISTS "${WINSDK_INCLUDE}" OR
   NOT EXISTS "${WINSDK_LIB}")
  message(SEND_ERROR
          "CMake variable WINSDK_BASE and WINSDK_VER must resolve to a valid "
          "Windows SDK installation")
endif()

set(CMAKE_C_COMPILER "${LLVM_NATIVE_TOOLCHAIN}/bin/clang-cl" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${LLVM_NATIVE_TOOLCHAIN}/bin/clang-cl" CACHE FILEPATH "")
set(CMAKE_LINKER "${LLVM_NATIVE_TOOLCHAIN}/bin/lld-link" CACHE FILEPATH "")

# Even though we're cross-compiling, we need some native tools (e.g. llvm-tblgen), and those
# native tools have to be built before we can start doing the cross-build.  LLVM supports
# a CROSS_TOOLCHAIN_FLAGS_NATIVE argument which consists of a list of flags to pass to CMake
# when configuring the NATIVE portion of the cross-build.  By default we construct this so
# that it points to the tools in the same location as the native clang-cl that we're using.
list(APPEND _CTF_NATIVE_DEFAULT "-DCMAKE_ASM_COMPILER=${LLVM_NATIVE_TOOLCHAIN}/bin/clang")
list(APPEND _CTF_NATIVE_DEFAULT "-DCMAKE_C_COMPILER=${LLVM_NATIVE_TOOLCHAIN}/bin/clang")
list(APPEND _CTF_NATIVE_DEFAULT "-DCMAKE_CXX_COMPILER=${LLVM_NATIVE_TOOLCHAIN}/bin/clang++")

set(CROSS_TOOLCHAIN_FLAGS_NATIVE "${_CTF_NATIVE_DEFAULT}" CACHE STRING "")

set(COMPILE_FLAGS
    -D_CRT_SECURE_NO_WARNINGS
    -imsvc "${MSVC_INCLUDE}"
    -imsvc "${WINSDK_INCLUDE}/ucrt"
    -imsvc "${WINSDK_INCLUDE}/shared"
    -imsvc "${WINSDK_INCLUDE}/um"
    -imsvc "${WINSDK_INCLUDE}/winrt")

string(REPLACE ";" " " COMPILE_FLAGS "${COMPILE_FLAGS}")

# We need to preserve any flags that were passed in by the user. However, we
# can't append to CMAKE_C_FLAGS and friends directly, because toolchain files
# will be re-invoked on each reconfigure and therefore need to be idempotent.
# The assignments to the _INITIAL cache variables don't use FORCE, so they'll
# only be populated on the initial configure, and their values won't change
# afterward.
set(_CMAKE_C_FLAGS_INITIAL "${CMAKE_C_FLAGS}" CACHE STRING "")
set(CMAKE_C_FLAGS "${_CMAKE_C_FLAGS_INITIAL} ${COMPILE_FLAGS}" CACHE STRING "" FORCE)

set(_CMAKE_CXX_FLAGS_INITIAL "${CMAKE_CXX_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${_CMAKE_CXX_FLAGS_INITIAL} ${COMPILE_FLAGS}" CACHE STRING "" FORCE)

set(LINK_FLAGS
    # Prevent CMake from attempting to invoke mt.exe. It only recognizes the slashed form and not the dashed form.
    /manifest:no

    # FIXME: We should support target architectures other than x64.
    -libpath:"${MSVC_LIB}/x64"
    -libpath:"${WINSDK_LIB}/ucrt/x64"
    -libpath:"${WINSDK_LIB}/um/x64")

string(REPLACE ";" " " LINK_FLAGS "${LINK_FLAGS}")

# See explanation for compiler flags above for the _INITIAL variables.
set(_CMAKE_EXE_LINKER_FLAGS_INITIAL "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "${_CMAKE_EXE_LINKER_FLAGS_INITIAL} ${LINK_FLAGS}" CACHE STRING "" FORCE)

set(_CMAKE_MODULE_LINKER_FLAGS_INITIAL "${CMAKE_MODULE_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "${_CMAKE_MODULE_LINKER_FLAGS_INITIAL} ${LINK_FLAGS}" CACHE STRING "" FORCE)

set(_CMAKE_SHARED_LINKER_FLAGS_INITIAL "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "${_CMAKE_SHARED_LINKER_FLAGS_INITIAL} ${LINK_FLAGS}" CACHE STRING "" FORCE)

# CMake populates these with a bunch of unnecessary libraries, which requires
# extra case-correcting symlinks and what not. Instead, let projects explicitly
# control which libraries they require.
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)

# CMake's InstallRequiredSystemLibraries module searches for a Visual Studio
# installation in order to determine where to copy the required DLLs. This
# installation won't exist when cross-compiling, of course, so silence the
# resulting warnings about missing libraries.
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)

