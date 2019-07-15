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

#include "ptree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memset

typedef int bool;
#define false 0
#define true 1

#define oom() abort()

#if (PTREE_STORAGE_64BIT == 1)
typedef uint64_t ptree_size_int;
#else
typedef uint32_t ptree_size_int;
#endif

/******************************************************
 * structs
 ******************************************************/

typedef struct ptree_node {
  void *ptr;
  struct ptree_node *links[2];
  struct ptree_node *parent;
  ptree_size_int flags;
} ptree_node;

struct ptree {
  ptree_node *root;
  ptree_size_int nodes_num;
  ptree_size_int allocated_nodes_num;
  ptree_node **nodes;
  ptree_cmp_fptr cmp;
  ptree_cmp_fptr cmp_key;
};

/******************************************************
 * node flags
 ******************************************************/

#if (PTREE_STORAGE_64BIT == 1)
#define red_flag 0x8000000000000000
const size_t max_nodes = 9223372036854775807; //(2<<63)-1
#else
#define red_flag 0x80000000
const size_t max_nodes = 2147483647; //(2<<31)-1
#endif

static ptree_node _leaf = {
    .ptr = NULL, .links = {NULL, NULL}, .parent = NULL, .flags = 0};
#define leaf &_leaf

#define is_red(node) (((node)->flags & red_flag) != 0)
#define is_black(node) (((node)->flags & red_flag) == 0)

#define paint_red(node) ((node)->flags |= red_flag)
#define paint_black(node) ((node)->flags &= ~red_flag)

#define has_child(node, dir) (node->links[dir] != leaf)
#define is_child(node, dir) (node->parent->links[dir] == node)

#define get_node_index(node) ((node)->flags & ~red_flag)
#define set_node_index(node, index)                                            \
  ((node)->flags = index | ((node)->flags & red_flag))

inline static void copy_color(ptree_node *dst, ptree_node *src) {
  if (is_red(src)) {
    paint_red(dst);
  } else {
    paint_black(dst);
  }
}

/******************************************************
 * itearation
 ******************************************************/

static inline ptree_node *get_next_node(ptree_node *node) {
  assert(node && node != leaf);
  if (node->links[1] != leaf) {
    node = node->links[1];
    while (node->links[0] != leaf) {
      node = node->links[0];
    }
    return node;
  } else {
    ptree_node *it = node->parent;
    while (it != leaf && node == it->links[1]) {
      node = it;
      it = it->parent;
    }
    return it != leaf ? it : NULL;
  }
}
static inline ptree_node *get_prev_node(ptree_node *node) {
  assert(node && node != leaf);
  if (node->links[0] != leaf) {
    node = node->links[0];
    while (node->links[1] != leaf) {
      node = node->links[1];
    }
    return node;
  } else {
    ptree_node *it = node->parent;
    while (it != leaf && node == it->links[0]) {
      node = it;
      it = it->parent;
    }
    return it != leaf ? it : NULL;
  }
}

ptree_it *ptree_it_next(ptree_it *node) {
  return (ptree_it *)get_next_node((ptree_node *)node);
}

ptree_it *ptree_it_prev(ptree_it *node) {
  return (ptree_it *)get_prev_node((ptree_node *)node);
}

/******************************************************
 * nodes management
 ******************************************************/

static size_t max_nodes_to_auto_allocate = 0;

size_t ptree_get_max_nodes_to_auto_allocate(void) {
  return max_nodes_to_auto_allocate;
}

void ptree_set_max_nodes_to_auto_allocate(size_t num) {
  max_nodes_to_auto_allocate = num;
}

void ptree_allocate_nodes(ptree *tree, size_t num_nodes) {
  ptree_size_int nodes_to_reallocate = tree->allocated_nodes_num + num_nodes;
  if (nodes_to_reallocate > max_nodes) {
    oom();
  }
  ptree_node **nodes =
      realloc(tree->nodes, nodes_to_reallocate * sizeof(ptree_node *));
  if (!nodes) {
    oom();
  }
  tree->nodes = nodes;
  for (ptree_size_int i = tree->allocated_nodes_num; i < nodes_to_reallocate;
       ++i) {
    tree->nodes[i] = malloc(sizeof(ptree_node));
    if (!tree->nodes[i]) {
      oom();
    }
    memset(tree->nodes[i], 0, sizeof(ptree_node));
    set_node_index(tree->nodes[i], i);
    paint_black(tree->nodes[i]);
  }
  tree->allocated_nodes_num = nodes_to_reallocate;
}

