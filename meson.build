project('aria', 'c', default_options: ['c_std=gnu23', 'warning_level=2', 'werror=true'])

src = files(
  'src/pairing_heap.c',
  'src/base.c',
  'src/ubsan.c',
  'src/printf.c',
  'src/hamt.c',
  'src/avl.c',
  'src/rbtree.c',
  'src/bst.c',
)

inc = include_directories('include')

aria_lib = static_library('aria', src, include_directories: [inc, include_directories('.')])
aria_dep = declare_dependency(link_with: aria_lib, include_directories: inc)

if not meson.is_subproject()
  add_languages('cpp', native: false)
  bench_dep = [dependency('benchmark'), dependency('absl_base'), dependency('absl_raw_logging_internal'), dependency('absl_raw_hash_set')]

  executable('bench', sources: files('benchmarks/hamt.cc', 'src/hamt.c', 'src/avl.c', 'src/bst.c'), include_directories: inc, dependencies: bench_dep)
endif
