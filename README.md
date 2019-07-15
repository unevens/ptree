# ptree

ptree is a red-black tree of pointers written in C. 

# Wait, what? 

In practice, a ptree is a __container data structure
that can be used as an ordered set of pointers__, like a
`std::set<your_struct *, your_ordering_functor>` in C++.

Let's say you have a struct

```c
typedef struct your_struct your_struct;
```

and a `comparision-function` to order instances of `your_struct` 

```c
int cmp(void *lhs, void *rhs);
```

that behaves like `memcmp` and `strcmp`, meaning that it will return 

- 0 if `lhs` and `rhs` are to be considered equal,
- something less than 0, if `lhs` is to be conidererd less than `rhs`
- something greater than 0 if `lhs` is to be conidererd greater than `rhs`

Then you can create a ptree for elements of type `your_struct` with

```c
ptree *tree = ptree_new(cmp, NULL, 0);
```

You can insert and remove elements from `tree` with

```c
your_struct *elem = create_your_struct(/*...*/);
ptree_insert(tree, elem);
ptree_remove(tree, elem);
```

You can iterate trough the tree (in-order traversal) with

```c
ptree_it *it = ptree_min(tree);
while(it)
{
    your_struct *elem = it->ptr;
    /*...*/
    it = ptree_it_next(it);
}
```

For a better explanation, see the file `src/example.c`. 

# A ptree can also be queried like a map

Let's say your struct contains a `key` field that you want to use to identify and order its instances. For example, `your_struct` looks like 

```c
typedef struct your_struct{
    //...
    int key;
    //...
} your_struct;
```

and the `comparision-function` is 

```c
int cmp(void *lhs,void *rhs){
    return ((your_struct *)lhs)->key - ((your_struct *)rhs)->key;
}
```

Then you can use an element's key to get the element from a ptree, if it is contained in it.
In order to do it, you have to define a `key-comparison-function`, that takes a pointer to a key and a pointer to a `your_struct`, and compares the key to the key contained in the struct

```c
int key_cmp(void *key,void *rhs){
    return (int *)key - ((your_struct *)rhs)->key;
}
```

So, when you call `ptree_new` pass both `cmp` and `key_cmp` to it,

```c
ptree *t = ptree_new(cmp, key_cmp, 0);
```

and now you can also query the elements of `t` by their keys, using `ptree_get`.

```c
int key = 7;
your_struct *x = ptree_get(tree, key);
if(x){/*...*/}
```

__Remark__: `key_cmp` must induce the same ordering as `cmp`, meaning that

```c
key_cmp(elem_A->key, elem_B) == cmp(elem_A, elem_B)
```

An easy way to do it, is to define `cmp` on top of `key_cmp`

```c
int cmp(void *lhs,void *rhs){
    return key_cmp((your_struct *)lhs)->key, rhs);
}
```

# Memory recycling

Each ptree recycles the memory it allocates for its nodes. If you remove an element from a ptree, or call `ptree_empty`, which removes all elements from it, no memory will be freed. 

If you then add elements to that ptree, the unused memory will be used to store them. 

If you want a ptree to free its unused memory, call `ptree_shrink`. 

The third argument of the function `ptree_new` is the number of elements to preallocate space for. 

There is also the function `ptree_allocate_nodes` that preallocates space for future insertions.

# But I don't like using void * 

Me neither. 

The functions `ptree_insert`, `ptree_remove`, `ptree_remove_by_key`, `ptree_get`, `ptree_get_it` and  `ptree_has` use `void *` to handle generic types. Their signatures are:

```c
int ptree_insert(ptree *tree, void *ptr);
int ptree_remove(ptree *tree, const void *ptr);
int ptree_remove_by_key(ptree *tree, void *key);
ptree_it *ptree_has(const ptree *tree, const void *ptr);
void *ptree_get(const ptree *tree, const void *key);
ptree_it *ptree_get_it(const ptree *tree, const void *key);
```

If you want ptree to be strictly typed, you can use the macro 

```c
DEFINE_TYPED_PTREE_OF(your_struct, your_key_type)
```

which implements the types `ptree_of_<your_struct>` and `ptree_of_<your_struct>_it`,
and strictly typed inlined aliases of all the ptree api calls (aka without `void *`, and using `ptree_of_<your_struct>` and `ptree_of_<your_struct>_it` instead of `ptree` and `ptree_it`).
For the functions above you will get these aliases:

