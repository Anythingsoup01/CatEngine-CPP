cmake_minimum_required(VERSION 3.15)
project(SampleProject)
set(INCLUDEDIRS
    /home/anyth/Doge/Dev/CatEngine/CatEngine/src
    /home/anyth/Doge/Dev/CatEngine/vendor/spdlog/include
    /home/anyth/Doge/Dev/CatEngine/vendor/glm
    /home/anyth/Doge/Dev/CatEngine/vendor/entt
    /home/anyth/Doge/Dev/CatEngine/vendor/Glad/include)

set(PRECOMPILEDHEADER /home/anyth/Doge/Dev/CatEngine/CatEngine/src/cepch.h)
set(CATENGINELIB /home/anyth/Doge/Dev/CatEngine/build/CatEngine/libCatEngine.a dl)
set(COMPILEFLAGS CE_SCRIPT_COMPILATION)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -fPIC")  
add_library(CameraController SHARED "Scripts/CameraController-int.cpp")
target_include_directories(CameraController PRIVATE ${INCLUDEDIRS})
target_precompile_headers(CameraController PRIVATE ${PRECOMPILEDHEADER})
target_link_libraries(CameraController PRIVATE ${CATENGINELIB})
target_compile_definitions(CameraController PRIVATE ${COMPILEFLAGS})

add_library(Test SHARED "Scripts/Test-int.cpp")
target_include_directories(Test PRIVATE ${INCLUDEDIRS})
target_precompile_headers(Test PRIVATE ${PRECOMPILEDHEADER})
target_link_libraries(Test PRIVATE ${CATENGINELIB})
target_compile_definitions(Test PRIVATE ${COMPILEFLAGS})

add_library(PlayerController SHARED "Scripts/PlayerController-int.cpp")
target_include_directories(PlayerController PRIVATE ${INCLUDEDIRS})
target_precompile_headers(PlayerController PRIVATE ${PRECOMPILEDHEADER})
target_link_libraries(PlayerController PRIVATE ${CATENGINELIB})
target_compile_definitions(PlayerController PRIVATE ${COMPILEFLAGS})

