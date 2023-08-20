cmake_minimum_required(VERSION 3.20)

add_executable(plic)

target_sources(plic PRIVATE src/main.cpp)

target_compile_features(plic PRIVATE cxx_std_23)
target_compile_options(plic PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(plic PRIVATE ${SAMPLE_ROOT_DIR}/lib/include)

target_link_libraries(plic PRIVATE lib)
