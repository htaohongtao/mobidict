cmake_minimum_required(VERSION 2.8.11)

project(mobidict)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (UNIX)
  option(ASAN "Enable AddressSanitizer (Linux only)" OFF)
endif()
option(AUTOTEST "Enable Automatic testing" OFF)

find_package(Qt5 COMPONENTS Concurrent Widgets Test REQUIRED)
if (NOT WIN32)
  find_package(ZLIB)
endif()

qt5_wrap_ui(UI_HEADERS mainwindow.ui)

set(SOURCES completer.cpp htmlbrowser.cpp main.cpp mainwindow.cpp
            mobidict.cpp libmobi/src/buffer.c libmobi/src/compression.c
            libmobi/src/encryption.c libmobi/src/index.c libmobi/src/memory.c
            libmobi/src/meta.c libmobi/src/parse_rawml.c libmobi/src/sha1.c
            libmobi/src/read.c libmobi/src/structure.c libmobi/tools/common.c
            libmobi/src/util.c)
include_directories("libmobi/src")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_ENCRYPTION")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_ENCRYPTION")

if (APPLE)
  set(OS_BUNDLE MACOSX_BUNDLE)
  set(RES_FILES "res/app.icns")
  set_source_files_properties("res/app.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (WIN32)
  ENABLE_LANGUAGE(RC)
  list(APPEND SOURCES libmobi/src/miniz.c)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS /DUSE_MINIZ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DUSE_MINIZ /std:c++latest")
  set(OS_BUNDLE WIN32)
  set(RES_FILES "mobidict.rc")
endif()

if (NOT CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_DEBUG_OUTPUT")
endif()

if (ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

if (AUTOTEST)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAUTOTEST")
  set(TEST_LIB "Qt5::Test")
endif()

add_executable(mobidict ${OS_BUNDLE} ${SOURCES} ${UI_HEADERS} ${RES_FILES})
target_link_libraries(mobidict Qt5::Concurrent Qt5::Widgets ${TEST_LIB} ${ZLIB_LIBRARIES})