```c
int ptree_insert__your_struct(ptree_of_your_struct *tree, your_struct *ptr);
int ptree_remove__your_struct(ptree_of_your_struct *tree, const void *ptr);
int ptree_remove_by_key__your_struct(ptree_of_your_struct *tree, your_key_type *key);
ptree_of_your_struct_it *ptree_has__your_struct(const ptree_of_your_struct *tree, const your_struct *ptr);
your_struct *ptree_get__your_struct(const ptree_of_your_struct *tree, const your_key_type *key);
ptree_of_your_struct_it *ptree_get_it__your_struct(const ptree_of_your_struct *tree, const your_key_type *key);
```

Note: The `cmp` and `key_cmp` function that you pass to `ptree_new__<your_struct>`
are still expected to be `int(void *, void *)`, as `strcmp` and `memcmp`.

So you can do

```c
DEFINE_TYPED_PTREE_OF(your_struct, your_key_type)

ptree_of_your_struct *tree = ptree_new__your_struct(cmp, key_cmp, 0);
your_struct *elem = create_your_struct(/*...*/);
ptree_insert__your_struct(tree, elem);
ptree_remove__your_struct(tree, elem);

int key = 7;
your_struct *x = ptree_get__your_struct(tree, key);
if(x){/*...*/}

ptree_of_your_struct_it *it = ptree_min__your_struct(tree);
while(it)
{
    your_struct *elem = it->ptr;
    /*...*/
    it = ptree_it_next__your_struct(it);
}
```

Then again, for a tutorial, see the file `src/example.c`. 

# Implementation notes

The maximum number of elements in a ptree, is 2^31. If you define the macro `PTREE_STORAGE_64BIT` to `1`, it becomes 2^63.   

ptree does not use recursion.

ptree uses parent pointers.

# Create the example, test and benchmark executables

Run

```bash
$ git clone https://github.com/unevens/ptree.git
$ cd ptree
$ mkdir build
$ cd build
$ cmake ..
```

Then run `make` on Linux, or open the generated project in your IDE on Win/Mac.

# Performance

Performance is close to std::set in my benchmarks.
You can test it on your system with the `ptree-bench` executable. 

Here are the results of a couple of benchmarks on Debian and Windows.

__TLDR__: using preallocation and with less than 1 million elements, ptree is generally faster, with the notable exception of iterating over the tree in Windows, where std::set is already faster at 10000 elements. Without preallocation, std::set is a bit faster at inserting elements.

## Windows, 64 bit, Visual Studio 2019, Intel i5-6600K, ptree preallocation is: ON

-------------------------
10 random elements and 1000000 measurements:


INSERT time of ptree = 41% of std::set's 6.09e-07 s | std::set variation coef = 131% | ptree variation coef = 376%

REMOVE time of ptree = 57% of std::set's 5.10e-07 s | std::set variation coef = 75% | ptree variation coef = 1628%

LOOP   time of ptree = 99% of std::set's 7.65e-08 s | std::set variation coef = 158% | ptree variation coef = 253%

ACCESS time of ptree = 71% of std::set's 2.34e-07 s | std::set variation coef = 342% | ptree variation coef = 109%

----------------------------------------
100 random elements and 100000 measurements:


INSERT time of ptree = 46% of std::set's 9.72e-06 s | std::set variation coef = 39% | ptree variation coef = 15%

REMOVE time of ptree = 47% of std::set's 9.22e-06 s | std::set variation coef = 19% | ptree variation coef = 21%

LOOP   time of ptree = 97% of std::set's 6.17e-07 s | std::set variation coef = 59% | ptree variation coef = 59%

ACCESS time of ptree = 66% of std::set's 3.74e-06 s | std::set variation coef = 20% | ptree variation coef = 27%

----------------------------------------
1000 random elements and 10000 measurements:


INSERT time of ptree = 55% of std::set's 1.06e-04 s | std::set variation coef = 5% | ptree variation coef = 9%

REMOVE time of ptree = 46% of std::set's 1.24e-04 s | std::set variation coef = 5% | ptree variation coef = 6%

LOOP   time of ptree = 94% of std::set's 7.17e-06 s | std::set variation coef = 19% | ptree variation coef = 12%

