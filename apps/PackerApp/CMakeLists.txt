add_executable(packer
    src/main.cpp
)

target_link_libraries(packer PRIVATE RepackCore)

