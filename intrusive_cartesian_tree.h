#pragma once
#include "nodes.h"

template <class Tag, class Value, class LessComparator>
class IntrusiveCartesianTree {
private:
  using node_base = NodeBase<Value, Tag>;
  IntrusiveNode<Tag>* head;
  LessComparator less_comp;

  std::pair<IntrusiveNode<Tag>*, IntrusiveNode<Tag>*> split(IntrusiveNode<Tag>* node,
                                          const Value& split_value) {
    if (node == nullptr) {
      return {nullptr, nullptr};
    } else if (less_comp(getValue(node), split_value)) {
      auto pair = split(node->right, split_value);
      linkRight(node, pair.first);
      return removeTops(node, pair.second);
    } else {
      auto pair = split(node->left, split_value);
      linkLeft(node, pair.second);
      return removeTops(pair.first, node);
    }
  }

  std::pair<IntrusiveNode<Tag> *, IntrusiveNode<Tag>*>
  removeTops(IntrusiveNode<Tag>* left, IntrusiveNode<Tag> * right) {
    if (left != nullptr) {
      left->top = nullptr;
    }
    if (right != nullptr) {
      right->top = nullptr;
    }
    return {left, right};
  }


  IntrusiveNode<Tag>* merge(IntrusiveNode<Tag>* left,
                            IntrusiveNode<Tag>* right) {
    if (right == nullptr) {
      return left;
    }
    if (left == nullptr) {
      return right;
    }
    if (left->weight > right->weight) {
      linkRight(left, merge(left->right, right));
      left->top = nullptr;
      return left;
    } else {
      linkLeft(right, merge(left, right->left));
      right->top = nullptr;
      return right;
    }
  }

  void linkRight(IntrusiveNode<Tag>* parent, IntrusiveNode<Tag>* child) {
    if (parent != nullptr) {
      parent->right = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  void linkLeft(IntrusiveNode<Tag>* parent, IntrusiveNode<Tag>* child) {
    if (parent != nullptr) {
      parent->left = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  const IntrusiveNode<Tag>* find(const Value& value, const IntrusiveNode<Tag> * node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (less_comp(value, getValue(node))) {
      return find(value, node->left);
    }
    if (less_comp(getValue(node), value)) {
      return find(value, node->right);
    }
    return node;
  }



public:
  IntrusiveCartesianTree(IntrusiveNode<Tag>* head, LessComparator lessComp)
      : head(head), less_comp(lessComp) {}

  void insert(node_base* node) { // DONE
    auto split_by_value = split(head->left, node->value);
    auto left_subtree = merge(split_by_value.first, node);
    linkLeft(head, merge(left_subtree, split_by_value.second));
  }

  const IntrusiveNode<Tag>* find(const Value& value) const {
    return find(value, head->left);
  }

  IntrusiveNode<Tag>* remove(const Value& value) {
    auto found = find(value, head->left);
    if (found == nullptr) {
      return nullptr;
    }
    auto next_found = found->next();
    auto split_by_value = split(head->left, value);
    if (next_found == head) {
      linkLeft(head, split_by_value.first);
    } else {
      auto split_by_next_value =
          split(split_by_value.second, getValue(next_found));
      linkLeft(head, merge(split_by_value.first, split_by_next_value.second));
    }
    return const_cast<IntrusiveNode<Tag>*>(found);
  }

  const IntrusiveNode<Tag>* end() const {
    return head;
  }

  const IntrusiveNode<Tag>* begin() const {
    return head->left == nullptr ? head : head->left->min();
  }

  const Value& getValue(const IntrusiveNode<Tag> * node) const {
    return dynamic_cast<const node_base*>(node)->value;
  }

  const IntrusiveNode<Tag>* lower_bound(const Value& value) const {
    auto found = find(value);
    if (found != nullptr) {
      return found;
    }
    auto cur = head->left;
    while (cur != nullptr && less_comp(getValue(cur), value)) {
      cur = cur->right;
    }
    if (cur == nullptr) {
      return nullptr;
    }
    auto cur_lower_bound = cur;
    cur = cur->left;
    while (cur != nullptr) {
      if (less_comp(getValue(cur), value)) {
        cur = cur->right;
      } else {
        if (less_comp(getValue(cur), getValue(cur_lower_bound))) {
          cur_lower_bound = cur;
        }
        cur = cur->left;
      }
    }
    return cur_lower_bound;
  }
};