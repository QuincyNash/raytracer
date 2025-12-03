#include <metal_stdlib>
using namespace metal;

kernel void speedTest(device float* data [[buffer(0)]],
                         uint id [[thread_position_in_grid]]) {
  float x = float(id);

  #pragma unroll
  for (uint i = 0; i < 16384; ++i) {
    x = sqrt(x + 1.0);
  }

  data[id] = x;
}