#ifndef PARSEDELIMITED_HPP
#define PARSEDELIMITED_HPP

#include<memory>

template<typename Message>
std::shared_ptr<Message> parseDelimited(const void* data, size_t size, size_t* bytesConsumed = 0); 

#endif