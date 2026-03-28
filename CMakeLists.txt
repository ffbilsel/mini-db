cmake_minimum_required(VERSION 3.15)

project(DB_Engine
    VERSION 1.0.0
    DESCRIPTION "A simple relational database engine"
    LANGUAGES C
)

enable_testing()

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror -Wpedantic -Wno-comment)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

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

add_library(dmap_lib 
    src/dmap.c
    include/dmap.h
)
target_include_directories(dmap_lib PUBLIC include)

add_library(buffer_pool_manager_lib 
    src/buffer_pool_manager.c
    include/buffer_pool_manager.h
)
target_include_directories(buffer_pool_manager_lib PUBLIC include)
target_link_libraries(buffer_pool_manager_lib PRIVATE disk_manager_lib)
target_link_libraries(buffer_pool_manager_lib PRIVATE page_layout_lib)
target_link_libraries(buffer_pool_manager_lib PRIVATE dmap_lib)

add_executable(test_disk_manager 
    test/test_disk_manager.c
)
target_link_libraries(test_disk_manager PRIVATE disk_manager_lib)

add_executable(test_page_layout 
    test/test_page_layout.c
)
target_link_libraries(test_page_layout PRIVATE page_layout_lib)

add_executable(test_dmap 
    test/test_dmap.c
)
target_link_libraries(test_dmap PRIVATE dmap_lib)

add_executable(test_buffer_pool_manager 
    test/test_buffer_pool_manager.c
)
target_link_libraries(test_buffer_pool_manager PRIVATE buffer_pool_manager_lib)

add_test(NAME DiskManagerTest COMMAND test_disk_manager)
add_test(NAME PageLayoutTest COMMAND test_page_layout)
add_test(NAME DMapTest COMMAND test_dmap)
add_test(NAME BufferPoolManagerTest COMMAND test_buffer_pool_manager)
