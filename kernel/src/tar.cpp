#include <tar>
#include <utility>
#include <kprintf>

namespace tar {

TarParser::TarParser(const uint8_t* data, size_t size) : data(data), size(size), offset(0), currentHeaderPtr(nullptr), fileSize(0) {
    currentHeaderPtr = reinterpret_cast<const Header*>(data);
    if (std::strncmp(currentHeaderPtr->magic, "ustar", 5) == 0) {
        fileSize = std::strtol(currentHeaderPtr->size, nullptr, 8);
    }
    kprintf(" -> Initialized TarParser with file: %s, size: %lu\n", currentHeaderPtr->name, fileSize);
}

bool TarParser::next() {
    if (offset == 0) {
        offset += sizeof(Header);
    } else {
        size_t currentFileTotalSize = fileSize + sizeof(Header);
        size_t padding = (512 - (fileSize % 512)) % 512;
        currentFileTotalSize += padding;
        offset += currentFileTotalSize;
    }

    if (offset >= size || std::strlen(reinterpret_cast<const char*>(data + offset)) == 0) {
        kprintf(" -> End of TAR archive reached or offset beyond size.\n");
        return false;
    }

    currentHeaderPtr = reinterpret_cast<const Header*>(data + offset);

    if (std::strncmp(currentHeaderPtr->magic, "ustar", 5) != 0) {
        kprintf(" -> Invalid TAR magic number: %s\n", currentHeaderPtr->magic);
        return false;
    }

    fileSize = std::strtol(currentHeaderPtr->size, nullptr, 8);
    return true;
}

const Header* TarParser::currentHeader() const {
    return currentHeaderPtr;
}

const uint8_t* TarParser::currentFileData() const {
    return data + offset + sizeof(Header);
}

size_t TarParser::currentFileSize() const {
    return fileSize;
}

uint8_t* TarParser::currentFileData() {
    return const_cast<uint8_t*>(data + offset + sizeof(Header));
}

} // namespace tar
