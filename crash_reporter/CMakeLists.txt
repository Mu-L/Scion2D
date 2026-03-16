add_executable(SCION_CRASH_REPORTER "src/main.cpp")

target_include_directories(
    SCION_CRASH_REPORTER PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
	SCION_CRASH_REPORTER
	PRIVATE
	SDL3::SDL3
	glad::glad
	soil2
	imgui
)
