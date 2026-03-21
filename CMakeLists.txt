cmake_minimum_required(VERSION 3.15)

project(DBEngine
    VERSION 1.0.0
    DESCRIPTION "A simple database page-based I/O engine"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------------------------------------------------------
# 1. Global Compiler Flags & Sanitizers (MUST go before targets)
# ---------------------------------------------------------
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    # Strict C90 warnings
    add_compile_options(-Wall -Wextra -Werror -Wpedantic -Wno-comment)
endif()

# Sanitizers to catch memory leaks and out-of-bounds arrays
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# ---------------------------------------------------------
# 2. Libraries
# ---------------------------------------------------------
add_library(disk_manager_lib 
    src/disk_manager.c
    include/disk_manager.h
)
target_include_directories(disk_manager_lib PUBLIC include)

add_library(page_layout_lib 
    src/page_layout.c
    include/page_layout.h
)
target_include_directories(page_layout_lib PUBLIC include)
# page_layout requires disk_manager's headers (like DM_PAGE_SIZE)
target_link_libraries(page_layout_lib PRIVATE disk_manager_lib)


# ---------------------------------------------------------
# 3. Test Executables
# ---------------------------------------------------------
add_executable(disk_manager_test 
    test/disk_manager_test.c
)
target_link_libraries(disk_manager_test PRIVATE disk_manager_lib)

add_executable(page_layout_test 
    test/page_layout_test.c
)
# We only need to link page_layout_lib here; it handles its own disk_manager dependencies transitively
target_link_libraries(page_layout_test PRIVATE page_layout_lib)