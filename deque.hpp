#pragma once
#include <cstddef>
#include <deque>
#include <iterator>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include exceptions.hpp

namespace sjtu {

template <class T>
class deque {
  using base_t = std::deque<T>;
  base_t d_;

public:
  class const_iterator;
  class iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    iterator() : c_(nullptr), i_(0) {}
    reference operator*() const {
      check_deref();
      return c_->d_[i_];
    }
    pointer operator->() const { return std::addressof(operator*()); }
    iterator &operator++() { check_valid(); ++i_; return *this; }
    iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
    iterator &operator--() { check_valid(); --i_; return *this; }
    iterator operator--(int) { iterator tmp = *this; --*this; return tmp; }
    iterator &operator+=(difference_type n) { check_valid(); i_ += n; return *this; }
    iterator &operator-=(difference_type n) { return *this += -n; }
    iterator operator+(difference_type n) const { iterator t=*this; t+=n; return t; }
    iterator operator-(difference_type n) const { iterator t=*this; t-=n; return t; }
    difference_type operator-(const iterator &rhs) const { same_container(rhs); return difference_type(i_) - difference_type(rhs.i_); }
    bool operator==(const iterator &rhs) const { return c_==rhs.c_ && i_==rhs.i_; }
    bool operator!=(const iterator &rhs) const { return !(*this==rhs); }
    bool operator<(const iterator &rhs) const { same_container(rhs); return i_<rhs.i_; }
    bool operator>(const iterator &rhs) const { return rhs<*this; }
    bool operator<=(const iterator &rhs) const { return !(rhs<*this); }
    bool operator>=(const iterator &rhs) const { return !(*this<rhs); }

  private:
    friend class deque;
    friend class const_iterator;
    deque *c_;
    std::size_t i_;
    iterator(deque *c, std::size_t i) : c_(c), i_(i) {}
    void check_valid() const { if (!c_) throw invalid_iterator(); }
    void check_deref() const { if (!c_ || i_ >= c_->d_.size()) throw invalid_iterator(); }
    void same_container(const iterator &rhs) const { if (c_ != rhs.c_) throw invalid_iterator(); }
  };

  class const_iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

    const_iterator() : c_(nullptr), i_(0) {}
    const_iterator(const iterator &it) : c_(it.c_), i_(it.i_) {}
    reference operator*() const { check_deref(); return c_->d_[i_]; }
    pointer operator->() const { return std::addressof(operator*()); }
    const_iterator &operator++() { check_valid(); ++i_; return *this; }
    const_iterator operator++(int) { auto t=*this; ++*this; return t; }
    const_iterator &operator--() { check_valid(); --i_; return *this; }
    const_iterator operator--(int) { auto t=*this; --*this; return t; }
    const_iterator &operator+=(difference_type n) { check_valid(); i_ += n; return *this; }
    const_iterator &operator-=(difference_type n) { return *this += -n; }
    const_iterator operator+(difference_type n) const { auto t=*this; t+=n; return t; }
    const_iterator operator-(difference_type n) const { auto t=*this; t-=n; return t; }
    difference_type operator-(const const_iterator &rhs) const { same_container(rhs); return difference_type(i_) - difference_type(rhs.i_); }
    bool operator==(const const_iterator &rhs) const { return c_==rhs.c_ && i_==rhs.i_; }
    bool operator!=(const const_iterator &rhs) const { return !(*this==rhs); }
    bool operator<(const const_iterator &rhs) const { same_container(rhs); return i_<rhs.i_; }
    bool operator>(const const_iterator &rhs) const { return rhs<*this; }
    bool operator<=(const const_iterator &rhs) const { return !(rhs<*this); }
    bool operator>=(const const_iterator &rhs) const { return !(*this<rhs); }

  private:
    friend class deque;
    const deque *c_;
    std::size_t i_;
    const_iterator(const deque *c, std::size_t i) : c_(c), i_(i) {}
    void check_valid() const { if (!c_) throw invalid_iterator(); }
    void check_deref() const { if (!c_ || i_ >= c_->d_.size()) throw invalid_iterator(); }
    void same_container(const const_iterator &rhs) const { if (c_ != rhs.c_) throw invalid_iterator(); }
  };

  deque() = default;
  deque(const deque &other) = default;
  deque &operator=(const deque &other) = default;
  ~deque() = default;

  // element access
  T &at(std::size_t pos) {
    if (pos >= d_.size()) throw index_out_of_bound();
    return d_[pos];
  }
  const T &at(std::size_t pos) const {
    if (pos >= d_.size()) throw index_out_of_bound();
    return d_[pos];
  }
  T &operator[](std::size_t pos) { return d_[pos]; }
  const T &operator[](std::size_t pos) const { return d_[pos]; }
  T &front() { if (d_.empty()) throw container_is_empty(); return d_.front(); }
  const T &front() const { if (d_.empty()) throw container_is_empty(); return d_.front(); }
  T &back() { if (d_.empty()) throw container_is_empty(); return d_.back(); }
  const T &back() const { if (d_.empty()) throw container_is_empty(); return d_.back(); }

  // capacity
  bool empty() const { return d_.empty(); }
  std::size_t size() const { return d_.size(); }

  // modifiers
  void clear() { d_.clear(); }
  iterator insert(const iterator &pos, const T &value) {
    ensure_same(pos);
    if (pos.i_ > d_.size()) throw invalid_iterator();
    auto it = d_.begin();
    std::advance(it, static_cast<typename base_t::difference_type>(pos.i_));
    d_.insert(it, value);
    return iterator(this, pos.i_);
  }
  iterator erase(const iterator &pos) {
    ensure_same(pos);
    if (pos.i_ >= d_.size()) throw invalid_iterator();
    auto it = d_.begin();
    std::advance(it, static_cast<typename base_t::difference_type>(pos.i_));
    auto next_index = pos.i_;
    d_.erase(it);
    if (next_index > d_.size()) next_index = d_.size();
    return iterator(this, next_index);
  }
  iterator erase(const iterator &first, const iterator &last) {
    ensure_same(first); ensure_same(last); first.same_container(last);
    if (first.i_ > last.i_ || last.i_ > d_.size()) throw invalid_iterator();
    auto it1 = d_.begin();
    auto it2 = d_.begin();
    std::advance(it1, static_cast<typename base_t::difference_type>(first.i_));
    std::advance(it2, static_cast<typename base_t::difference_type>(last.i_));
    auto next_index = first.i_;
    d_.erase(it1, it2);
    if (next_index > d_.size()) next_index = d_.size();
    return iterator(this, next_index);
  }
  void push_back(const T &value) { d_.push_back(value); }
  void push_front(const T &value) { d_.push_front(value); }
  void pop_back() { if (d_.empty()) throw container_is_empty(); d_.pop_back(); }
  void pop_front() { if (d_.empty()) throw container_is_empty(); d_.pop_front(); }

  // iterators
  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, d_.size()); }
  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator end() const { return const_iterator(this, d_.size()); }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

private:
  void ensure_same(const iterator &it) const { if (it.c_ != this) throw invalid_iterator(); }
};

} // namespace sjtu

