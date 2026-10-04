cmake_minimum_required(VERSION 3.15)
project(minesync C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(WIN32)
    # Prevent libcurl from trying to link missing wkscli library in MinGW
    set(HAVE_LIBWKSCLI OFF CACHE BOOL "" FORCE)
    set(HAVE_WKSCLI_H OFF CACHE BOOL "" FORCE)
    set(CURL_DISABLE_NTLM ON CACHE BOOL "" FORCE)
    set(CURL_DISABLE_SMB ON CACHE BOOL "" FORCE)

    # Force Windows non-blocking sockets
    set(HAVE_IOCTLSOCKET 1 CACHE BOOL "" FORCE)
    set(HAVE_IOCTLSOCKET_FIONBIO 1 CACHE BOOL "" FORCE)

    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(ENABLE_CURL_MANUAL OFF CACHE BOOL "" FORCE)
    set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
endif()

# Disable CPR internal install targets
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CPR_INSTALL OFF CACHE BOOL "" FORCE)

# CPR Dependency
FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG        1.10.5
)
FetchContent_MakeAvailable(cpr)

# Fix static cURL dependencies on Windows
if(TARGET libcurl_static)
    get_target_property(CURL_LINK_LIBS libcurl_static INTERFACE_LINK_LIBRARIES)
    if(CURL_LINK_LIBS)
        list(REMOVE_ITEM CURL_LINK_LIBS "wkscli" "libwkscli")
        set_target_properties(libcurl_static PROPERTIES INTERFACE_LINK_LIBRARIES "${CURL_LINK_LIBS}")
    endif()
    if(WIN32)
        target_link_libraries(libcurl_static INTERFACE unistring iconv)
    endif()
endif()

# Asio Dependency (Header-only library required by Crow)
FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG        asio-1-30-2
)
FetchContent_MakeAvailable(asio)
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "Asio header directory" FORCE)

# Crow Dependency
FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(crow)

# GUI Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
set(FLTK_USE_STATIC_LIBS ON)
find_package(FLTK REQUIRED)

# ==============================================================================
#  Shared Common Library
# ==============================================================================
add_library(minesync_common STATIC
        common/lib/zip/zip.c
        common/lib/zip/zip.h
        common/zip/ZipUtils.cpp
        common/zip/ZipUtils.h
        common/files/CacheHandler.cpp
        common/files/CacheHandler.h
        common/lib/pfd/portable-file-dialogs.h
        common/files/FileHelpers.cpp
        common/files/FileHelpers.h
)

if(NOT WIN32)
    target_compile_definitions(minesync_common PRIVATE
            _GNU_SOURCE
            _LARGEFILE64_SOURCE
            _FILE_OFFSET_BITS=64
    )
endif()

target_include_directories(minesync_common PUBLIC
        common/
        common/lib/zip
)

# ==============================================================================
#  Client Target
# ==============================================================================
add_executable(minesync_client
        client/main.cpp
        client/src/ui/MainWindow.cpp
        client/src/ui/MainWindow.h
        client/src/ui/ConnectionWindow.cpp
        client/src/ui/ConnectionWindow.h
        client/src/client/Client.cpp
        client/src/client/Client.h
        client/src/api/ApiController.cpp
        client/src/api/ApiController.h
        common/data/World.cpp
        common/data/World.h
        common/files/ClientAppConfig.h
        client/src/services/WorldService.cpp
        client/src/services/WorldService.h
        client/src/presenters/MainPresenter.cpp
        client/src/presenters/MainPresenter.h
        client/src/coordinators/AppCoordinator.cpp
        client/src/coordinators/AppCoordinator.h
        client/src/presenters/ConnectionPresenter.cpp
        client/src/presenters/ConnectionPresenter.h
)

target_include_directories(minesync_client PRIVATE
        client/src
        client/config
        ${FLTK_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
)

target_link_libraries(minesync_client PRIVATE
        minesync_common
        ${FLTK_LIBRARIES}
        ${CAIRO_LIBRARIES}
        cpr::cpr
)

# ==============================================================================
#  Server Target
# ==============================================================================
add_executable(minesync_server
        server/main.cpp
)

target_include_directories(minesync_server PRIVATE
        server/src
)

target_link_libraries(minesync_server PRIVATE
        minesync_common
        Crow::Crow
)

# ==============================================================================
#  Install Rules & Packaging (CPack)
# ==============================================================================
include(GNUInstallDirs)

install(TARGETS minesync_client
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT client
)

install(TARGETS minesync_server
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT server
)

set(CPACK_PACKAGE_NAME "minesync")
set(CPACK_PACKAGE_VENDOR "MineSync")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MineSync Client and Server synchronization app")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CPACK_STRIP_FILES TRUE)
set(CPACK_GENERATOR "TGZ;TXZ")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(CPACK_COMPONENTS_ALL client server)

set(CPACK_COMPONENT_CLIENT_DISPLAY_NAME "MineSync Client GUI")
set(CPACK_COMPONENT_SERVER_DISPLAY_NAME "MineSync Server")

# ==============================================================================
#  Windows Specific System Links & Packaging Setup
# ==============================================================================
if(WIN32)
    target_link_libraries(minesync_client PRIVATE ws2_32 mswsock crypt32 gdi32 comctl32 ole32 uuid)
    target_link_libraries(minesync_server PRIVATE ws2_32 mswsock crypt32)

    target_link_options(minesync_client PRIVATE -static)
    target_link_options(minesync_server PRIVATE -static)

    set(CPACK_GENERATOR "ZIP")
endif()

if(WIN32)
    file(GLOB MINGW_CLIENT_DLLS
            "/usr/x86_64-w64-mingw32/bin/libssp-0.dll"
            "/usr/x86_64-w64-mingw32/bin/mgwfltk*.dll"
            "/usr/x86_64-w64-mingw32/bin/libcairo*.dll"
            "/usr/x86_64-w64-mingw32/bin/libpixman*.dll"
            "/usr/x86_64-w64-mingw32/bin/libpng*.dll"
            "/usr/x86_64-w64-mingw32/bin/zlib1.dll"
    )
    install(FILES ${MINGW_CLIENT_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT client)
endif()

include(CPack)