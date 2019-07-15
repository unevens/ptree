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

//#define PTREE_LEFT_LEANING 1
#include "ptree.h"

#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

struct simple_obj {
  int key;
};

int cmp_simple_obj(const void *lhs, const void *rhs) {
  return ((simple_obj *)lhs)->key - ((simple_obj *)rhs)->key;
}

struct cmp_simple_obj_cpp {
  bool operator()(const simple_obj *lhs, const simple_obj *rhs) const {
    return lhs->key < rhs->key;
  }
};

DEFINE_TYPED_PTREE_OF(simple_obj, void)

#define NUM_OBJS 10000000

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

int main() {

  cout << "this test inserts and remove the same objects from a ptree and a "
          "std::set"
       << endl;
  cout << "and check the ptree content against the content of the std::set."
       << endl
       << endl;

  cout << "creating " << NUM_OBJS
       << " simple objects with random keys to insert," << endl;


  random_int_generator rng = random_int_generator(NUM_OBJS);

  vector<simple_obj> objs;
  objs.reserve(NUM_OBJS);
  for (int i = 0; i < NUM_OBJS; ++i) {
    objs.push_back(simple_obj());
    objs.back().key = rng.next();
  }

  cout << "inserting the simple objects" << endl;

  ptree_of_simple_obj *t =
      ptree_new__simple_obj(cmp_simple_obj, NULL, NUM_OBJS);
  set<simple_obj *, cmp_simple_obj_cpp> s;

  for (int i = 0; i < NUM_OBJS; ++i) {
    s.insert(&objs[i]);
  }

  for (int i = 0; i < NUM_OBJS; ++i) {
    ptree_insert__simple_obj(t, &objs[i]);
  }

  vector<int> set_inorder;
  vector<int> tree_inorder;
  set_inorder.reserve(NUM_OBJS);
  tree_inorder.reserve(NUM_OBJS);

  for (auto *x : s) {
    set_inorder.push_back(x->key);
  }

  ptree_of_simple_obj_it *it = ptree_min__simple_obj(t);
  while (it) {
    tree_inorder.push_back(it->ptr->key);
    it = ptree_it_next__simple_obj(it);
  }

  cout << "checking coherence after insertion" << endl;
  cout << "size... " << endl;
  cout << "std::set " << s.size() << endl;
  cout << "ptree " << ptree_size__simple_obj(t) << endl;
  cout << ((s.size() == ptree_size__simple_obj(t)) ? "...is the same"
                                                   : "NOT the same!")
       << endl;

  assert(set_inorder.size() == tree_inorder.size());
  cout << "checking element by element (order)..." << endl;
  bool ok = true;
  for (int i = 0; i < set_inorder.size(); ++i) {
    if (tree_inorder[i] != set_inorder[i]) {
      cout << "order error!" << endl;
      for (int j = 0; j < set_inorder.size(); ++j) {
        cout << set_inorder[j] << " " << tree_inorder[j] << endl;
      }
      ok = false;
      break;
    }
  }
  if (ok) {
    cout << "...insertion is ok" << endl << endl;
  }

  cout << "creating " << NUM_OBJS
       << " simple objects with random keys to insert," << endl;
  objs.clear();
  
  for (int i = 0; i < NUM_OBJS; ++i) {
    objs.push_back(simple_obj());
    objs.back().key = rng.next();
  }

  cout << "removing the simple objects" << endl;

  for (int i = 0; i < NUM_OBJS; ++i) {
    s.erase(&objs[i]);
  }

  for (int i = 0; i < NUM_OBJS; ++i) {
    ptree_remove__simple_obj(t, &objs[i]);
  }

  std::cout << "checking coherence after delation" << endl;
  cout << "size... " << endl;
  cout << "std::set " << s.size() << endl;
  cout << "ptree " << ptree_size__simple_obj(t) << endl;
  cout << ((s.size() == ptree_size__simple_obj(t)) ? "...is the same"
                                                   : "NOT the same!")
       << endl;

  assert(set_inorder.size() == tree_inorder.size());
  cout << "checking element by element (order)..." << endl;
  ok = true;
  for (int i = 0; i < set_inorder.size(); ++i) {
    if (tree_inorder[i] != set_inorder[i]) {
      cout << "order error!" << endl;
      for (int j = 0; j < set_inorder.size(); ++j) {
        cout << set_inorder[j] << " " << tree_inorder[j] << endl;
      }
      ok = false;
      break;
    }
  }
  if (ok) {
    cout << "...delation is ok" << endl << endl;
  }

  cout << "test completed" << endl;

  cin.get();
  return 0;
}
