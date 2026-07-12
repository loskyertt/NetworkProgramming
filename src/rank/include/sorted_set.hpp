//
// Created by sky on 2026/7/9.
//

#ifndef NETWORK_PROGRAMMING_SORTED_SET_H
#define NETWORK_PROGRAMMING_SORTED_SET_H

#include <cstddef>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>

namespace sky::rank {

template <typename Key, typename Value>
class SortedSet {
 public:
  using key_type = Key;
  using value_type = Value;
  using size_type = std::size_t;

 private:
  struct KeyValueType {
    key_type key;
    value_type value;

    explicit KeyValueType(const key_type &k, const value_type &v) : key(k), value(v) {}
  };

  using key_value_ptr = std::shared_ptr<KeyValueType>;

  /* Comparator: ascending order. */
  struct ValueLess {
    bool operator()(const key_value_ptr &left, const key_value_ptr &right) const {
      if (left->value != right->value)
        return left->value < right->value;

      return left->key < right->key;
    }
  };

  static constexpr size_type default_top = 1000;  // Default rank size.

  size_type m_top;                                    // Rank size.
  std::unordered_map<key_type, key_value_ptr> m_map;  // Fast lookup: O(1) average.
  std::set<key_value_ptr, ValueLess> m_set;           // Ordered by ValueLess: O(log n).

 public:
  SortedSet() : m_top(default_top) {}

  explicit SortedSet(const size_type top) : m_top(top == 0 ? default_top : top) {}

  SortedSet(SortedSet &&right) noexcept
      : m_top(right.m_top),
        m_map(std::move(right.m_map)),
        m_set(std::move(right.m_set)) {}

  SortedSet &operator=(SortedSet &&right) noexcept {
    if (this == &right)
      return *this;

    m_top = right.m_top;
    m_map = std::move(right.m_map);
    m_set = std::move(right.m_set);
    return *this;
  }

  ~SortedSet() = default;

  // Keep deleted copy operations public for clearer compiler diagnostics.
  SortedSet(const SortedSet &) = delete;
  SortedSet &operator=(const SortedSet &) = delete;

  // Iterate from largest to smallest.
  auto begin() const { return m_set.rbegin(); }

  auto end() const { return m_set.rend(); }

  auto find(const key_type &k) const { return m_map.find(k); }

  auto map_end() const { return m_map.end(); }

  /**
   * @brief Insert or update a value.
   *
   * @param k Key
   * @param v Value
   */
  void insert(const key_type &k, const value_type &v) {
    auto add = [&] {
      key_value_ptr ptr = std::make_shared<KeyValueType>(k, v);
      m_map.emplace(k, ptr);
      m_set.insert(ptr);
    };

    auto it = m_map.find(k);
    if (it == m_map.end()) {
      if (m_set.size() < m_top) {
        add();
      } else if (m_set.size() == m_top) {
        auto min = m_set.begin();  // Smallest value in m_set.
        auto ptr = std::make_shared<KeyValueType>(k, v);

        if (!ValueLess{}(*min, ptr))
          return;

        m_map.erase((*min)->key);
        m_set.erase(min);
        m_map.emplace(k, ptr);
        m_set.insert(ptr);
      }
    }
    // Existing key: update it.
    else {
      // Erase from m_set first because m_map.erase(it) invalidates it.
      m_set.erase(it->second);
      m_map.erase(it);
      add();
    }
  }

  /**
   * @brief Erase a key.
   *
   * @param k Key
   */
  void erase(const key_type &k) {
    auto it = m_map.find(k);
    if (it == m_map.end())
      return;
    m_set.erase(it->second);
    m_map.erase(it);
  }

  /**
   * @brief Clear the container.
   */
  void clear() {
    m_map.clear();
    m_set.clear();
  }

  /**
   * @return Current size.
   */
  size_type size() const { return m_set.size(); }

  /**
   * @brief Get current rank by key.
   *
   * @param k Key
   * @return Rank, or 0 if the key does not exist.
   */
  size_type get_rank(const key_type &k) const {
    if (m_map.contains(k)) {
      size_type rank = 0;
      for (auto it = m_set.rbegin(); it != m_set.rend(); ++it) {
        ++rank;
        if ((*it)->key == k)
          return rank;
      }
    }

    return 0;
  }
};

}  // namespace sky::rank

#endif  //NETWORK_PROGRAMMING_SORTED_SET_H
