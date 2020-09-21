__kernel void vector_add(__global const int * lhs, __global const int * rhs, __global int * res) {
    int i = get_global_id(0);
    res[i] = lhs[i] + rhs[i];
}