ACCESS time of ptree = 69% of std::set's 5.40e-05 s | std::set variation coef = 5% | ptree variation coef = 9%

----------------------------------------
10000 random elements and 1000 measurements:


INSERT time of ptree = 62% of std::set's 1.51e-03 s | std::set variation coef = 4% | ptree variation coef = 2%

REMOVE time of ptree = 55% of std::set's 1.81e-03 s | std::set variation coef = 2% | ptree variation coef = 2%

LOOP   time of ptree = 111% of std::set's 1.07e-04 s | std::set variation coef = 6% | ptree variation coef = 17%

ACCESS time of ptree = 71% of std::set's 9.95e-04 s | std::set variation coef = 1% | ptree variation coef = 3%

----------------------------------------
100000 random elements and 100 measurements:


INSERT time of ptree = 80% of std::set's 2.60e-02 s | std::set variation coef = 2% | ptree variation coef = 5%

REMOVE time of ptree = 70% of std::set's 3.34e-02 s | std::set variation coef = 1% | ptree variation coef = 3%

LOOP   time of ptree = 148% of std::set's 2.42e-03 s | std::set variation coef = 8% | ptree variation coef = 10%

ACCESS time of ptree = 84% of std::set's 2.16e-02 s | std::set variation coef = 3% | ptree variation coef = 6%

----------------------------------------
1000000 random elements and 10 measurements:


INSERT time of ptree = 97% of std::set's 6.99e-01 s | std::set variation coef = 0% | ptree variation coef = 1%

REMOVE time of ptree = 84% of std::set's 8.78e-01 s | std::set variation coef = 0% | ptree variation coef = 0%

LOOP   time of ptree = 140% of std::set's 4.94e-02 s | std::set variation coef = 0% | ptree variation coef = 0%

ACCESS time of ptree = 94% of std::set's 6.72e-01 s | std::set variation coef = 0% | ptree variation coef = 0%


## Windows, 64 bit, Visual Studio 2019, Intel i5-6600K, ptree preallocation is: OFF

----------------------------------------
10 random elements and 1000000 measurements:


INSERT time of ptree = 157% of std::set's 6.14e-07 s | std::set variation coef = 57% | ptree variation coef = 53%

REMOVE time of ptree = 55% of std::set's 5.18e-07 s | std::set variation coef = 71% | ptree variation coef = 74%

LOOP   time of ptree = 98% of std::set's 7.69e-08 s | std::set variation coef = 146% | ptree variation coef = 85%

ACCESS time of ptree = 72% of std::set's 2.32e-07 s | std::set variation coef = 84% | ptree variation coef = 90%

----------------------------------------
100 random elements and 100000 measurements:


INSERT time of ptree = 117% of std::set's 9.46e-06 s | std::set variation coef = 30% | ptree variation coef = 28%

REMOVE time of ptree = 48% of std::set's 9.21e-06 s | std::set variation coef = 230% | ptree variation coef = 33%

LOOP   time of ptree = 97% of std::set's 6.29e-07 s | std::set variation coef = 83% | ptree variation coef = 102%

ACCESS time of ptree = 67% of std::set's 3.77e-06 s | std::set variation coef = 28% | ptree variation coef = 37%

----------------------------------------
1000 random elements and 10000 measurements:


INSERT time of ptree = 86% of std::set's 1.11e-04 s | std::set variation coef = 6% | ptree variation coef = 9%

REMOVE time of ptree = 45% of std::set's 1.26e-04 s | std::set variation coef = 7% | ptree variation coef = 7%

LOOP   time of ptree = 92% of std::set's 7.31e-06 s | std::set variation coef = 31% | ptree variation coef = 25%

ACCESS time of ptree = 68% of std::set's 5.48e-05 s | std::set variation coef = 7% | ptree variation coef = 9%

----------------------------------------
10000 random elements and 1000 measurements:


INSERT time of ptree = 101% of std::set's 1.51e-03 s | std::set variation coef = 3% | ptree variation coef = 6%

REMOVE time of ptree = 55% of std::set's 1.81e-03 s | std::set variation coef = 3% | ptree variation coef = 4%

LOOP   time of ptree = 108% of std::set's 1.08e-04 s | std::set variation coef = 6% | ptree variation coef = 21%

ACCESS time of ptree = 69% of std::set's 1.03e-03 s | std::set variation coef = 4% | ptree variation coef = 8%

