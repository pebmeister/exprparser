add_library(exprrules
    expr_rules.cpp
    expr_rules.h
)
target_link_libraries(exprrules PUBLIC coreparser)
target_include_directories(exprrules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

