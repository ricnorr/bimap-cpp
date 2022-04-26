#pragma once
#include "nodes.h"

template <template <typename> class Node, class Value, class LessComparator>
class IntrusiveCartesianTree {
private:
  using node_t = Node<Value>;

  node_t* head;
  LessComparator less_comp;

  std::pair<node_t*, node_t*> split(node_t* treap, const Value& split_value) {
    if (treap == nullptr) {
      return {nullptr, nullptr};
    } else if (less_comp(treap->value, split_value)) {
      auto pair = split(treap->right, split_value);
      linkRight(treap, pair.first);
      return removeTops(treap, pair.second);
    } else {
      auto pair = split(treap->left, split_value);
      linkLeft(treap, pair.second);
      return removeTops(pair.first, treap);
    }
  }

  std::pair<node_t*, node_t*> removeTops(node_t* left, node_t* right) {
    if (left != nullptr) {
      left->top = nullptr;
    }
    if (right != nullptr) {
      right->top = nullptr;
    }
    return {left, right};
  }

  void linkRight(node_t* parent, node_t* child) {
    if (parent != nullptr) {
      parent->right = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  void linkLeft(node_t* parent, node_t* child) {
    if (parent != nullptr) {
      parent->left = child;
    }
    if (child != nullptr) {
      child->top = parent;
    }
  }

  node_t* merge(node_t* left_treap, node_t* right_treap) {
    if (right_treap == nullptr) {
      return left_treap;
    }
    if (left_treap == nullptr) {
      return right_treap;
    }
    if (left_treap->weight > right_treap->weight) {
      linkRight(left_treap, merge(left_treap->right, right_treap));
      left_treap->top = nullptr;
      return left_treap;
    } else {
      linkLeft(right_treap, merge(left_treap, right_treap->left));
      right_treap->top = nullptr;
      return right_treap;
    }
  }

  node_t* find(const Value& value, node_t* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (less_comp(value, node->value)) {
      return find(value, node->left);
    }
    if (less_comp(node->value, value)) {
      return find(value, node->right);
    }
    return node;
  }

  node_t* max(node_t* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (node->right == nullptr) {
      return node;
    }
    return max(node->right);
  }

  node_t* min(node_t* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (node->left == nullptr) {
      return node;
    }
    return min(node->left);
  }

public:
  IntrusiveCartesianTree(node_t* head, LessComparator lessComp)
      : head(head), less_comp(lessComp) {}

  void insert(node_t* node) {
    auto pair = split(head, node->value);
    auto temp_head = merge(pair.first, node);
    head = merge(temp_head, pair.second);
  }

  node_t* find(const Value& value) const {
    return find(value, head);
  }

  node_t* remove(const Value& value) {
    auto found = find(value, head);
    if (found == nullptr) {
      return nullptr;
    }
    auto pair = split(head, value);
    auto cur = pair.second;
    if (cur->left == nullptr) {
      if (cur->right != nullptr) {
        cur->right->top = nullptr;
      }
      head = merge(pair.first, cur->right);
      cur->right = nullptr;
      return cur;
    }
    while (cur->left->left != nullptr) {
      cur = cur->left;
    }
    auto ans = cur->left;
    cur->left = nullptr;
    ans->top = nullptr;
    if (ans->right != nullptr) {
      ans->right->top = nullptr;
    }
    head = merge(pair.first, merge(ans->right, pair.second));
    ans->right = nullptr;
    return ans;
  }

  node_t* next(const node_t* node) const {
    if (node->right != nullptr) {
      return min(node->right);
    } else {
      auto cur = node;
      while (cur->top != nullptr && cur->top->left != cur) {
        cur = cur->top;
      }
      return cur->top;
    }
  }

  node_t* prev(const node_t* node) const {
    if (node->left != nullptr) {
      return max(node->left);
    } else {
      auto cur = node;
      while (cur->top != nullptr && cur->top->right != cur) {
        cur = cur->top;
      }
      return cur->top;
    }
  }

  node_t* end() const {
    auto cur = head;
    if (cur == nullptr) {
      return nullptr;
    }
    while (cur->right != nullptr) {
      cur = cur->right;
    }
    return cur;
  }

  node_t* begin() const {
    auto cur = head;
    if (head == nullptr) {
      return nullptr;
    }
    while (cur->left != nullptr) {
      cur = cur->left;
    }
    return cur;
  }

  node_t* lower_bound(const Value& value) const {
    auto found = find(value);
    if (found != nullptr) {
      return found;
    }
    auto cur = head;
    while (cur != nullptr && less_comp(cur->value, value)) {
      cur = cur->right;
    }
    if (cur == nullptr) {
      return nullptr;
    }
    node_t * cur_lower_bound = cur;
    cur = cur->left;
    while (cur != nullptr) {
      if (less_comp(cur->value, value)) {
        cur = cur->right;
      } else {
        if (less_comp(cur->value, cur_lower_bound->value)) {
          cur_lower_bound = cur;
        }
        cur = cur->left;
      }
    }
    return cur_lower_bound;
  }
};