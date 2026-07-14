#include "sorted_set.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace sky::rank;

namespace {

void test_insert_keeps_descending_top_n() {
  SortedSet<int, int> rank(3);

  rank.insert(101, 1000);
  rank.insert(102, 3000);
  rank.insert(103, 2000);
  rank.insert(104, 500);

  assert(rank.size() == 3);
  assert(rank.get_rank(102) == 1);
  assert(rank.get_rank(103) == 2);
  assert(rank.get_rank(101) == 3);
  assert(rank.get_rank(104) == 0);

  std::vector<int> keys;
  std::vector<int> values;
  for (const auto &item : rank) {
    keys.push_back(item->key);
    values.push_back(item->value);
  }

  assert((keys == std::vector<int>{102, 103, 101}));
  assert((values == std::vector<int>{3000, 2000, 1000}));
}

void test_update_existing_key() {
  SortedSet<int, int> rank(3);

  rank.insert(1, 100);
  rank.insert(2, 200);
  rank.insert(3, 300);
  rank.insert(1, 400);

  assert(rank.size() == 3);
  assert(rank.get_rank(1) == 1);
  assert(rank.get_rank(3) == 2);
  assert(rank.get_rank(2) == 3);

  const auto it = rank.find(1);
  assert(it != rank.map_end());
  assert(it->second->value == 400);
}

void test_erase_and_clear() {
  SortedSet<int, int> rank(3);

  rank.insert(1, 100);
  rank.insert(2, 200);
  rank.insert(3, 300);

  rank.erase(2);
  assert(rank.size() == 2);
  assert(rank.find(2) == rank.map_end());
  assert(rank.get_rank(3) == 1);
  assert(rank.get_rank(1) == 2);

  rank.erase(999);
  assert(rank.size() == 2);

  rank.clear();
  assert(rank.size() == 0);
  assert(rank.get_rank(1) == 0);
}

void test_string_keys_and_tie_breaker() {
  SortedSet<std::string, int> rank(3);

  rank.insert("alice", 100);
  rank.insert("bob", 100);
  rank.insert("carol", 200);

  assert(rank.size() == 3);
  assert(rank.get_rank("carol") == 1);
  assert(rank.get_rank("bob") == 2);
  assert(rank.get_rank("alice") == 3);
}

void test_move_constructor_and_assignment() {
  SortedSet<int, int> source(3);
  source.insert(1, 100);
  source.insert(2, 200);

  SortedSet<int, int> moved_constructed(std::move(source));
  assert(moved_constructed.size() == 2);
  assert(moved_constructed.get_rank(2) == 1);
  assert(moved_constructed.get_rank(1) == 2);

  SortedSet<int, int> moved_assigned(3);
  moved_assigned.insert(9, 900);
  moved_assigned = std::move(moved_constructed);

  assert(moved_assigned.size() == 2);
  assert(moved_assigned.get_rank(2) == 1);
  assert(moved_assigned.get_rank(1) == 2);
  assert(moved_assigned.get_rank(9) == 0);
}

}  // namespace

int main() {
  test_insert_keeps_descending_top_n();
  test_update_existing_key();
  test_erase_and_clear();
  test_string_keys_and_tie_breaker();
  test_move_constructor_and_assignment();

  std::cout << "All SortedSet example tests passed.\n";
  return 0;
}
