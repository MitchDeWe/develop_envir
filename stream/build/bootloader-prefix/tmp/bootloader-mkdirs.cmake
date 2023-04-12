# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/opt/esp/idf/components/bootloader/subproject"
  "/workspaces/stream/build/bootloader"
  "/workspaces/stream/build/bootloader-prefix"
  "/workspaces/stream/build/bootloader-prefix/tmp"
  "/workspaces/stream/build/bootloader-prefix/src/bootloader-stamp"
  "/workspaces/stream/build/bootloader-prefix/src"
  "/workspaces/stream/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/stream/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
