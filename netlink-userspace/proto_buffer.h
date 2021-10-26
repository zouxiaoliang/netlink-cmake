#ifndef PROTO_BUFFER_H
#define PROTO_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#define USING_BUFFER
template <uint32_t ValueLenght>
class ProtoBuffer
{
public:
    ProtoBuffer(const void *v, uint32_t vl) {
        ::bzero(value, ValueLenght);

        pid = ::getpid();
        value_length = (vl >= ValueLenght ? ValueLenght : vl);
        memcpy(value, v, value_length);
    }

    void *c_data() {
        return (void *)this;
    }

    uint32_t c_data_length() {
        return value_length + sizeof(pid) + sizeof(value_length);
    }

    uint32_t c_length() {
        return ValueLenght + sizeof(pid) + sizeof(value_length);
    }
private:
    int32_t pid;
    uint32_t value_length;
    char value[ValueLenght];
};

#endif // PROTO_BUFFER_H
