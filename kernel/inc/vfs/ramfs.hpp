#pragma once

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <vector>
#include <string>
#include <util>

namespace ramfs {

namespace internal {
    struct RAMFSFile {
        const char* name;
        size_t size;
        const uint8_t* data;
        bool isDirectory;
        RAMFSFile* next;
    };

    void init();
    RAMFSFile* open(const std::string& filename);
    size_t fsize(RAMFSFile* file);
    size_t read(RAMFSFile* file, uint8_t* buffer, size_t size);
    bool dir(RAMFSFile* folder);
    void cleanup();
}

} // namespace ramfs
