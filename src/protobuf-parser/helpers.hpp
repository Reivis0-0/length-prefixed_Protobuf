#ifndef SRC_PROTOBUF_PARSER_HELPERS_H_
#define SRC_PROTOBUF_PARSER_HELPERS_H_

#include <memory>
#include <vector>
#include <exception>
#include "protocol/Messages.pb.h"

#if GOOGLE_PROTOBUF_VERSION >= 3012004
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSizeLong())
#else
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSize())
#endif
typedef std::vector<char> Data;
typedef std::shared_ptr<const Data> PointerToConstData;

template <typename Message> PointerToConstData serializeDelimited(const Message& msg) 
{
  const size_t messageSize = PROTOBUF_MESSAGE_BYTE_SIZE(msg);
  const size_t headerSize = google::protobuf::io::CodedOutputStream::VarintSize32(messageSize);
  const PointerToConstData& result = std::make_shared<Data>(headerSize + messageSize);

  google::protobuf::uint8* buffer = reinterpret_cast<google::protobuf::uint8*>(&result->begin());
  google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(messageSize, buffer);
  
  msg.SerializeWithCachedSizeToArray(buffer + headerSize);

  return result;
}

template <typename Message>
std::shared_ptr<Message> parseDelimited(const void* data, size_t size,
                                        size_t* bytesConsumed = 0)
{
  if(data == nullptr)
  {
    return nullptr;
  }

  google::protobuf::uint32 messageSize;
  google::protobuf::io::CodedInputStream input(reinterpret_cast<const google::protobuf::uint8*>(data), size);
  auto initialPOsition = input.CurrentPosition();

  if(!input.ReadVarint32(&messageSize) || messageSize > size - google::protobuf::io::CodedOutputStream::VarintSize32(messageSize))
  {
    if(bytesConsumed != nullptr)
    {
      *bytesConsumed = 0;
    }
    return nullptr;
  }

  input.PushLimit(messageSize);
  std::shared_ptr<Message> result = std::make_shared<Message>();
  
  if(!result->ParseFromCodeStream(&input))
  {
    throw std::runtime_error("Message parsing error\n");
  }
  
  input.PopLimit(messageSize);

  if(bytesConsumed != nullptr)
  {
    *bytesConsumed = input.CurrentPosition() - initialPOsition;
  }

  return results;  
}

#endif
