# Тестовый исполняемый файл
add_executable(protobuf-parser-tests
    Tests.cpp
    Test_DelimitedMessagesStreamParser.cpp
    Test_Helpers.cpp
)

target_link_libraries(protobuf-parser-tests
    PRIVATE
    protobuf-messages
    GTest::GTest
    GTest::Main
    protobuf::libprotobuf
)

add_test(NAME parser_tests COMMAND protobuf-parser-tests)