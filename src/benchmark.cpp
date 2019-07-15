/*
ptree - red black tree of pointers in C
by Dario Mambro @ https://github.com/unevens/ptree
*/

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

//#define PTREE_STORAGE_64BIT 1
#include "ptree.h"

#include <algorithm>
#include <assert.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

struct simple_obj {
  int key;
};

int key_cmp_simple_obj(const void *key, const void *rhs) {
  return *((int *)key) - ((simple_obj *)rhs)->key;
}

int cmp_simple_obj(const void *lhs, const void *rhs) {
  return ((simple_obj *)lhs)->key - ((simple_obj *)rhs)->key;
}

struct cmp_simple_obj_cpp {
  bool operator()(const simple_obj *lhs, const simple_obj *rhs) const {
    return lhs->key < rhs->key;
  }
};

DEFINE_TYPED_PTREE_OF(simple_obj, int)

class random_int_generator {
  uniform_int_distribution<int> uniform;
  mt19937 rng;

public:
  random_int_generator(int max) {
    random_device rd;
    rng = mt19937(rd());
    uniform = uniform_int_distribution<int>(0, max);
  }
  int next() { return uniform(rng); }
};

class profiler {

  vector<long long> data;
  double mean, deviation, _max, _min;
  std::chrono::high_resolution_clock::time_point start_time;
  string name;

public:
  profiler(string name)
      : name(name), mean(0.0), _max(-1.0), _min(1E20), deviation(0.0) {}

  void start() { start_time = chrono::high_resolution_clock::now(); }

  void end() {
    auto end_time = chrono::high_resolution_clock::now();
    auto delta =
        chrono::duration_cast<chrono::nanoseconds>(end_time - start_time)
            .count();
    data.push_back(delta);
  }

  void compute() {
    double inv = 1. / (double)data.size();

    for (auto time : data) {
      mean += inv * time;
      _max = max(_max, (double)time);
      _min = min(_min, (double)time);
    }
    deviation = 0.;
    for (auto time : data) {
      double x = mean - (double)time;
      deviation += inv * x * x;
    }
    deviation = sqrt(deviation);
  }

  void print() const {
    cout << name << " mean: " << mean << " | var: " << deviation
         << " | min: " << _min << " | max: " << _max << endl
         << endl;
  }

  double get_mean() const { return mean; }
  double get_deviation() const { return deviation; }
  double get_variation_coef() const { return deviation / mean; }
};

void print_ratio(const string &name, const profiler &tree_profiler,
                 const profiler &set_profiler) {
  auto insert_ratio = 100. * tree_profiler.get_mean() / set_profiler.get_mean();

  cout << name << " time of ptree = " << (int)insert_ratio
       << "% of std::set's ";
  cout << scientific << set_profiler.get_mean() / 1E9 << " s";
  cout << " | std::set variation coef = "
       << (int)(100. * set_profiler.get_variation_coef())
       << "% | ptree variation coef = "
       << (int)(100. * tree_profiler.get_variation_coef()) << "%" << endl
       << endl;
}

