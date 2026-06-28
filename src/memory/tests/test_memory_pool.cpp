//
// Created by sky on 2026/6/24.
//

// ======== 应用示例 ========

#include "memory_pool/memory_pool.h"

#include <iostream>
#include <ostream>

struct Particle {
  float x, y, z;
  int lifetime;

  static void *operator new(size_t size);
  static void operator delete(void *ptr);

  void update() { ++lifetime; }
};

static FixedSizePool g_particle_pool(sizeof(Particle), 4096);

void *Particle::operator new(size_t size) {
  return g_particle_pool.allocate();
}

void Particle::operator delete(void *ptr) noexcept {
  g_particle_pool.deallocate(ptr);
}

int main() {
  std::vector<Particle *> particles;
  particles.reserve(10000);

  for (int i = 0; i < 10000; ++i) {
    Particle *p = new Particle{0, 0, 0};
    particles.push_back(p);
  }

  // 一些业务 ......

  for (auto *p : particles) {
    delete p;
  }

  std::cout << "BlockSize = " << g_particle_pool.get_block_size()
            << ", BlocksPerPage = " << g_particle_pool.get_blocks_per_page() << std::endl;
}
