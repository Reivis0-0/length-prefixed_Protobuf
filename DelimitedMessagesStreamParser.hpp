#ifndef DELIMITEDMESSAGESSTREAMPARSER_HPP
#define DELIMITEDMESSAGESSTREAMPARSER_HPP

#include "parseDelimited.hpp"
#include<list>
#include<vector>
#include<string>

template<typename MessageType>
class DelimitedMessagesStreamParser
{
public:
  typedef std::shared_ptr<const MessageType> PointerToConstValue;
  std::list<PoinerToConstValue> parse(const std::string& data)
  {
    m_buffer.insert(m_buffer.end(), data.begin(), data.end());
    std::list<PointerToConstValue> messages;
    while(!m_buffer.empty())
    {
      size_t consumed = 0;
      auto  masg = parseDelimited<MessageType>(m_buffer.data(), m_bufer.size(), &consumed);
      
      if(msg && consumed > 0)
      {
        messages.push_back(std::move(msg));
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + consumed);
      }  
      else 
      {
        break;
      }
    }
    return messages;
  }

private:
  std::vector<char> m_buffer;
};

#endif