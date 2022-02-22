
cmake_minimum_required (VERSION 3.15)

add_executable(proj "main.cpp" "orignial-issue.cpp")

#target_compile_options(proj PUBLIC
#    "/std:c++20"
#)
target_compile_options(proj PUBLIC
    "/std:c++latest"
    
            "/fsanitize=address"
)
    
