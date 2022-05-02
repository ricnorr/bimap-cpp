#pragma once
#include "intrusive_cartesian_tree.h"
#include <cstddef>
#include <list>
#include <set>
#include <climits>

template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
          typename CompareRight = std::less<Right>>
class bimap {

  using left_t = Left;
  using right_t = Right;
  using left_cmp_t = CompareLeft;
  using right_cmp_t = CompareRight;
  using left_tree_t = IntrusiveCartesianTree<LeftTag, left_t, left_cmp_t>;
  using right_tree_t = IntrusiveCartesianTree<RightTag, right_t, right_cmp_t>;

private:
  left_cmp_t left_compare;
  right_cmp_t right_compare;
  NodeHead head = NodeHead();
  left_tree_t left_set;
  right_tree_t right_set;
  size_t map_size = 0;

  void delete_all() {
    size_t init_size = size();
    for (size_t i = 0; i < init_size; i++) {
      erase_left(*begin_left());
    }
  }

  template <class Tag, class Value, class Derived>
  class base_iterator {
  protected:
    const IntrusiveNode<Tag>* node_ptr;
    base_iterator(const IntrusiveNode<Tag>* node_ptr) : node_ptr(node_ptr) {}

  public:
    Value const& operator*() const {
      return static_cast<const NodeBase<Value, Tag>*>(node_ptr)->value;
    }

    Value const* operator->() const {
      return &(*(*this));
    }

    Derived& operator++() {
      node_ptr = node_ptr->next();
      return static_cast<Derived&>(*this);
    }

    Derived operator++(int) {
      Derived temp = static_cast<Derived&>(*this);
      ++*this;
      return temp;
    }

    Derived& operator--() {
      node_ptr = node_ptr->prev();
      return static_cast<Derived&>(*this);
    }

    Derived operator--(int) {
      Derived temp = static_cast<Derived&>(*this);
      --*this;
      return temp;
    }

    bool operator==(const base_iterator& rhs) const {
      return node_ptr == rhs.node_ptr;
    }

    bool operator!=(const base_iterator& rhs) const {
      return !(*this == rhs);
    }
  };

  class right_iterator;

  class left_iterator : public base_iterator<LeftTag, Left, left_iterator> {
    friend class bimap;

    left_iterator(const IntrusiveNode<LeftTag>* node_ptr)
        : base_iterator<LeftTag, Left, left_iterator>(node_ptr) {}

  public:
    right_iterator flip() {
      return right_iterator(
          dynamic_cast<const IntrusiveNode<RightTag>*>(this->node_ptr));
    }
  };

  class right_iterator : public base_iterator<RightTag, Right, right_iterator> {
    friend class bimap;
    right_iterator(const IntrusiveNode<RightTag>* node_ptr)
        : base_iterator<RightTag, Right, right_iterator>(node_ptr) {}

  public:
    left_iterator flip() {
      return left_iterator(
          dynamic_cast<const IntrusiveNode<LeftTag>*>(this->node_ptr));
    }
  };

public:
  bimap(left_cmp_t compare_left = left_cmp_t(),
        right_cmp_t compare_right = right_cmp_t())
      : left_compare(compare_left), right_compare(compare_right),
        left_set(&head, compare_left), right_set(&head, compare_right){

                                       };

  bimap(bimap const& other) : bimap(other.left_compare, other.right_compare) {
    left_iterator other_left_iterator = other.begin_left();
    while (other_left_iterator != other.end_left()) {
      auto left_val = *other_left_iterator;
      auto right_val = *other_left_iterator.flip();
      this->insert(left_val, right_val);
      other_left_iterator++;
    }
  }
  bimap(bimap&& other) noexcept
      : bimap(other.left_compare, other.right_compare) {
    this->swap(other);
  }

  bimap& operator=(bimap const& other) {
    if (this == &other) {
      return *this;
    }
    delete_all();
    auto other_left_iterator = other.begin_left();
    while (other_left_iterator != other.end_left()) {
      this->insert(*other_left_iterator, *other_left_iterator.flip());
      other_left_iterator++;
    }
    return *this;
  }

  void swap(bimap& other) {
    std::swap(head, other.head);
    std::swap(left_compare, other.left_compare);
    std::swap(right_compare, other.right_compare);
    this->left_set = left_tree_t(&head, left_compare);
    this->right_set = right_tree_t(&head, right_compare);
    if (left_set.end()->left != nullptr) {
      left_set.end()->left->top = &head;
    }
    if (right_set.end()->left != nullptr) {
      right_set.end()->left->top = &head;
    }
    other.left_set = left_tree_t(&other.head, other.left_compare);
    other.right_set = right_tree_t(&other.head, other.right_compare);
    if (other.left_set.end()->left != nullptr) {
      other.left_set.end()->left->top = &other.head;
    }
    if (other.right_set.end()->left != nullptr) {
      other.right_set.end()->left->top = &other.head;
    }
  }

  bimap& operator=(bimap&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->swap(other);
    return *this;
  }

  ~bimap() {
    delete_all();
  }

  left_iterator insert(const left_t& left, const right_t& right) {
    auto l = left;
    auto r = right;
    return insert(std::move(l), std::move(r));
  }

  left_iterator insert(const left_t& left, right_t&& right) {
    auto l = left;
    return insert(std::move(l), std::move(right));
  }

  left_iterator insert(left_t&& left, const right_t& right) {
    auto r = right;
    return insert(std::move(left), std::move(r));
  }

