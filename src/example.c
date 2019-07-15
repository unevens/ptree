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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a point in a 3d space
typedef struct vec3 {
  float xyz[3];
} vec3;

static inline float inner_product(const float *a, const float *b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// the axis used to order the point
float axis[3] = {1.f, 0.f, 0.f};

// this is the function to compare the a key to an element of the tree,
// it is only used with ptree_get and ptree_get_it
// so it is only needed if you want to access the elements of the tree by a key
int key_cmp_vec3(const void *_key, const void *_rhs) {
  float *lhs_xyz = (float *)_key;
  vec3 *rhs = (vec3 *)_rhs;
  float vdiff[3];
  for (int i = 0; i < 3; ++i) {
    vdiff[i] = lhs_xyz[i] - rhs->xyz[i];
  }
  float diff = inner_product(vdiff, axis);
  return (diff > 0.f) - (diff < 0.f);
}

// this is the compare function used by the tree to order the elements during
// insertion and delation
int cmp_vec3(const void *_lhs, const void *_rhs) {
  return key_cmp_vec3(((vec3 *)_lhs)->xyz, _rhs);
}

// this will define typed aliases of the ptree api for pointers to vec3 and
// float* keys
DEFINE_TYPED_PTREE_OF(vec3, float)

#define NUM_POINTS 20

int main(int argc, char **argv) {

  // create the ptree, passing the cmp functions and preallocating NUM_POINTS
  // nodes
  ptree_of_vec3 *t = ptree_new__vec3(cmp_vec3, key_cmp_vec3, NUM_POINTS);

  // create the points, insert them in the ptree
  for (int i = 0; i < NUM_POINTS; ++i) {
    vec3 *v = malloc(sizeof *v);
    v->xyz[0] = sin(i * 0.8768f / NUM_POINTS);
    v->xyz[1] = sin(i * 0.6547f / NUM_POINTS);
    v->xyz[2] = sin(i * 0.8436f / NUM_POINTS);
    ptree_insert__vec3(t, v);
  }

  // get an iterator to the first element of the tree
  ptree_of_vec3_it *first = ptree_min__vec3(t);

  // get an iterator to the next element in the tree
  ptree_of_vec3_it *second = ptree_it_next__vec3(first);

  // get the element from the iterator
  vec3 *x = second->ptr;

  // remove an element from the tree, this invalidates the iterator
  ptree_remove__vec3(t, x);

  // get an iterator to the last element of the tree
  ptree_of_vec3_it *last = ptree_max__vec3(t);

  // search an element with a specific key in the tree (will be NULL if not such
  // element exists)
  float key[3] = {0.f, 0.f, 0.f};
  vec3 *elem = ptree_get__vec3(t, key);

  // remove an element with a given key from the tree 
  int elem_with_key_was_in_the_tree = ptree_remove_by_key__vec3(t, key);

  // search an element with a specific key in the tree and get an iterator to it
  // (will be NULL if not such element exists)
  ptree_of_vec3_it *elem_it = ptree_get_it__vec3(t, key);

  // remove the last element of the tree
  ptree_remove_by_it__vec3(t, ptree_max__vec3(t));

  // get the size of the tree
  int32_t size = ptree_size__vec3(t);

  // iterate the tree, starting from the beginning
  ptree_of_vec3_it *it = ptree_min__vec3(t);
  while (it) {
    // get the pointer from the iterator
    vec3 *v = it->ptr;
    // do something with it, for example print its position (and key)
    printf("%f %f %f\n", v->xyz[0], v->xyz[1], v->xyz[2]);
    // increment the iterator
    it = ptree_it_next__vec3(it);
  }
  printf("\n");

  // iterate the tree, starting from the end
  it = ptree_max__vec3(t);
  while (it) {
    // get the pointer from the iterator
    vec3 *v = it->ptr;
    // do something with it, for example print its position (and key)
    printf("%f %f %f\n", v->xyz[0], v->xyz[1], v->xyz[2]);
    // increment the iterator
    it = ptree_it_prev__vec3(it);
  }
  printf("\n");

  // let's clean up,
  // ptree does not manage its element, so to free them, you have to iterate the
  // tree
  it = ptree_min__vec3(t);
  while (it) {
    free(it->ptr);
    it = ptree_it_next__vec3(it);
  }

  // if we want to keep using the tree, adding new elements to it, empty it
  // (this will not deallocate the tree internal resources, but the tree will
  // behave as it was new
  ptree_empty__vec3(t);

  // or you can just free the tree
  ptree_free__vec3(t);
  t = NULL;

  getchar();
  getchar();
  return 0;
}