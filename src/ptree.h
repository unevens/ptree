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

#pragma once

#include <stddef.h>
#include <stdint.h>

// define this macro to 1 if you need to store more than 2^31 elements in a tree
#ifndef PTREE_STORAGE_64BIT
#define PTREE_STORAGE_64BIT 0
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// the tree struct
typedef struct ptree ptree;

// the iterator struct.
typedef struct ptree_it {
  void *ptr;
} ptree_it;

// the type for the ordering functions
typedef int (*ptree_cmp_fptr)(const void *a, const void *b);

// creates a tree. `cmp_elem` is the ordering function, `cmp_key` is the
// optional function to use keys, `preallocated_nodes` is the number of elements
// to preallocate memory for
ptree *ptree_new(ptree_cmp_fptr cmp_elem, ptree_cmp_fptr cmp_key,
                 int32_t preallocated_nodes);

// frees a tree
void ptree_free(ptree *tree);

// drops all elements, but keeps the allocated space
void ptree_empty(ptree *tree);

// free unused memory
void ptree_shrink(ptree *tree);

// insert an element in the tree and returns 1 if ptr was not already in the
// tree, 0 if it was already there
int ptree_insert(ptree *tree, void *ptr);

// removes an element from the tree, and returns 1 if it was removed, 0 if it
// was not contained in tree to begin with
int ptree_remove(ptree *tree, const void *ptr);

// searches the tree for an element with the given key, removes it from the tree
// and returns 1 if the element exists, else returns 0
int ptree_remove_by_key(ptree *tree, void *key);

// removes from the tree the element corresponding to the iterator it
void ptree_remove_by_it(ptree *tree, ptree_it *it);

// returns an iterator to the inorder minimum element of the tree
ptree_it *ptree_min(ptree *tree);

// returns an iterator to the inorder maximum element of the tree
ptree_it *ptree_max(ptree *tree);

// increment and iterator
ptree_it *ptree_it_next(ptree_it *it);

// decremente an iterator
ptree_it *ptree_it_prev(ptree_it *it);

// searches the tree for the given element, and returns and iterator to it if it
// exists, else it returns NULL
ptree_it *ptree_has(const ptree *tree, const void *ptr);

// searches the tree for an element with the given tree, and returns it it
// exists, else it returns NULL
void *ptree_get(const ptree *tree, const void *key);

// searches the tree for an element with the given tree, and returns an iterator
// ot it if it exists, else it returns NULL
ptree_it *ptree_get_it(const ptree *tree, const void *key);

// returns the number of elements in the tree
int32_t ptree_size(const ptree *tree);

// allocates memory to store num_nodes more elements in the tree
void ptree_allocate_nodes(ptree *tree, size_t num_nodes);

// set an upper bound the number of nodes that a tree can allocate during a
// single call to ptree_insert. 0 means that there is no upper bound.
void ptree_set_max_nodes_to_auto_allocate(size_t num);

// get the maximum number of nodes that a tree can allocate during a
// single call to ptree_insert. 0 means that there is no maximum number.
size_t ptree_get_max_nodes_to_auto_allocate(void);

/******************************************************
 * macro to define strictly typed APIs
 ******************************************************/

#define DEFINE_TYPED_PTREE_OF(type, key_type)                                  \
  typedef struct ptree_of_##type {                                             \
    ptree_it *root;                                                            \
  } ptree_of_##type;                                                           \
  typedef struct ptree_of_##type##_it {                                        \
    type *ptr;                                                                 \
  } ptree_of_##type##_it;                                                      \
  static inline ptree_of_##type *ptree_new__##type(                            \
      ptree_cmp_fptr cmp_elem, ptree_cmp_fptr cmp_key,                         \
      int32_t preallocated_nodes) {                                            \
    return (ptree_of_##type *)ptree_new(cmp_elem, cmp_key,                     \
                                        preallocated_nodes);                   \
  }                                                                            \
  static inline void ptree_free__##type(ptree_of_##type *tree) {               \
    ptree_free((ptree *)tree);                                                 \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_min__##type(                       \
      ptree_of_##type *tree) {                                                 \
    return (ptree_of_##type##_it *)ptree_min((ptree *)tree);                   \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_max__##type(                       \
      ptree_of_##type *tree) {                                                 \
    return (ptree_of_##type##_it *)ptree_max((ptree *)tree);                   \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_it_next__##type(                   \
      ptree_of_##type##_it *it) {                                              \
    return (ptree_of_##type##_it *)ptree_it_next((ptree_it *)it);              \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_it_prev__##type(                   \
      ptree_of_##type##_it *it) {                                              \
    return (ptree_of_##type##_it *)ptree_it_prev((ptree_it *)it);              \
  }                                                                            \
  static inline int ptree_insert__##type(ptree_of_##type *tree, type *ptr) {   \
    return ptree_insert((ptree *)tree, ptr);                                   \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_has__##type(                       \
      const ptree_of_##type *tree, const type *ptr) {                          \
    return (ptree_of_##type##_it *)ptree_has((const ptree *)tree, ptr);        \
  }                                                                            \
  static inline type *ptree_get__##type(const ptree_of_##type *tree,           \
                                        const key_type *key) {                 \
    return (type *)ptree_get((const ptree *)tree, key);                        \
  }                                                                            \
  static inline ptree_of_##type##_it *ptree_get_it__##type(                    \
      const ptree_of_##type *tree, const key_type *key) {                      \
    return (ptree_of_##type##_it *)ptree_get_it((const ptree *)tree, key);     \
  }                                                                            \
  static inline void ptree_empty__##type(ptree_of_##type *tree) {              \
    ptree_empty((ptree *)tree);                                                \
  }                                                                            \
  static inline int ptree_remove__##type(ptree_of_##type *tree,                \
                                         const type *ptr) {                    \
    return ptree_remove((ptree *)tree, ptr);                                   \
  }                                                                            \
  static inline int ptree_remove_by_key__##type(ptree_of_##type *tree,         \
                                                key_type *key) {               \
    return ptree_remove_by_key((ptree *)tree, key);                            \
  }                                                                            \
  static inline void ptree_remove_by_it__##type(ptree_of_##type *tree,         \
                                                ptree_of_##type##_it *it) {    \
    return ptree_remove_by_it((ptree *)tree, (ptree_it *)it);                  \
  }                                                                            \
  static inline int32_t ptree_size__##type(const ptree_of_##type *tree) {      \
    return ptree_size((const ptree *)tree);                                    \
  }                                                                            \
  static inline void ptree_allocate_nodes__##type(ptree_of_##type *tree,       \
                                                  int32_t num_nodes) {         \
    ptree_allocate_nodes((ptree *)tree, num_nodes);                            \
  }                                                                            \
  static inline void ptree_shrink__##type(ptree_of_##type *tree) {             \
    ptree_shrink((ptree *)tree);                                               \
  }

#if defined(__cplusplus)
}
#endif