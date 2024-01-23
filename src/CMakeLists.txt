set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH ${PROJECT_SOURCE_DIR}/lib)

find_package(OpenCV REQUIRED)
										
#link_directories

add_library(svs SHARED test.cpp)

target_link_libraries(svs PUBLIC pthread
																 ${OpenCV_LIBRARIES})
																	
target_include_directories(svs BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
																						 ${OpenCV_INCLUDE_DIRS})


install(FILES structures.hpp	
							utils.hpp
							param_setting.hpp
							semaphore.hpp
							camera_model.hpp
							imagebuffer.hpp
							base_thread.hpp 
							capture_thread.hpp
							process_thread.hpp
							bird_view.hpp
							simple_gui.hpp
							DESTINATION ${PROJECT_SOURCE_DIR}/include)
							
install(TARGETS svs LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/lib)
