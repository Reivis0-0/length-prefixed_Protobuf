#include "protobuf-parser/helpers.hpp"
#include "protobuf-parser/DelimitedMessagesStreamParser.hpp"

#include <gtest/gtest.h>

TEST(MyTestParser, FullMsg)
{
    //Create FullMsg
    TestTask::Messages::WrapperMessage m;
    TestTask::Messages::FastResponse* fast = m.mutable_fast_response();
    std::string s = "19851019T050107.333";
    fast->set_current_date_time(s);

    TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
    slow->set_connected_client_count(209);

    TestTask::Messages::RequestForSlowResponse* slow_request = m.mutable_request_for_slow_response();
    slow_request->set_time_in_seconds_to_sleep(10);

    TestTask::Messages::RequestForFastResponse* fast_request = m.mutable_request_for_fast_response();

    //FullMsg to string
    std::string data_str;
    auto data = serializeDelimited(m);
    data_str.append(data->begin(), data->end());

    //Parsing
    typedef DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> Parser;
        Parser parser;
    std::list<std::shared_ptr<const TestTask::Messages::WrapperMessage>> parsedMessages = parser.parse(data_str);
    for (auto msg : parsedMessages) {
        ASSERT_TRUE(msg->has_request_for_fast_response());
        ASSERT_TRUE(msg->has_fast_response());
        ASSERT_TRUE(msg->has_slow_response());
        ASSERT_TRUE(msg->has_request_for_slow_response());
        ASSERT_TRUE(msg->fast_response().has_current_date_time());
        ASSERT_TRUE(msg->slow_response().has_connected_client_count());
        ASSERT_TRUE(msg->request_for_slow_response().has_time_in_seconds_to_sleep());
    }
    EXPECT_EQ(parsedMessages.size(), 1);
}