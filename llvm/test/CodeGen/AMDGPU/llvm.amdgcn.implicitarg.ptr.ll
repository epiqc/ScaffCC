; RUN: llc -mtriple=amdgcn-amd-amdhsa -verify-machineinstrs < %s | FileCheck -check-prefixes=GCN,HSA,HSA-NOENV %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa-opencl -verify-machineinstrs < %s | FileCheck -check-prefixes=GCN,HSA,HSA-OPENCL %s
; RUN: llc -mtriple=amdgcn-mesa-mesa3d -verify-machineinstrs < %s | FileCheck -check-prefixes=GCN,MESA %s

; GCN-LABEL: {{^}}kernel_implicitarg_ptr_empty:
; GCN: enable_sgpr_kernarg_segment_ptr = 1

; HSA-NOENV: kernarg_segment_byte_size = 0
; HSA-OPENCL: kernarg_segment_byte_size = 32
; MESA: kernarg_segment_byte_size = 16

; HSA: s_load_dword s0, s[4:5], 0x0
define amdgpu_kernel void @kernel_implicitarg_ptr_empty() #0 {
  %implicitarg.ptr = call i8 addrspace(2)* @llvm.amdgcn.implicitarg.ptr()
  %cast = bitcast i8 addrspace(2)* %implicitarg.ptr to i32 addrspace(2)*
  %load = load volatile i32, i32 addrspace(2)* %cast
  ret void
}

; GCN-LABEL: {{^}}kernel_implicitarg_ptr:
; GCN: enable_sgpr_kernarg_segment_ptr = 1

; HSA-NOENV: kernarg_segment_byte_size = 112
; HSA-OPENCL: kernarg_segment_byte_size = 144
; MESA: kernarg_segment_byte_size = 464

; HSA: s_load_dword s0, s[4:5], 0x1c
define amdgpu_kernel void @kernel_implicitarg_ptr([112 x i8]) #0 {
  %implicitarg.ptr = call i8 addrspace(2)* @llvm.amdgcn.implicitarg.ptr()
  %cast = bitcast i8 addrspace(2)* %implicitarg.ptr to i32 addrspace(2)*
  %load = load volatile i32, i32 addrspace(2)* %cast
  ret void
}

; GCN-LABEL: {{^}}func_implicitarg_ptr:
; GCN: s_waitcnt
; GCN-NEXT: s_load_dword s{{[0-9]+}}, s[6:7], 0x0{{$}}
; GCN-NEXT: s_waitcnt
; GCN-NEXT: s_setpc_b64
define void @func_implicitarg_ptr() #1 {
  %implicitarg.ptr = call i8 addrspace(2)* @llvm.amdgcn.implicitarg.ptr()
  %cast = bitcast i8 addrspace(2)* %implicitarg.ptr to i32 addrspace(2)*
  %load = load volatile i32, i32 addrspace(2)* %cast
  ret void
}

; GCN-LABEL: {{^}}kernel_call_implicitarg_ptr_func_empty:
; GCN: enable_sgpr_kernarg_segment_ptr = 1
; HSA-NOENV: kernarg_segment_byte_size = 0
; HSA-OPENCL: kernarg_segment_byte_size = 32
; MESA: kernarg_segment_byte_size = 16
; GCN: s_mov_b64 s[6:7], s[4:5]
; GCN: s_swappc_b64
define amdgpu_kernel void @kernel_call_implicitarg_ptr_func_empty() #0 {
  call void @func_implicitarg_ptr()
  ret void
}

; GCN-LABEL: {{^}}kernel_call_implicitarg_ptr_func:
; GCN: enable_sgpr_kernarg_segment_ptr = 1
; HSA-OPENCL: kernarg_segment_byte_size = 144
; HSA-NOENV: kernarg_segment_byte_size = 112
; MESA: kernarg_segment_byte_size = 464

; HSA: s_add_u32 s6, s4, 0x70
; MESA: s_add_u32 s6, s4, 0x1c0

; GCN: s_addc_u32 s7, s5, 0{{$}}
; GCN: s_swappc_b64
define amdgpu_kernel void @kernel_call_implicitarg_ptr_func([112 x i8]) #0 {
  call void @func_implicitarg_ptr()
  ret void
}

; GCN-LABEL: {{^}}func_call_implicitarg_ptr_func:
; GCN-NOT: s6
; GCN-NOT: s7
; GCN-NOT: s[6:7]
define void @func_call_implicitarg_ptr_func() #1 {
  call void @func_implicitarg_ptr()
  ret void
}

; GCN-LABEL: {{^}}func_kernarg_implicitarg_ptr:
; GCN: s_waitcnt
; GCN: s_load_dword s{{[0-9]+}}, s[6:7], 0x0{{$}}
; GCN: s_load_dword s{{[0-9]+}}, s[8:9], 0x0{{$}}
define void @func_kernarg_implicitarg_ptr() #1 {
  %kernarg.segment.ptr = call i8 addrspace(2)* @llvm.amdgcn.kernarg.segment.ptr()
  %implicitarg.ptr = call i8 addrspace(2)* @llvm.amdgcn.implicitarg.ptr()
  %cast.kernarg.segment.ptr = bitcast i8 addrspace(2)* %kernarg.segment.ptr to i32 addrspace(2)*
  %cast.implicitarg = bitcast i8 addrspace(2)* %implicitarg.ptr to i32 addrspace(2)*
  %load0 = load volatile i32, i32 addrspace(2)* %cast.kernarg.segment.ptr
  %load1 = load volatile i32, i32 addrspace(2)* %cast.implicitarg
  ret void
}

; GCN-LABEL: {{^}}kernel_call_kernarg_implicitarg_ptr_func:
; GCN: s_mov_b64 s[6:7], s[4:5]
; HSA: s_add_u32 s8, s6, 0x70
; MESA: s_add_u32 s8, s6, 0x1c0
; GCN: s_addc_u32 s9, s7, 0
; GCN: s_swappc_b64
define amdgpu_kernel void @kernel_call_kernarg_implicitarg_ptr_func([112 x i8]) #0 {
  call void @func_kernarg_implicitarg_ptr()
  ret void
}

declare i8 addrspace(2)* @llvm.amdgcn.implicitarg.ptr() #2
declare i8 addrspace(2)* @llvm.amdgcn.kernarg.segment.ptr() #2

attributes #0 = { nounwind noinline }
attributes #1 = { nounwind noinline }
attributes #2 = { nounwind readnone speculatable }
