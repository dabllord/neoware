cmake_minimum_required(VERSION 3.5)

project(neoware)

set(LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(FONTS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/fonts)
set(KITTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/kitty_memory)
set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/images)

file(GLOB KITTY_SRC ${KITTY_PATH}/*.cpp)
file(GLOB IMGUI_SRC ${IMGUI_PATH}/*.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -w -Wno-unused-command-line-argument")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -w -Wno-unused-command-line-argument")

add_library(neoware SHARED main.cpp post_render.cpp settings.cpp shoot_inner.cpp swappy.cpp gui.cpp basic_sdk.cpp ${KITTY_SRC} ${IMGUI_SRC})

target_include_directories(neoware PUBLIC ${LOCAL_PATH})
target_link_libraries(neoware -llog -landroid -lEGL -lGLESv3)