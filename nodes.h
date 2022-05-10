#pragma once
#include <climits>

template <class Tag>
struct IntrusiveNode {
  IntrusiveNode<Tag>* left = nullptr;
  IntrusiveNode<Tag>* right = nullptr;
  IntrusiveNode<Tag>* top = nullptr;
  int weight;
  IntrusiveNode() {}

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

template <class Type, class Tag>
struct NodeBase : public IntrusiveNode<Tag> {
  Type value;

  template <class ValueType = Type>
  NodeBase(ValueType&& value)
      : IntrusiveNode<Tag>(), value(std::forward<ValueType>(value)) {}
};
