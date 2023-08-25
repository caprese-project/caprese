cmake_minimum_required(VERSION 3.20)

add_executable(fs)

target_sources(fs PRIVATE src/main.cpp)

target_compile_features(fs PRIVATE cxx_std_23)
target_compile_options(fs PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(fs PRIVATE ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(fs PRIVATE runtime)
target_link_options(fs PRIVATE -nostartfiles)
