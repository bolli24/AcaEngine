add_compile_definitions(RESOURCE_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_executable(test_meshdata_load test_meshdata_load.cpp)
set_target_properties(test_meshdata_load PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
)
target_link_libraries(test_meshdata_load PRIVATE AcaEngine)
add_test(meshdata_load test_meshdata_load)

add_executable(test_octree test_octree.cpp)
set_target_properties(test_octree PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
)
target_link_libraries(test_octree PRIVATE AcaEngine)
add_test(octree test_octree)

add_executable(test_slotmap test_slotmap.cpp)
set_target_properties(test_slotmap PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
)
target_link_libraries(test_slotmap PRIVATE AcaEngine)
add_test(slotmap test_slotmap)

add_executable(test_registry test_registry.cpp)
set_target_properties(test_registry PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
)
target_link_libraries(test_registry PRIVATE AcaEngine)
add_test(registry test_registry)





