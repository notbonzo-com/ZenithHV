#pragma once
#include <stdint.h>
#include <new>
#include <vector>
#include <memory>

namespace vfs {

class Node;

using NodePtr = std::shared_ptr<Node>;

class Node {
public:
	~Node() = default;

	void add_child(NodePtr child) { children.push_back(child); }

private:
	std::vector<NodePtr> children;
};

}