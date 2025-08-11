#include "protobuf-parser/helpers.hpp"
#include "protobuf-parser/DelimitedMessagesStreamParser.hpp"

#include <gtest/gtest.h>

TEST(MyTestParser, FullMessage)
{
    //Create FullMessage
    TestTask::Messages::WrapperMessage m;
    TestTask::Messages::FastResponse* fast = m.mutable_fast_response();
    std::string s = "20250811T100000.00";
    fast->set_current_date_time(s);

    TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
    slow->set_connected_client_count(209);

    TestTask::Messages::RequestForSlowResponse* slow_request = m.mutable_request_for_slow_response();
    slow_request->set_time_in_seconds_to_sleep(10);

    TestTask::Messages::RequestForFastResponse* fast_request = m.mutable_request_for_fast_response();

    //FullMessage to string
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

TEST(MyTestParser, SeveralFullMessages)
{
 //Create FullMessage
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::FastResponse* fast = m.mutable_fast_response();
  std::string s = "20250810T100000.00";
  fast->set_current_date_time(s);

  TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
  slow->set_connected_client_count(9);

  TestTask::Messages::RequestForSlowResponse* slow_request = m.mutable_request_for_slow_response();
  slow_request->set_time_in_seconds_to_sleep(0);

  TestTask::Messages::RequestForFastResponse* fast_request = m.mutable_request_for_fast_response();

  //FullMessage to string
  std::string data_str;
  auto data = serializeDelimited(m);
  data_str.append(data->begin(), data->end());
  typedef DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> Parser;
  Parser parser;
  std::list<std::shared_ptr<const TestTask::Messages::WrapperMessage>> parsedMessages;
  std::list<std::shared_ptr<const TestTask::Messages::WrapperMessage>> result;
  
  //parsing 3 messages
  for(int i = 0; i < 3; ++i)
  {
    parsedMessages = parser.parse(data_str);
    for(auto msg : parsedMessages)
    {
        ASSERT_TRUE(msg->has_request_for_fast_response());
        ASSERT_TRUE(msg->has_fast_response());
        ASSERT_TRUE(msg->has_slow_response());
        ASSERT_TRUE(msg->has_request_for_slow_response());
        ASSERT_TRUE(msg->fast_response().has_current_date_time());
        ASSERT_TRUE(msg->slow_response().has_connected_client_count());
        ASSERT_TRUE(msg->request_for_slow_response().has_time_in_seconds_to_sleep());
        result.push_back(msg);
    }
  }
  EXPECT_EQ(result.size(), 3);
}

TEST(MyTestParser,MessagesWithExtraBytesBetween)
{
  //create First Full Message
  TestTask::Messages::WrapperMessage m1;
  TestTask::Messages::FastResponse* fast = m1.mutable_fast_response();
  std::string s = "20250810T100000.00";
  fast->set_current_date_time(s);

  TestTask::Messages::SlowResponse* slow = m1.mutable_slow_response();
  slow->set_connected_client_count(5);

  TestTask::Messages::RequestForSlowResponse* slow_request = m1.mutable_request_for_slow_response();
  slow_request->set_time_in_seconds_to_sleep(3);

  TestTask::Messages::RequestForFastResponse* fast_request = m1.mutable_request_for_fast_response();  
  
  //Create Second Message
  TestTask::Messages::WrapperMessage m2;
  fast = m2.mutable_fast_response();
  s = "20250810T100000.00";
  fast->set_current_date_time(s);

  slow_request = m2.mutable_request_for_slow_response();
  slow_request->set_time_in_seconds_to_sleep(1);

 fast_request = m2.mutable_request_for_fast_response();

  //Full Message to string
  std::string data_str;
  auto data = serializeDelimited(m1);
  data_str.append(data->begin(), data->end());

  //Add extra bytes between messages
  data_str.append(std::string("9qwertyuio"));

  //Second message to string
  data = serializeDelimited(m2);
  data_str.append(data->begin(), data->end());
  
  typedef DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> Parser;
  Parser parser;
  std::list<std::shared_ptr<const TestTask::Messages::WrapperMessage>> parsedMessages;

  //Parsing
  parsedMessages = parser.parse(data_str);
  for(auto msg : parsedMessages)
  {
    ASSERT_TRUE(msg->has_request_for_fast_response());
    ASSERT_TRUE(msg->has_fast_response());
    ASSERT_TRUE(msg->has_request_for_slow_response());
    ASSERT_TRUE(msg->request_for_slow_response().has_time_in_seconds_to_sleep());
  }
  EXPECT_EQ(parsedMessages.size(), 1);
}

TEST(MyTestParser, DamageMessage)
{
  //Create Full Message
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
  slow->set_connected_client_count(9);

  //Full Message to string
  std::string data_str;
  auto data = serializeDelimited(m);
  data_str.append(data->begin(), data->end()); 

  //Dammaging Message
  data_str[data_str.length() - 1] = 'a';
  data_str[data_str.length() - 2] = 'b';
  data_str[data_str.length() - 3] = 'c';

  //Parsing 
  typedef DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> Parser;
  Parser parser;
  EXPECT_THROW(parser.parse(data_str), std::runtime_error);
}

TEST(MyTestParser, ExtraBytesBeforeAndAfterMessage)
{
    //Create Full Message
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
  slow->set_connected_client_count(9);

  //Full Message to string
  std::string data_str;
  auto data = serializeDelimited(m);
  data_str.append(std::string(2, '3'));
  data_str.append(data->begin(), data->end());
  data_str.append(std::string(2, '2'));

  //Parsing
  typedef DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> Parser;
  Parser parser;
  std::list<std::shared_ptr<const TestTask::Messages::WrapperMessage>> parsedMessages;
  parsedMessages = parser.parse(data_str);
  EXPECT_EQ(parsedMessages.size(), 0);
}

TEST(MyTestParserDelimeted, FullMessage)
{
  //Create Full Message
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::FastResponse* fast = m.mutable_fast_response();
  std::string s = "20250811T100000.00";
  fast->set_current_date_time(s);

  TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
  slow->set_connected_client_count(209);

  TestTask::Messages::RequestForSlowResponse* slow_request = m.mutable_request_for_slow_response();
  slow_request->set_time_in_seconds_to_sleep(10);

  TestTask::Messages::RequestForFastResponse* fast_request = m.mutable_request_for_fast_response();  
  
  //Parsing 
  auto data = serializeDelimited(m);
  std::shared_ptr<TestTask::Messages::WrapperMessage> result;
  size_t bytesConsumed = 0;

  result = parseDelimited<TestTask::Messages::WrapperMessage>(data->data(), data->size(), &bytesConsumed);
  ASSERT_FALSE(result == nullptr);
  ASSERT_TRUE(result->has_request_for_fast_response());
  ASSERT_TRUE(result->has_fast_response());
  ASSERT_TRUE(result->has_slow_response());
  ASSERT_TRUE(result->has_request_for_slow_response());
  ASSERT_TRUE(result->fast_response().has_current_date_time());
  ASSERT_TRUE(result->slow_response().has_connected_client_count());
  ASSERT_TRUE(result->request_for_slow_response().has_time_in_seconds_to_sleep());
}

TEST(MyTestParserDelimeted, NonExistentMessage)
{
  std::string data = "\x05wrong";
  EXPECT_THROW(parseDelimited<TestTask::Messages::WrapperMessage>(data.data(), data.size()), std::runtime_error);
}

TEST(MyTestParserDelimeted, NullMessage)
{
  std::shared_ptr<TestTask::Messages::WrapperMessage> result;
  std::string data = "";
  size_t bytesConsumed = 0;

  result = parseDelimited<TestTask::Messages::WrapperMessage>(data.data(), data.size(), &bytesConsumed);
  ASSERT_TRUE(result == nullptr);
  EXPECT_EQ(bytesConsumed, 0);
}

TEST(MyTestParserDelimeted, DamageMessage)
{
  //Create Message
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::SlowResponse* slow = m.mutable_slow_response();
  slow->set_connected_client_count(9);

  auto data = serializeDelimited(m);
  std::string data_str = std::string(data->begin(), data->end());

  //Dammaging message
  data_str[data_str.length() - 2] = 'a';

  EXPECT_THROW(parseDelimited<TestTask::Messages::WrapperMessage>(data_str.data(), data_str.size()), std::runtime_error);
}

TEST(MyTestParserDelimeted, WrongLengthPrefixedByte)
{
  //Create Message
  TestTask::Messages::WrapperMessage m;
  TestTask::Messages::FastResponse* fast = m.mutable_fast_response();
  std::string s = "20250811T100000.00";
  fast->set_current_date_time(s);

  auto data = serializeDelimited(m);
  std::string data_str = std::string(data->begin(), data->end());
  data_str[0] += 10;

  //Parsing
  std::shared_ptr<TestTask::Messages::WrapperMessage> result;
  size_t bytesConsumed = 0;
  result = parseDelimited<TestTask::Messages::WrapperMessage>(data_str.data(), data_str.size(), &bytesConsumed);

  ASSERT_TRUE(result == nullptr);
  EXPECT_EQ(bytesConsumed, 0);
}
