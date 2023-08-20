cmake_minimum_required(VERSION 3.20)

add_executable(mm)

target_sources(mm PRIVATE src/main.cpp)

target_compile_features(mm PRIVATE cxx_std_23)
target_compile_options(mm PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(mm PRIVATE ${SAMPLE_ROOT_DIR}/lib/include)

target_link_libraries(mm PRIVATE lib)
