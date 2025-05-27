cmake_minimum_required(VERSION 3.5)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

project(cam_eCAL)

find_package(OpenCV REQUIRED)
find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# eCAL / proto
set(protobuf_files
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/webcam.proto
)

add_executable(${PROJECT_NAME} main.cpp)

# compile protos
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/proto/ ${protobuf_files})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} eCAL::core protobuf::libprotobuf)

