# Генерация protobuf файлов
set(PROTO_FILE "Messages.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

add_library(protobuf-messages STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(protobuf-messages PRIVATE protobuf::libprotobuf)

set(PROTO_SRCS ${PROTO_SRCS} PARENT_SCOPE)
set(PROTO_HDRS ${PROTO_HDRS} PARENT_SCOPE)