  left_iterator insert(left_t&& left, right_t&& right) {
    if (left_set.find(left) != nullptr || right_set.find(right) != nullptr) {
      return left_iterator(left_set.end());
    }
    auto* node =
        new Node<left_t, right_t>(std::move(left), std::move(right), rand() % INT_MAX);
    left_set.insert(node);
    right_set.insert(node);
    map_size++;
    return left_iterator{node};
  }

  left_iterator erase_left(left_iterator it) {
    auto curIterator = *it;
    it++;
    erase_left(curIterator);
    return it;
  }

  bool erase_left(left_t const& left) {
    auto iterator_on_removed = find_left(left);
    if (iterator_on_removed == end_left()) {
      return false;
    }
    left_set.remove(*iterator_on_removed);
    auto removed = right_set.remove(*iterator_on_removed.flip());
    delete dynamic_cast<Node<left_t, right_t>*>(removed);
    map_size--;
    return true;
  }

  right_iterator erase_right(right_iterator it) {
    auto curValue = *it;
    it++;
    erase_right(curValue);
    return it;
  }

  bool erase_right(right_t const& right) {
    auto found_right = find_right(right);
    if (found_right == end_right()) {
      return false;
    } else {
      erase_left(*found_right.flip());
      return true;
    }
  }

  left_iterator erase_left(left_iterator first, left_iterator last) {
    while (true) {
      if (first == last) {
        return last;
      }
      left_iterator cur = first;
      first++;
      erase_left(*cur);
    }
  }

  right_iterator erase_right(right_iterator first, right_iterator last) {
    while (true) {
      if (first == last) {
        return last;
      }
      right_iterator cur = first;
      first++;
      erase_right(*cur);
    }
  }

  left_iterator find_left(left_t const& left) const {
    auto found = left_set.find(left);
    if (found == nullptr) {
      return end_left();
    } else {
      return left_iterator(found);
    }
  }

  right_iterator find_right(right_t const& right) const {
    auto found = right_set.find(right);
    if (found == nullptr) {
      return end_right();
    } else {
      return right_iterator(found);
    }
  }

  right_t const& at_left(left_t const& key) const {
    auto found_iterator = find_left(key);
    if (found_iterator == end_left()) {
      throw std::out_of_range("at_left fail");
    }
    return *found_iterator.flip();
  }

  left_t const& at_right(right_t const& key) const {
    auto found_iterator = find_right(key);
    if (found_iterator == end_right()) {
      throw std::out_of_range("at_left fail");
    }
    return *found_iterator.flip();
  }

  template <class Q = right_t>
  typename std::enable_if<std::is_default_constructible<Q>::value,
                          right_t>::type const&
  at_left_or_default(left_t const& key) {
    auto found_left = find_left(key);
    if (found_left == end_left()) {
      auto default_right = right_t();
      auto found_default_right = find_right(default_right);
      if (found_default_right != end_right()) {
        erase_right(default_right);
      }
      return *insert(key, std::move(default_right)).flip();
    } else {
      return *found_left.flip();
    }
  }

  template <class Q = left_t>
  typename std::enable_if<std::is_default_constructible<Q>::value,
                          left_t>::type const&
  at_right_or_default(right_t const& key) {
    auto found_right = find_right(key);
    if (found_right == end_right()) {
      auto default_left = left_t();
      auto found_default_left = find_left(default_left);
      if (found_default_left != end_left()) {
        erase_left(default_left);
      }
      return *insert(std::move(default_left), key);
    } else {
      return *found_right.flip();
    }
  }

  left_iterator lower_bound_left(const left_t& left) const {
    auto lower = left_set.lower_bound(left);
    if (lower == nullptr) {
      return end_left();
    }
    return left_iterator(lower);
  }

  left_iterator upper_bound_left(const left_t& left) const {
    auto lower = lower_bound_left(left);
    if (lower == end_left()) {
      return end_left();
    }
    if (*lower == left) {
      lower++;
    }
    return lower;
  }

  right_iterator lower_bound_right(const right_t& right) const {
    auto lower = right_set.lower_bound(right);
    if (lower == nullptr) {
      return end_right();
    }
    return right_iterator(lower);
  }

  right_iterator upper_bound_right(const right_t& right) const {
    auto lower = lower_bound_right(right);
    if (lower == end_right()) {
      return end_right();
    }
    if (*lower == right) {
      lower++;
    }
    return lower;
  }

  left_iterator begin_left() const {
    return left_iterator(left_set.begin());
  }

  left_iterator end_left() const {
    return left_iterator(left_set.end());
  }

  right_iterator begin_right() const {
    return right_iterator(right_set.begin());
  }

  right_iterator end_right() const {
    return right_iterator(right_set.end());
  }

  bool empty() const {
    return size() == 0;
  }

  std::size_t size() const {
    return map_size;
  }

  friend bool operator==(bimap const& a, bimap const& b) {
    if (a.size() != b.size()) {
      return false;
    }
    auto a_left_iterator = a.begin_left();
    auto b_left_iterator = b.begin_left();
    for (int i = 0; i < a.size(); i++) {
      if (*a_left_iterator != *b_left_iterator) {
        return false;
      }
      if (*a_left_iterator.flip() != *b_left_iterator.flip()) {
        return false;
      }
      a_left_iterator++;
      b_left_iterator++;
    }
    return true;
  }

  friend bool operator!=(bimap const& a, bimap const& b) {
    return !(a == b);
  }
};