----------------------------------------
100000 random elements and 100 measurements:


INSERT time of ptree = 103% of std::set's 2.54e-02 s | std::set variation coef = 3% | ptree variation coef = 5%

REMOVE time of ptree = 69% of std::set's 3.36e-02 s | std::set variation coef = 2% | ptree variation coef = 6%

LOOP   time of ptree = 149% of std::set's 2.43e-03 s | std::set variation coef = 6% | ptree variation coef = 12%

ACCESS time of ptree = 84% of std::set's 2.20e-02 s | std::set variation coef = 4% | ptree variation coef = 11%

----------------------------------------
1000000 random elements and 10 measurements:


INSERT time of ptree = 106% of std::set's 6.93e-01 s | std::set variation coef = 1% | ptree variation coef = 1%

REMOVE time of ptree = 85% of std::set's 8.84e-01 s | std::set variation coef = 2% | ptree variation coef = 4%

LOOP   time of ptree = 140% of std::set's 4.97e-02 s | std::set variation coef = 0% | ptree variation coef = 0%

ACCESS time of ptree = 93% of std::set's 6.81e-01 s | std::set variation coef = 1% | ptree variation coef = 1%

## Debian, 64 bit, GCC, Intel i5-3230M, ptree preallocation is: ON

----------------------------------------
10 random elements and 1000000 measurements:


INSERT time of ptree = 60% of std::set's 4.64e-07 s | std::set variation coef = 32% | ptree variation coef = 96%

REMOVE time of ptree = 65% of std::set's 5.26e-07 s | std::set variation coef = 31% | ptree variation coef = 41%

LOOP   time of ptree = 97% of std::set's 9.10e-08 s | std::set variation coef = 73% | ptree variation coef = 72%

ACCESS time of ptree = 83% of std::set's 2.61e-07 s | std::set variation coef = 99% | ptree variation coef = 119%

----------------------------------------
100 random elements and 100000 measurements:


INSERT time of ptree = 70% of std::set's 7.62e-06 s | std::set variation coef = 9% | ptree variation coef = 26%

REMOVE time of ptree = 68% of std::set's 8.17e-06 s | std::set variation coef = 9% | ptree variation coef = 9%

LOOP   time of ptree = 90% of std::set's 8.33e-07 s | std::set variation coef = 21% | ptree variation coef = 22%

ACCESS time of ptree = 80% of std::set's 4.19e-06 s | std::set variation coef = 24% | ptree variation coef = 24%

----------------------------------------
1000 random elements and 10000 measurements:


INSERT time of ptree = 71% of std::set's 1.03e-04 s | std::set variation coef = 4% | ptree variation coef = 5%

REMOVE time of ptree = 72% of std::set's 1.04e-04 s | std::set variation coef = 5% | ptree variation coef = 4%

LOOP   time of ptree = 95% of std::set's 9.35e-06 s | std::set variation coef = 8% | ptree variation coef = 28%

ACCESS time of ptree = 83% of std::set's 6.27e-05 s | std::set variation coef = 5% | ptree variation coef = 4%

----------------------------------------
10000 random elements and 1000 measurements:


INSERT time of ptree = 83% of std::set's 1.51e-03 s | std::set variation coef = 5% | ptree variation coef = 4%

REMOVE time of ptree = 90% of std::set's 1.50e-03 s | std::set variation coef = 5% | ptree variation coef = 4%

LOOP   time of ptree = 101% of std::set's 1.50e-04 s | std::set variation coef = 12% | ptree variation coef = 23%

ACCESS time of ptree = 80% of std::set's 1.20e-03 s | std::set variation coef = 5% | ptree variation coef = 5%

----------------------------------------
100000 random elements and 100 measurements:


INSERT time of ptree = 98% of std::set's 2.76e-02 s | std::set variation coef = 2% | ptree variation coef = 1%

REMOVE time of ptree = 114% of std::set's 2.98e-02 s | std::set variation coef = 1% | ptree variation coef = 0%

LOOP   time of ptree = 103% of std::set's 5.82e-03 s | std::set variation coef = 0% | ptree variation coef = 2%

ACCESS time of ptree = 76% of std::set's 3.10e-02 s | std::set variation coef = 1% | ptree variation coef = 1%

----------------------------------------
1000000 random elements and 10 measurements:


