add_library(RepackCore STATIC
    src/RepackCore.cpp
    src/RepackCoreCompression.cpp
)

target_include_directories(RepackCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find zstd and liblzma via pkg-config
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(ZSTD IMPORTED_TARGET libzstd)
    pkg_check_modules(LZMA IMPORTED_TARGET liblzma)
    if (ZSTD_FOUND)
        target_link_libraries(RepackCore PUBLIC PkgConfig::ZSTD)
    else()
        message(WARNING "zstd not found via pkg-config; compression will fall back to store")
    endif()
    if (LZMA_FOUND)
        target_link_libraries(RepackCore PUBLIC PkgConfig::LZMA)
    else()
        message(WARNING "liblzma not found via pkg-config; compression will fall back to store")
    endif()
else()
    message(WARNING "pkg-config not found; zstd/lzma2 compression will not be linked")
endif()

if (WIN32)
    target_compile_definitions(RepackCore PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