static ptree_node *add_node(ptree *tree, void *ptr) {
  if (tree->nodes_num >= tree->allocated_nodes_num) {
    ptree_size_int nodes_to_allocate =
        tree->allocated_nodes_num > 1 ? tree->allocated_nodes_num : 1;
    if (max_nodes_to_auto_allocate &&
        tree->allocated_nodes_num > max_nodes_to_auto_allocate) {
      nodes_to_allocate = max_nodes_to_auto_allocate;
    }
    ptree_allocate_nodes(tree, nodes_to_allocate);
  }
  ptree_node *node = tree->nodes[tree->nodes_num];
  ++(tree->nodes_num);
  node->ptr = ptr;
  paint_black(node);
  paint_red(node);
  node->parent = leaf;
  node->links[0] = leaf;
  node->links[1] = leaf;
  return node;
}

static void release_node(ptree *tree, ptree_node *node) {
  --(tree->nodes_num);
  ptree_node **last_ptr = tree->nodes + tree->nodes_num;
  int32_t node_index = get_node_index(node);
  set_node_index(*last_ptr, node_index);
  set_node_index(node, tree->nodes_num);
  tree->nodes[node_index] = *last_ptr;
  *last_ptr = node;
}

void ptree_shrink(ptree *tree) {
  for (ptree_size_int i = tree->nodes_num; i < tree->allocated_nodes_num; ++i) {
    free(tree->nodes[i]);
  }
  ptree_node **nodes =
      realloc(tree->nodes, tree->nodes_num * sizeof(ptree_node *));
  if (!nodes) {
    oom();
  }
  tree->nodes = nodes;
}

/******************************************************
 * ptree management
 ******************************************************/

ptree *ptree_new(ptree_cmp_fptr cmp_elem, ptree_cmp_fptr cmp_key,
                 int32_t preallocated_nodes) {
  ptree *tree = malloc(sizeof *tree);
  if (!tree) {
    oom();
  }
  memset(tree, 0, sizeof *tree);
  tree->nodes = NULL;
  tree->root = leaf;
  tree->cmp = cmp_elem;
  tree->cmp_key = cmp_key;
  ptree_allocate_nodes(tree, preallocated_nodes);
  return tree;
}

void ptree_free(ptree *tree) {
  for (ptree_size_int i = 0; i < tree->allocated_nodes_num; ++i) {
    free(tree->nodes[i]);
  }
  free(tree->nodes);
  free(tree);
}

void ptree_empty(ptree *tree) {
  tree->root = leaf;
  tree->nodes_num = 0;
}

/******************************************************
 * getters
 ******************************************************/

ptree_it *ptree_get_it(const ptree *tree, const void *key) {
  ptree_node *it = tree->root;
  while (it != leaf) {
    int diff = tree->cmp_key(key, it->ptr);
    if (diff == 0) {
      return (ptree_it *)it;
    }
    int dir = diff > 0;
    if (has_child(it, dir)) {
      it = it->links[dir];
    } else {
      return NULL;
    }
  }
  return NULL;
}

void *ptree_get(const ptree *tree, const void *key) {
  ptree_it *it = ptree_get_it(tree, key);
  if (it) {
    return it->ptr;
  }
  return NULL;
}

static ptree_node *ptree_search(const ptree *tree, const void *ptr) {
  if (tree->root == leaf) {
    return NULL;
  }
  ptree_node *z = tree->root;
  while (z != leaf) {
    int diff = tree->cmp(ptr, z->ptr);
    if (diff == 0) {
      break;
    }
    int dir = diff > 0;
    if (has_child(z, dir)) {
      z = z->links[dir];
    } else {
      return NULL;
    }
  }
  return z;
}

ptree_it *ptree_has(const ptree *tree, const void *ptr) {
  ptree_node *node = ptree_search(tree, ptr);
  if (!node) {
    return NULL;
  }
  return (ptree_it *)node;
}

ptree_it *ptree_min(ptree *tree) {
  if (tree->root == leaf) {
    return NULL;
  }
  ptree_node *it = tree->root;
  while (has_child(it, 0)) {
    it = it->links[0];
  }
  return (ptree_it *)it;
}

