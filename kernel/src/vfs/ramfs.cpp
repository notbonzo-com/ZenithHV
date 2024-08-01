#include <vfs/ramfs.hpp>
#include <limine.h>
#include <kprintf>
#include <sys/idt.hpp>
#include <util>
#include <tar>
#include <new>

static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0,
    .response = nullptr
};

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0,
    .response = nullptr,
    .internal_module_count = 0,
    .internal_modules = nullptr
};

namespace ramfs::internal {

static RAMFSFile* root = nullptr;

void init() {
    if (module_request.response == NULL || module_request.response->module_count < 1) {
        intr::kpanic(NULL, "Failed to load initial ram filesystem");
    }

    struct limine_file *initramfs = module_request.response->modules[0];

    uint64_t initramfs_size = initramfs->size;
    const uint8_t* initramfs_data = static_cast<const uint8_t*>(initramfs->address);

    tar::TarParser parser(initramfs_data, initramfs_size);
    RAMFSFile* last = nullptr;

    do {
        const tar::Header* header = parser.currentHeader();
        if (!header) break;

        RAMFSFile* file = new RAMFSFile;

        file->name = header->name;
        file->size = parser.currentFileSize();
        file->data = parser.currentFileData();
        file->isDirectory = (file->size == 0 && file->name[std::strlen(file->name) - 1] == '/');
        file->next = nullptr;

       	if (file->isDirectory) {
            kprintf("Loaded directory: %s\n", file->name);
        } else {
            kprintf("Loaded file: %s, size: %llu\n", file->name, file->size);
        }

        if (last) {
            last->next = file;
        } else {
            root = file;
        }
        last = file;

    } while (parser.next());
}

RAMFSFile* open(const char* filename) {
    RAMFSFile* current = root;
    while (current) {
        if (std::strcmp(current->name, filename) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

size_t read(RAMFSFile* file, uint8_t* buffer, size_t size) {
    if (!file || !buffer) return 0;
    size_t toRead = (size > file->size) ? file->size : size;
    std::memcpy(buffer, file->data, toRead);
    return toRead;
}

bool dir(RAMFSFile* folder) {
    if (!folder || !folder->isDirectory) {
        kprintf("Error: Not a directory\n");
        return false;
    }

    size_t folderNameLength = std::strlen(folder->name);
    RAMFSFile* current = root;

    while (current) {
        const char* relativePath = current->name + folderNameLength;
        if (std::strncmp(current->name, folder->name, folderNameLength) == 0 &&
            (relativePath[0] == '\0' || (relativePath[0] == '/' && std::strchr(relativePath + 1, '/') == nullptr))) {
            kprintf(" File: %s, Size: %llu\n", current->name, current->size);
        }
        current = current->next;
    }

    return true;
}

void cleanup() {
    RAMFSFile* current = root;
    while (current) {
        RAMFSFile* next = current->next;
        delete current;
        current = next;
    }
    root = nullptr;
    kprintf(" -> RAMFS Cleanup Complete.\n");
}

size_t fsize(RAMFSFile* file) {
    return file->size;
}

} // namespace ramfs::internal
