cmake_minimum_required(VERSION 3.15)

project(DB_Engine
    VERSION 1.0.0
    DESCRIPTION "A simple relational database engine"
    LANGUAGES C
)

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
target_link_libraries(page_layout_lib PRIVATE disk_manager_lib)

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

add_executable(disk_manager_test 
    test/disk_manager_test.c
)
target_link_libraries(disk_manager_test PRIVATE disk_manager_lib)

add_executable(page_layout_test 
    test/page_layout_test.c
)
target_link_libraries(page_layout_test PRIVATE page_layout_lib)

# add_executable(dmap_test 
#     test/dmap_test.c
# )
# target_link_libraries(dmap_test PRIVATE dmap_lib)

# add_executable(buffer_pool_manager_test 
#     test/buffer_pool_manager_test.c
# )
# target_link_libraries(buffer_pool_manager_test PRIVATE buffer_pool_manager_lib)