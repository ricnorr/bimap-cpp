#pragma once
#include <climits>

template <class Tag>
struct IntrusiveNode {
  IntrusiveNode<Tag>* left = nullptr;
  IntrusiveNode<Tag>* right = nullptr;
  IntrusiveNode<Tag>* top = nullptr;
  int weight;
  IntrusiveNode(int weight) : weight(weight) {}

  virtual ~IntrusiveNode() = default;

  const IntrusiveNode<Tag>* next() const {
    if (this->right != nullptr) {
      return this->right->min();
    } else {
      auto cur = this;
      while (cur->top != nullptr && cur->top->left != cur) {
        cur = cur->top;
      }
      return cur->top;
    }
  }

  const IntrusiveNode<Tag>* prev() const {
    if (this->left != nullptr) {
      return this->left->max();
    } else {
      auto cur = this;
      while (cur->top != nullptr && cur->top->right != cur) {
        cur = cur->top;
      }
      return cur->top;
    }
  }

  const IntrusiveNode<Tag>* min() const {
    if (this->left == nullptr) {
      return this;
    }
    return this->left->min();
  }

  const IntrusiveNode<Tag>* max() const {
    if (this->right == nullptr) {
      return this;
    }
    return this->right->max();
  }
};

struct LeftTag {};
struct RightTag {};

struct NodeHead : public IntrusiveNode<LeftTag>,
                  public IntrusiveNode<RightTag> {
  NodeHead()
      : IntrusiveNode<LeftTag>(INT_MAX), IntrusiveNode<RightTag>(INT_MAX) {}
};

template <class Type, class Tag>
struct NodeBase : public IntrusiveNode<Tag> {
  Type value;
  NodeBase(const Type& value, int weight)
      : IntrusiveNode<Tag>(weight), value(value) {}

  NodeBase(Type&& value, int weight)
      : IntrusiveNode<Tag>(weight), value(std::move(value)) {}
};

template <class Left, class Right>
struct Node : public NodeBase<Left, LeftTag>, public NodeBase<Right, RightTag> {
  Node(const Left& left, const Right& right, int weight)
      : NodeBase<Left, LeftTag>(left, weight), NodeBase<Right, RightTag>(
                                                   right, weight) {}

  Node(Left&& left, Right&& right, int weight)
      : NodeBase<Left, LeftTag>(std::move(left), weight),
        NodeBase<Right, RightTag>(std::move(right), weight) {}
};