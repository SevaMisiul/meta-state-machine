cmake_minimum_required(VERSION 3.21)

project(meta_state_machine_tests)

include(FetchContent)
FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG 		 v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

add_executable(${PROJECT_NAME} test.cpp)

gtest_discover_tests(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC meta_state_machine GTest::gtest_main gmock)

add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ctest)