INSERT time of ptree = 109% of std::set's 8.22e-01 s | std::set variation coef = 3% | ptree variation coef = 2%

REMOVE time of ptree = 118% of std::set's 8.31e-01 s | std::set variation coef = 1% | ptree variation coef = 3%

LOOP   time of ptree = 101% of std::set's 8.42e-02 s | std::set variation coef = 0% | ptree variation coef = 0%

ACCESS time of ptree = 100% of std::set's 8.24e-01 s | std::set variation coef = 3% | ptree variation coef = 2%


## Debian, 64 bit, Intel i5-3230M, ptree preallocation is: OFF

----------------------------------------
10 random elements and 1000000 measurements:


INSERT time of ptree = 119% of std::set's 4.81e-07 s | std::set variation coef = 26% | ptree variation coef = 49%

REMOVE time of ptree = 63% of std::set's 5.33e-07 s | std::set variation coef = 24% | ptree variation coef = 29%

LOOP   time of ptree = 96% of std::set's 9.14e-08 s | std::set variation coef = 61% | ptree variation coef = 58%

ACCESS time of ptree = 82% of std::set's 2.62e-07 s | std::set variation coef = 135% | ptree variation coef = 36%

----------------------------------------
100 random elements and 100000 measurements:


INSERT time of ptree = 92% of std::set's 7.62e-06 s | std::set variation coef = 7% | ptree variation coef = 8%

REMOVE time of ptree = 68% of std::set's 8.16e-06 s | std::set variation coef = 12% | ptree variation coef = 8%

LOOP   time of ptree = 91% of std::set's 8.26e-07 s | std::set variation coef = 18% | ptree variation coef = 21%

ACCESS time of ptree = 80% of std::set's 4.17e-06 s | std::set variation coef = 20% | ptree variation coef = 24%

----------------------------------------
1000 random elements and 10000 measurements:


INSERT time of ptree = 88% of std::set's 1.12e-04 s | std::set variation coef = 4% | ptree variation coef = 3%

REMOVE time of ptree = 72% of std::set's 1.05e-04 s | std::set variation coef = 4% | ptree variation coef = 4%

LOOP   time of ptree = 94% of std::set's 9.55e-06 s | std::set variation coef = 25% | ptree variation coef = 7%

ACCESS time of ptree = 82% of std::set's 6.36e-05 s | std::set variation coef = 5% | ptree variation coef = 6%

----------------------------------------
10000 random elements and 1000 measurements:


INSERT time of ptree = 108% of std::set's 1.52e-03 s | std::set variation coef = 1% | ptree variation coef = 2%

REMOVE time of ptree = 91% of std::set's 1.43e-03 s | std::set variation coef = 2% | ptree variation coef = 4%

LOOP   time of ptree = 102% of std::set's 1.38e-04 s | std::set variation coef = 7% | ptree variation coef = 10%

ACCESS time of ptree = 81% of std::set's 1.17e-03 s | std::set variation coef = 1% | ptree variation coef = 3%

----------------------------------------
100000 random elements and 100 measurements:


INSERT time of ptree = 111% of std::set's 2.83e-02 s | std::set variation coef = 2% | ptree variation coef = 2%

REMOVE time of ptree = 116% of std::set's 2.93e-02 s | std::set variation coef = 1% | ptree variation coef = 1%

LOOP   time of ptree = 103% of std::set's 5.82e-03 s | std::set variation coef = 1% | ptree variation coef = 1%

ACCESS time of ptree = 74% of std::set's 3.09e-02 s | std::set variation coef = 1% | ptree variation coef = 2%

----------------------------------------
1000000 random elements and 10 measurements:


INSERT time of ptree = 111% of std::set's 8.14e-01 s | std::set variation coef = 3% | ptree variation coef = 1%

REMOVE time of ptree = 113% of std::set's 8.29e-01 s | std::set variation coef = 0% | ptree variation coef = 2%

LOOP   time of ptree = 100% of std::set's 8.46e-02 s | std::set variation coef = 0% | ptree variation coef = 0%

ACCESS time of ptree = 96% of std::set's 8.13e-01 s | std::set variation coef = 0% | ptree variation coef = 1%



# Credits

ptree was developed by Dario Mambro @ https://github.com/unevens/ptree

ptree is free and unencumbered software released into the public domain.
For more information, please refer to http://unlicense.org/
