project('aria', ['c', 'cpp'], default_options: ['c_std=gnu23', 'warning_level=2', 'werror=true'])

src = files(
  'src/bitmap.c',
  'src/aslr.c',
  'src/circular_queue.c',
  'src/dictionary.c',
  'src/elf.c',
  'src/pairing_heap.c',
  'src/slab.c',
  'src/base.c',
  'src/ubsan.c',
  'src/printf.c',
)

inc = include_directories('include')

aria_lib = static_library('aria', src, include_directories: [inc, include_directories('.')])
aria_dep = declare_dependency(link_with: aria_lib, include_directories: inc)
