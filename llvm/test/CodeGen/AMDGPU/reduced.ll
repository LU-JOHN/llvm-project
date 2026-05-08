; Derived from buffer-fat-pointer-atomicrmw-fmax.ll
; llc -mtriple=amdgcn-amd-amdpal -mcpu=gfx1200 -mattr=+real-true16 < reduced.ll

define double @buffer_fat_ptr_agent_atomic_fmax_ret_f64__offset__amdgpu_no_fine_grained_memory() {
  %result = atomicrmw fmax ptr addrspace(7) null, double 0.000000e+00 monotonic, align 8
  ret double %result
}
