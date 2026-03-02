
file(GLOB_RECURSE AUDIO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(SCION_SOUNDS STATIC ${AUDIO_SRC})

target_include_directories(
    SCION_SOUNDS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
	SCION_SOUNDS
	PRIVATE
	SCION_UTILITIES
	sol2::sol2
	${LUA_LIBRARIES}
)

target_compile_options(
    SCION_SOUNDS PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CXX_COMPILE_FLAGS}>)

target_precompile_headers(SCION_SOUNDS REUSE_FROM PCH)
