#pragma once

template <class Child>
struct IntrusiveNode {
  Child* left = nullptr;
  Child* right = nullptr;
  Child* top = nullptr;
  int weight;
  explicit IntrusiveNode(int weight) : weight(weight) {}
};

template <class Left>
struct LeftNode : public IntrusiveNode<LeftNode<Left>> {
  Left value;
  LeftNode(const Left& value, int weight)
      : value(value), IntrusiveNode<LeftNode<Left>>(weight) {}

  LeftNode(Left&& value, int weight)
      : value(std::move(value)), IntrusiveNode<LeftNode<Left>>(weight) {}
};

template <class Right>
struct RightNode : public IntrusiveNode<RightNode<Right>> {
  Right value;
  RightNode(const Right& value, int weight)
      : value(value), IntrusiveNode<RightNode<Right>>(weight) {}

  RightNode(Right&& value, int weight)
      : value(std::move(value)), IntrusiveNode<RightNode<Right>>(weight) {}
};

template <class Left, class Right>
struct Node : public LeftNode<Left>, public RightNode<Right> {
  Node(const Left& left, const Right& right, int weight)
      : LeftNode<Left>(left, weight), RightNode<Right>(right, weight) {}

  Node(Left&& left, Right&& right, int weight)
      : LeftNode<Left>(std::move(left), weight), RightNode<Right>(
                                                     std::move(right), weight) {
  }
};