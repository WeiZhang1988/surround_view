set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH ${PROJECT_SOURCE_DIR}/../lib)

find_package (OpenCV REQUIRED)

link_directories(${PROJECT_SOURCE_DIR}/../lib)

add_executable(test test.cpp)

target_link_libraries(test PUBLIC svs
																	pthread
																	${OpenCV_LIBRARIES})

target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
																			 ${PROJECT_SOURCE_DIR}/../include
																			 ${OpenCV_INCLUDE_DIRS})

install(TARGETS test DESTINATION ${PROJECT_SOURCE_DIR}/exe)
