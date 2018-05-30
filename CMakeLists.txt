project(DelveScript)
cmake_minimum_required(VERSION 3.0)

include(build/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set (script_sources
	token.h
	token.cpp
	lexer.h
	lexer.cpp
	ast.h
	parser.h
	parser.cpp
)

add_library(libdelvescript ${script_sources})
set_property(TARGET libdelvescript PROPERTY CXX_STANDARD 17)
set_property(TARGET libdelvescript PROPERTY CXX_STANDARD_REQUIRED ON)


set (console_sources
	console.h
	console.cpp
	main.cpp
)

add_executable(delvescript_console ${console_sources})
target_link_libraries(delvescript_console libdelvescript CONAN_PKG::gtest)
set_property(TARGET delvescript_console PROPERTY CXX_STANDARD 17)
set_property(TARGET delvescript_console PROPERTY CXX_STANDARD_REQUIRED ON)

set (test_sources
	ast_test.cpp
	token_test.cpp
	lexer_test.cpp
	parser_test.cpp
)

add_executable(delvescript_test ${test_sources})
target_link_libraries(delvescript_test libdelvescript CONAN_PKG::gtest)
set_property(TARGET delvescript_test PROPERTY CXX_STANDARD 17)
set_property(TARGET delvescript_test PROPERTY CXX_STANDARD_REQUIRED ON)