ptree_it *ptree_max(ptree *tree) {
  if (tree->root == leaf) {
    return NULL;
  }
  ptree_node *it = tree->root;
  while (has_child(it, 1)) {
    it = it->links[1];
  }
  return (ptree_it *)it;
}

int32_t ptree_size(const ptree *tree) { return tree->nodes_num; }

static void rotate(ptree *tree, ptree_node *x, int dir) {
  assert(has_child(x, !dir));
  ptree_node *y = x->links[!dir];
  x->links[!dir] = y->links[dir];
  if (y->links[dir] != leaf) {
    y->links[dir]->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == leaf) {
    tree->root = y;
  } else {
    if (x == x->parent->links[0]) {
      x->parent->links[0] = y;
    } else {
      x->parent->links[1] = y;
    }
  }
  y->links[dir] = x;
  x->parent = y;
}

bool ptree_insert(ptree *tree, void *ptr) {
  if (tree->root == leaf) {
    tree->root = add_node(tree, ptr);
    paint_black(tree->root);
    return true;
  }
  // insertion
  ptree_node *x = tree->root;
  while (true) {
    int cmp = tree->cmp(ptr, x->ptr);
    if (cmp == 0) {
      return false;
    } else {
      int dir = cmp > 0;
      if (has_child(x, dir)) {
        x = x->links[dir];
      } else {
        x->links[dir] = add_node(tree, ptr);
        x->links[dir]->parent = x;
        x = x->links[dir];
        break;
      }
    }
  }
  // keep tree balanced
  while (x != tree->root && is_red(x->parent)) {
    bool lefty = is_child(x->parent, 0);
    ptree_node *y = x->parent->parent->links[lefty];
    if (is_red(y)) {
      paint_black(x->parent);
      paint_black(y);
      paint_red(x->parent->parent);
      x = x->parent->parent;
    } else {
      if (is_child(x, lefty)) {
        x = x->parent;
        rotate(tree, x, !lefty);
      }
      paint_black(x->parent);
      paint_red(x->parent->parent);
      rotate(tree, x->parent->parent, lefty);
    }
  }
  paint_black(tree->root);
  return true;
}

static bool ptree_remove_node(ptree *tree, ptree_node *z) {
  ptree_node *y;
  if (!has_child(z, 0) || !has_child(z, 1)) {
    y = z;
  } else {
    y = get_next_node(z);
  }
  ptree_node *x = y->links[!has_child(y, 0)];
  x->parent = y->parent;
  if (y->parent == leaf) {
    tree->root = x;
  } else {
    y->parent->links[is_child(y, 1)] = x;
  }
  if (y != z) {
    z->ptr = y->ptr;
  }
  // keep tree balanced
  if (is_black(y)) {
    while (x != tree->root && is_black(x)) {
      bool XL = is_child(x, 0);
      ptree_node *w = x->parent->links[XL];
      assert(w != leaf);
      if (is_red(w)) {
        paint_black(w);
        paint_red(x->parent);
        rotate(tree, x->parent, !XL);
        w = x->parent->links[XL];
        assert(w != leaf);
      }
      if (is_black(w->links[0]) && is_black(w->links[1])) {
        paint_red(w);
        x = x->parent;
      } else {
        if (is_black(w->links[XL])) {
          paint_black(w->links[!XL]);
          paint_red(w);
          rotate(tree, w, XL);
          w = x->parent->links[XL];
          assert(w != leaf);
        }
        copy_color(w, x->parent);
        paint_black(x->parent);
        paint_black(w->links[XL]);
        rotate(tree, x->parent, !XL);
        break;
      }
    }
  }
  paint_black(x);
  release_node(tree, y);
  return true;
}

bool ptree_remove(ptree *tree, const void *ptr) {
  if (tree->root == leaf) {
    return false;
  }
  ptree_node *z = ptree_search(tree, ptr);
  if (!z) {
    return false;
  }
  return ptree_remove_node(tree, z);
}

void ptree_remove_by_it(ptree *tree, ptree_it *it) {
  ptree_remove_node(tree, (ptree_node *)it);
}

bool ptree_remove_by_key(ptree *tree, void *key) {
  ptree_it *it = ptree_get_it(tree, key);
  if (it) {
    ptree_remove_node(tree, (ptree_node *)it);
    return true;
  }
  return false;
}