//
// Created by notbonzo on 1/31/25.
//

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

struct [[gnu::packed]] table_pointer {
    uint16_t size;
    uintptr_t offset;
};

#endif //COMMON_H
