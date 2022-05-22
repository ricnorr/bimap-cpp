#pragma once
#include "nodes.h"
#include <random>

template <class Tag, class Value, class LessComparator = std::less<Value>>
class IntrusiveCartesianTree : private LessComparator {
private:

  IntrusiveNode<Tag>* head;
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<int> dist;

  std::pair<IntrusiveNode<Tag>*, IntrusiveNode<Tag>*>
  split(IntrusiveNode<Tag>* node, const Value& split_value) {
    if (node == nullptr) {
      return {nullptr, nullptr};
    } else if (LessComparator::operator()(get_value(node), split_value)) {
      auto pair = split(node->right, split_value);
      link_right(node, pair.first);
      return remove_tops(node, pair.second);
    } else {
      auto pair = split(node->left, split_value);
      link_left(node, pair.second);
      return remove_tops(pair.first, node);
    }
  }

  std::pair<IntrusiveNode<Tag>*, IntrusiveNode<Tag>*>
  remove_tops(IntrusiveNode<Tag>* left, IntrusiveNode<Tag>* right) {
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
      link_right(left, merge(left->right, right));
      left->top = nullptr;
      return left;
    } else {
      link_left(right, merge(left, right->left));
      right->top = nullptr;
      return right;
    }
  }

  void link_right(IntrusiveNode<Tag>* parent, IntrusiveNode<Tag>* child) {
    if (parent != nullptr) {
      parent->right = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  void link_left(IntrusiveNode<Tag>* parent, IntrusiveNode<Tag>* child) {
    if (parent != nullptr) {
      parent->left = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  const IntrusiveNode<Tag>* find(const Value& value,
                                 const IntrusiveNode<Tag>* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (LessComparator::operator()(value, get_value(node))) {
      return find(value, node->left);
    }
    if (LessComparator::operator()(get_value(node), value)) {
      return find(value, node->right);
    }
    return node;
  }

public:
  IntrusiveCartesianTree(IntrusiveNode<Tag>* head,
                         LessComparator lessComp = LessComparator())
      : LessComparator(lessComp), head(head), dist(0, INT_MAX - 1), gen(rd()) {
    head->weight = INT_MAX;
    if (head->left != nullptr) {
      head->left->top = head;
    }
  }

  IntrusiveCartesianTree& operator=(const IntrusiveCartesianTree& rhs) {
    if (this == &rhs) {
      return *this;
    }
    this->head = rhs.head;
    return *this;
  }

  void insert(NodeBase<Value, Tag>* node) {
    node->weight = dist(gen);
    auto split_by_value = split(head->left, node->value);
    auto left_subtree = merge(split_by_value.first, node);
    link_left(head, merge(left_subtree, split_by_value.second));
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
      link_left(head, split_by_value.first);
    } else {
      auto split_by_next_value =
          split(split_by_value.second, get_value(next_found));
      link_left(head, merge(split_by_value.first, split_by_next_value.second));
    }
    return const_cast<IntrusiveNode<Tag>*>(found);
  }

  const IntrusiveNode<Tag>* end() const {
    return head;
  }

  const IntrusiveNode<Tag>* begin() const {
    return head->left == nullptr ? head : head->left->min();
  }

  const Value& get_value(const IntrusiveNode<Tag>* node) const {
    return dynamic_cast<const NodeBase<Value, Tag>*>(node)->value;
  }

  const IntrusiveNode<Tag>* lower_bound(const Value& value) const {
    auto found = find(value);
    if (found != nullptr) {
      return found;
    }
    auto cur = head->left;
    while (cur != nullptr && LessComparator::operator()(get_value(cur), value)) {
      cur = cur->right;
    }
    if (cur == nullptr) {
      return nullptr;
    }
    auto cur_lower_bound = cur;
    cur = cur->left;
    while (cur != nullptr) {
      if (LessComparator::operator()(get_value(cur), value)) {
        cur = cur->right;
      } else {
        if (LessComparator::operator()(get_value(cur),
                                       get_value(cur_lower_bound))) {
          cur_lower_bound = cur;
        }
        cur = cur->left;
      }
    }
    return cur_lower_bound;
  }
};