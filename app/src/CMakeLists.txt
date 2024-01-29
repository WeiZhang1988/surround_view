set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH ${PROJECT_SOURCE_DIR}/../lib)

find_package (OpenCV REQUIRED)

# link_directories(${PROJECT_SOURCE_DIR}/../lib)

# add_executable(run_get_weight_matrices run_get_weight_matrices.cpp)
# target_link_libraries(run_get_weight_matrices PUBLIC pthread
# 																										 ${OpenCV_LIBRARIES})
# target_include_directories(run_get_weight_matrices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
# 																			 										${OpenCV_INCLUDE_DIRS}
# 																													${PROJECT_SOURCE_DIR}/../include)
# install(TARGETS run_get_weight_matrices DESTINATION ${PROJECT_SOURCE_DIR}/exe)

# ##############################################################################################
# add_executable(run_get_projection_maps run_get_projection_maps.cpp)
# target_link_libraries(run_get_projection_maps PUBLIC pthread
# 																										 ${OpenCV_LIBRARIES})
# target_include_directories(run_get_projection_maps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
# 																			 										${OpenCV_INCLUDE_DIRS}
# 																													${PROJECT_SOURCE_DIR}/../include)
# install(TARGETS run_get_projection_maps DESTINATION ${PROJECT_SOURCE_DIR}/exe)

##############################################################################################
add_executable(run_calibrate_camera run_calibrate_camera.cpp)
target_link_libraries(run_calibrate_camera PUBLIC pthread
																										 ${OpenCV_LIBRARIES})
target_include_directories(run_calibrate_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
																			 										${OpenCV_INCLUDE_DIRS}
																													${PROJECT_SOURCE_DIR}/../include)
install(TARGETS run_calibrate_camera DESTINATION ${PROJECT_SOURCE_DIR}/exe)
