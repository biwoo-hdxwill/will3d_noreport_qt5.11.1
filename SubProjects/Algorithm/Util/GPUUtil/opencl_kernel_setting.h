#define cl_kernel_loop(i, n)\
for (int i = get_global_id(0); i < (n); i += get_global_size(0))
