include(FetchContent)

# 1. Download Criterion from GitHub
FetchContent_Declare(
    criterion
    GIT_REPOSITORY https://github.com/Snaipe/Criterion.git
    GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(criterion)

add_executable(test_main test_s3.c test_helper.c)
target_link_libraries(test_main PRIVATE s3 criterion)

add_test(NAME UnitTests COMMAND test_main)