#pragma once
#include <stdint.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace vfs {

class Node;

using NodePtr = std::shared_ptr<Node>;

enum : uint8_t {
    READ_OWNER = 0x01,     // 0000 0001
    READ_GROUP = 0x02,     // 0000 0010
    READ_OTHERS = 0x04,    // 0000 0100
    WRITE_OWNER = 0x10,    // 0001 0000
    WRITE_GROUP = 0x20,    // 0010 0000
    WRITE_OTHERS = 0x40,   // 0100 0000
    EXECUTE = 0x80         // 1000 0000
};

class Node {
public:
    Node(NodePtr parent, const std::string& name)
        : parent(parent), children(), name(name), file_size(0), permissions(READ_OTHERS | WRITE_OTHERS) {}

    ~Node() = default;

    NodePtr get_parent() const {
        return parent;
    }

    void set_parent(NodePtr) = delete; // Cannot change the parent

    void add_child(const std::string& name, NodePtr child) {
        children[name] = child;
    }

    NodePtr get_child(const std::string& name) const {
        auto it = children.find(name);
        if (*it != children.end().second()) {
            return *it;
        }
        return nullptr; // Return nullptr if child is not found
    }

    const std::string& get_name() const {
        return name;
    }

    void set_name(const std::string& name) {
        this->name = name;
    }

    void set_size(size_t size) {
        file_size = size;
    }

    size_t get_size() const {
        return file_size;
    }

    void set_permissions(uint8_t perm) {
        permissions = perm;
    }

    uint8_t get_permissions() const {
        return permissions;
    }

    virtual bool is_directory() const {
        return false;
    }

    virtual bool is_file() const {
        return false;
    }

    virtual bool read(size_t offset, uint8_t* buffer, size_t size) const = 0;
    virtual bool write(size_t offset, const uint8_t* buffer, size_t size) = 0;

    Node(Node&&) noexcept = default;
    Node& operator=(Node&&) noexcept = default;
    Node(const Node&) = default;
    Node& operator=(const Node&) = default;

private:
    NodePtr parent;
    std::unordered_map<std::string, NodePtr> children;
    std::string name;
    size_t file_size;
    uint8_t permissions;
};

}