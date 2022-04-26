#pragma once
#include "intrusive_cartesian_tree.h"
#include <cstddef>
#include <list>
#include <set>
template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
          typename CompareRight = std::less<Right>>
class bimap {

  using left_t = Left;
  using right_t = Right;
  using left_cmp_t = CompareLeft;
  using right_cmp_t = CompareRight;
  using left_tree_t = IntrusiveCartesianTree<LeftNode, left_t, left_cmp_t>;
  using right_tree_t = IntrusiveCartesianTree<RightNode, right_t, right_cmp_t>;
  using left_node_t = LeftNode<left_t>;
  using right_node_t = RightNode<right_t>;
private:
  left_cmp_t left_compare;
  right_cmp_t right_compare;
  left_tree_t left_set;
  right_tree_t right_set;
  size_t map_size = 0;

  template <class A, class B, class AComparator, class BComparator,
            template <typename> class ANode, template <typename> class BNode,
            class Derived, class FlippedDerived>
  class base_iterator {
    using a_tree_t = IntrusiveCartesianTree<ANode, A, AComparator>;
    using b_tree_t = IntrusiveCartesianTree<BNode, B, BComparator>;
    using a_node_t = ANode<A>;
    using b_node_t = BNode<B>;

    const a_tree_t* set;
    const b_tree_t* flip_set;
    const a_node_t* node_ptr;
    bool end;

  protected:
    base_iterator(const a_tree_t* set, const b_tree_t* flip_set,
                  const a_node_t* node_ptr, bool end = false)
        : set(set), flip_set(flip_set), node_ptr(node_ptr), end(end) {}
  public:
    A const& operator*() const {
      return node_ptr->value;
    }

    A const* operator->() const {
      return &(node_ptr->value);
    }

    Derived& operator++() {
      if (set->next(node_ptr) == nullptr) {
        end = true;
        return static_cast<Derived&>(*this);
      }
      node_ptr = set->next(node_ptr);
      return static_cast<Derived&>(*this);
    }

    Derived operator++(int) {
      Derived temp = static_cast<Derived&>(*this);
      ++*this;
      return temp;
    }

    Derived& operator--() {
      if (end) {
        end = false;
        return static_cast<Derived&>(*this);
      }
      node_ptr = set->prev(node_ptr);
      return static_cast<Derived&>(*this);
    }

    Derived operator--(int) {
      Derived temp = static_cast<Derived&>(*this);
      --*this;
      return temp;
    }

    FlippedDerived flip() const {
      if (end) {
        return FlippedDerived(flip_set, set, flip_set->end(), true);
      }
      return FlippedDerived(
          flip_set, set,
          static_cast<const b_node_t*>(
              static_cast<const Node<left_t, right_t>*>(this->node_ptr)),
          end);
    }

    bool operator==(const base_iterator& rhs) const {
      return (end == rhs.end) && (node_ptr == rhs.node_ptr);
    }

    bool operator!=(const base_iterator& rhs) const {
      return !(*this == rhs);
    }
  };

  class right_iterator;

  class left_iterator
      : public base_iterator<left_t, right_t, left_cmp_t, right_cmp_t, LeftNode,
                             RightNode, left_iterator, right_iterator> {
    friend class bimap;

    left_iterator(const left_tree_t* set, const right_tree_t* flip_set,
                  const left_node_t* node_ptr, bool end = false)
        : base_iterator<left_t, right_t, left_cmp_t, right_cmp_t, LeftNode,
                        RightNode, left_iterator, right_iterator>(
              set, flip_set, node_ptr, end) {}
  };

  class right_iterator : public base_iterator<right_t, left_t, right_cmp_t,
                                               left_cmp_t, RightNode, LeftNode,
                                               right_iterator, left_iterator> {
    friend class bimap;
    right_iterator(const right_tree_t* set, const left_tree_t* flip_set,
                   const right_node_t* node_ptr, bool end = false)
        : base_iterator<right_t, left_t, right_cmp_t, left_cmp_t, RightNode,
                        LeftNode, right_iterator, left_iterator>(
              set, flip_set, node_ptr, end) {}
  };

public:
  bimap(left_cmp_t compare_left = left_cmp_t(),
        right_cmp_t compare_right = right_cmp_t())
      : left_compare(compare_left), right_compare(compare_right),
        left_set(nullptr, compare_left), right_set(nullptr, compare_right) {};

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
    std::swap(left_set, other.left_set);
    std::swap(right_set, other.right_set);
  }

  bimap& operator=(bimap const& other) {
    if (this == &other) {
      return *this;
    }
    this->~bimap();
    left_iterator other_left_iterator = other.begin_left();
    while (other_left_iterator != other.end_left()) {
      this->insert(*other_left_iterator, *other_left_iterator.flip());
      other_left_iterator++;
    }
    return *this;
  }

  void swap(bimap& other) {
    std::swap(left_set, other.left_set);
    std::swap(right_set, other.right_set);
    std::swap(left_compare, other.left_compare);
    std::swap(right_compare, other.right_compare);
    std::swap(map_size, other.map_size);
  }

  bimap& operator=(bimap&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->~bimap();
    this->swap(other);
    return *this;
  }

  ~bimap() {
    size_t init_size = size();
    for (size_t i = 0; i < init_size; i++) {
      erase_left(*begin_left());
    }
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
    auto* node =
        new Node<left_t, right_t>(std::move(left), std::move(right), rand());
    if (left_set.find(node->LeftNode<left_t>::value) != nullptr ||
        right_set.find(node->RightNode<right_t>::value) != nullptr) {
      delete node;
      return left_iterator{&left_set, &right_set, left_set.end(), true};
    }
    left_set.insert(node);
    right_set.insert(node);
    map_size++;
    return left_iterator{&left_set, &right_set, node};
  }

  left_iterator erase_left(left_iterator it) {
    auto curIterator = *it;
    it++;
    erase_left(curIterator);
    return it;
  }

  bool erase_left(left_t const& left) {
    auto removedNode = left_set.remove(left);
    if (removedNode == nullptr) {
      return false;
    }
    auto right = static_cast<RightNode<right_t>*>(
        static_cast<Node<left_t, right_t>*>(removedNode));
    right_set.remove(right->value);
    delete static_cast<Node<left_t, right_t>*>(removedNode);
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
      return left_iterator(&left_set, &right_set, found);
    }
  }

  right_iterator find_right(right_t const& right) const {
    auto found = right_set.find(right);
    if (found == nullptr) {
      return end_right();
    } else {
      return right_iterator(&right_set, &left_set, found);
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

  template<class Q = right_t>
  typename std::enable_if<std::is_default_constructible<Q>::value, right_t>::type const&
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

  template<class Q = left_t>
  typename std::enable_if<std::is_default_constructible<Q>::value, left_t>::type const&
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
    return left_iterator(&left_set, &right_set, lower);
  }

  left_iterator upper_bound_left(const left_t& left) const {
    auto lower = lower_bound_left(left);
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
    return right_iterator(&right_set, &left_set, lower);
  }

  right_iterator upper_bound_right(const right_t& right) const {
    auto lower = lower_bound_right(right);
    if (*lower == right) {
      lower++;
    }
    return lower;
  }

  left_iterator begin_left() const {
    return left_iterator(&left_set, &right_set, left_set.begin());
  }

  left_iterator end_left() const {
    return left_iterator(&left_set, &right_set, left_set.end(), true);
  }

  right_iterator begin_right() const {
    return right_iterator(&right_set, &left_set, right_set.begin());
  }

  right_iterator end_right() const {
    return right_iterator(&right_set, &left_set, right_set.end(), true);
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