void test(int num_elements, int iterations, bool preallocate,
          bool details) {

  profiler set_insert = profiler("std::set insert");
  profiler tree_insert = profiler("ptree insert");
  profiler set_remove = profiler("std::set remove");
  profiler tree_remove = profiler("ptree remove");
  profiler set_loop = profiler("std::set loop");
  profiler tree_loop = profiler("ptree loop");
  profiler set_access = profiler("std::set access");
  profiler tree_access = profiler("ptree access");

  int randomness = 3;
  random_int_generator rng = random_int_generator(randomness * num_elements);

  for (int n = 0; n < iterations; ++n) {
    vector<simple_obj> objs;
    objs.reserve(num_elements);
    for (int i = 0; i < num_elements; ++i) {
      objs.push_back(simple_obj());
      objs.back().key = rng.next();
    }

    int nodes_to_preallocate = preallocate ? num_elements : 0;

    auto t = ptree_new__simple_obj(cmp_simple_obj, key_cmp_simple_obj,
                                   nodes_to_preallocate);

    set<simple_obj *, cmp_simple_obj_cpp> s;

    set_insert.start();

    for (int i = 0; i < num_elements; ++i) {
      s.insert(&objs[i]);
    }

    set_insert.end();

    tree_insert.start();

    for (int i = 0; i < num_elements; ++i) {
      ptree_insert__simple_obj(t, &objs[i]);
    }

    tree_insert.end();

    vector<int> set_inorder;
    vector<int> tree_inorder;
    set_inorder.reserve(num_elements);
    tree_inorder.reserve(num_elements);

    for (auto *x : s) {
      set_inorder.push_back(x->key);
    }

    ptree_of_simple_obj_it *it = ptree_min__simple_obj(t);
    while (it) {
      tree_inorder.push_back(it->ptr->key);
      it = ptree_it_next__simple_obj(it);
    }

    int acc_tree = 0;

    tree_access.start();

    for (int i = 0; i < num_elements; ++i) {
      auto x = ptree_get__simple_obj(t, &objs[i].key);
      if (x) {
        acc_tree += x->key;
      }
    }

    tree_access.end();

    int acc_set = 0;

    set_access.start();

    for (int i = 0; i < num_elements; ++i) {
      auto x = s.find(&objs[i]);
      if (x != s.end()) {
        acc_set += (*x)->key;
      }
    }

    set_access.end();

    tree_loop.start();

    it = ptree_min__simple_obj(t);
    while (it) {
      acc_tree += it->ptr->key;
      it = ptree_it_next__simple_obj(it);
    }

    tree_loop.end();

    set_loop.start();

    for (auto *x : s) {
      acc_set += x->key;
    }

    set_loop.end();

    if (acc_set != acc_tree) {
      fprintf(stderr, "Coeherence Error\n");
    }

    tree_remove.start();

    for (int i = 0; i < num_elements; ++i) {
      ptree_remove__simple_obj(t, &objs[i]);
    }

    tree_remove.end();

    set_remove.start();

    for (int i = 0; i < num_elements; ++i) {
      s.erase(&objs[i]);
    }

    set_remove.end();

	ptree_free__simple_obj(t);
  }

  cout << "----------------------------------------" << endl;
  cout << num_elements << " random elements and " << iterations
       << " measurements:" << endl
       << endl;

  tree_insert.compute();
  set_insert.compute();
  tree_remove.compute();
  set_remove.compute();
  tree_loop.compute();
  set_loop.compute();
  tree_access.compute();
  set_access.compute();

  cout << endl;

  print_ratio("INSERT", tree_insert, set_insert);
  print_ratio("REMOVE", tree_remove, set_remove);
  print_ratio("LOOP  ", tree_loop, set_loop);
  print_ratio("ACCESS", tree_access, set_access);

  if (details) {
    cout << "DETAILS:" << endl << endl;
    tree_insert.print();
    set_insert.print();
    tree_remove.print();
    set_remove.print();
    tree_loop.print();
    set_loop.print();
    tree_access.print();
    set_access.print();
  }
}

int main(int argc, char *argv[]) {
  cout.precision(2);
  bool details = false;
  if (argc > 1) {
    if (strcmp("-v", argv[1]) == 0) {
      details = true;
    }
  }

  cout << "ptree benchmark program start" << endl << endl;

  for (int preallocate = 1; preallocate > -1; --preallocate) {
    cout << "========================================" << endl;
    cout << "ptree preallocation is: " << (preallocate ? "ON" : "OFF") << endl;
    cout << "========================================" << endl << endl << endl;
    for (int i = 1; i <= 6; ++i) {
      int num_elements = pow(10, i);
      int num_measurements = pow(10, 7 - i);
      test(num_elements, num_measurements, preallocate, details);
	}
    cout << endl;
  }

  cout << "ptree benchmark program end" << endl;

  cin.get();

  return 0;
}
