; RUN: llc -march=amdgcn -mcpu=tahiti  -verify-machineinstrs -show-mc-encoding < %s | FileCheck -check-prefix=SI   -check-prefix=GCN -check-prefix=SICI -check-prefix=SIVIGFX9 %s
; RUN: llc -march=amdgcn -mcpu=bonaire -verify-machineinstrs -show-mc-encoding < %s | FileCheck -check-prefix=CI   -check-prefix=GCN -check-prefix=SICI %s
; RUN: llc -march=amdgcn -mcpu=tonga   -verify-machineinstrs -show-mc-encoding < %s | FileCheck -check-prefix=VI   -check-prefix=GCN -check-prefix=VIGFX9 -check-prefix=SIVIGFX9 %s
; RUN: llc -march=amdgcn -mcpu=gfx900  -verify-machineinstrs -show-mc-encoding < %s | FileCheck -check-prefix=GFX9 -check-prefix=GCN -check-prefix=VIGFX9 -check-prefix=SIVIGFX9  %s

; SMRD load with an immediate offset.
; GCN-LABEL: {{^}}smrd0:
; SICI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x1 ; encoding: [0x01
; VIGFX9: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x4
define amdgpu_kernel void @smrd0(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 1
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; SMRD load with the largest possible immediate offset.
; GCN-LABEL: {{^}}smrd1:
; SICI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0xff ; encoding: [0xff,0x{{[0-9]+[137]}}
; VIGFX9: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x3fc
define amdgpu_kernel void @smrd1(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 255
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; SMRD load with an offset greater than the largest possible immediate.
; GCN-LABEL: {{^}}smrd2:
; SI: s_movk_i32 s[[OFFSET:[0-9]]], 0x400
; SI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], s[[OFFSET]] ; encoding: [0x0[[OFFSET]]
; CI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x100
; VIGFX9: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x400
; GCN: s_endpgm
define amdgpu_kernel void @smrd2(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 256
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; SMRD load with a 64-bit offset
; GCN-LABEL: {{^}}smrd3:
; FIXME: There are too many copies here because we don't fold immediates
;        through REG_SEQUENCE
; SI: s_load_dwordx2 s[{{[0-9]:[0-9]}}], s[{{[0-9]:[0-9]}}], 0xb ; encoding: [0x0b
; TODO: Add VI checks
; GCN: s_endpgm
define amdgpu_kernel void @smrd3(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 4294967296
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; SMRD load with the largest possible immediate offset on VI
; GCN-LABEL: {{^}}smrd4:
; SI: s_mov_b32 [[OFFSET:s[0-9]+]], 0xffffc
; SI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], [[OFFSET]]
; CI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x3ffff
; VIGFX9: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0xffffc
define amdgpu_kernel void @smrd4(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 262143
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; SMRD load with an offset greater than the largest possible immediate on VI
; GCN-LABEL: {{^}}smrd5:
; SIVIGFX9: s_mov_b32 [[OFFSET:s[0-9]+]], 0x100000
; SIVIGFX9: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], [[OFFSET]]
; CI: s_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x40000
; GCN: s_endpgm
define amdgpu_kernel void @smrd5(i32 addrspace(1)* %out, i32 addrspace(2)* %ptr) #0 {
entry:
  %tmp = getelementptr i32, i32 addrspace(2)* %ptr, i64 262144
  %tmp1 = load i32, i32 addrspace(2)* %tmp
  store i32 %tmp1, i32 addrspace(1)* %out
  ret void
}

; GCN-LABEL: {{^}}smrd_hazard:
; GCN-DAG: s_mov_b32 s3, 3
; GCN-DAG: s_mov_b32 s2, 2
; GCN-DAG: s_mov_b32 s1, 1
; GCN-DAG: s_mov_b32 s0, 0
; SI-NEXT: nop 3
; GCN-NEXT: s_buffer_load_dword s0, s[0:3], 0x0
define amdgpu_ps float @smrd_hazard(<4 x i32> inreg %desc) #0 {
main_body:
  %d0 = insertelement <4 x i32> undef, i32 0, i32 0
  %d1 = insertelement <4 x i32> %d0, i32 1, i32 1
  %d2 = insertelement <4 x i32> %d1, i32 2, i32 2
  %d3 = insertelement <4 x i32> %d2, i32 3, i32 3
  %r = call float @llvm.SI.load.const.v4i32(<4 x i32> %d3, i32 0)
  ret float %r
}

; SMRD load using the load.const.v4i32 intrinsic with an immediate offset
; GCN-LABEL: {{^}}smrd_load_const0:
; SICI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x4 ; encoding: [0x04
; VIGFX9: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x10
define amdgpu_ps void @smrd_load_const0(<4 x i32> addrspace(2)* inreg %arg, <4 x i32> addrspace(2)* inreg %arg1, <32 x i8> addrspace(2)* inreg %arg2, i32 inreg %arg3, <2 x i32> %arg4, <2 x i32> %arg5, <2 x i32> %arg6, <3 x i32> %arg7, <2 x i32> %arg8, <2 x i32> %arg9, <2 x i32> %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, float %arg17, float %arg18, float %arg19) #0 {
main_body:
  %tmp = getelementptr <4 x i32>, <4 x i32> addrspace(2)* %arg, i32 0
  %tmp20 = load <4 x i32>, <4 x i32> addrspace(2)* %tmp
  %tmp21 = call float @llvm.SI.load.const.v4i32(<4 x i32> %tmp20, i32 16)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %tmp21, float %tmp21, float %tmp21, float %tmp21, i1 true, i1 true) #0
  ret void
}

; SMRD load using the load.const.v4i32 intrinsic with the largest possible immediate
; offset.
; GCN-LABEL: {{^}}smrd_load_const1:
; SICI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0xff ; encoding: [0xff
; VIGFX9: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x3fc
define amdgpu_ps void @smrd_load_const1(<4 x i32> addrspace(2)* inreg %arg, <4 x i32> addrspace(2)* inreg %arg1, <32 x i8> addrspace(2)* inreg %arg2, i32 inreg %arg3, <2 x i32> %arg4, <2 x i32> %arg5, <2 x i32> %arg6, <3 x i32> %arg7, <2 x i32> %arg8, <2 x i32> %arg9, <2 x i32> %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, float %arg17, float %arg18, float %arg19) #0 {
main_body:
  %tmp = getelementptr <4 x i32>, <4 x i32> addrspace(2)* %arg, i32 0
  %tmp20 = load <4 x i32>, <4 x i32> addrspace(2)* %tmp
  %tmp21 = call float @llvm.SI.load.const.v4i32(<4 x i32> %tmp20, i32 1020)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %tmp21, float %tmp21, float %tmp21, float %tmp21, i1 true, i1 true) #0
  ret void
}

; SMRD load using the load.const.v4i32 intrinsic with an offset greater than the
; largets possible immediate.
; immediate offset.
; GCN-LABEL: {{^}}smrd_load_const2:
; SI: s_movk_i32 s[[OFFSET:[0-9]]], 0x400
; SI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], s[[OFFSET]] ; encoding: [0x0[[OFFSET]]
; CI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x100
; VIGFX9: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x400
define amdgpu_ps void @smrd_load_const2(<4 x i32> addrspace(2)* inreg %arg, <4 x i32> addrspace(2)* inreg %arg1, <32 x i8> addrspace(2)* inreg %arg2, i32 inreg %arg3, <2 x i32> %arg4, <2 x i32> %arg5, <2 x i32> %arg6, <3 x i32> %arg7, <2 x i32> %arg8, <2 x i32> %arg9, <2 x i32> %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, float %arg17, float %arg18, float %arg19) #0 {
main_body:
  %tmp = getelementptr <4 x i32>, <4 x i32> addrspace(2)* %arg, i32 0
  %tmp20 = load <4 x i32>, <4 x i32> addrspace(2)* %tmp
  %tmp21 = call float @llvm.SI.load.const.v4i32(<4 x i32> %tmp20, i32 1024)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %tmp21, float %tmp21, float %tmp21, float %tmp21, i1 true, i1 true) #0
  ret void
}

; SMRD load with the largest possible immediate offset on VI
; GCN-LABEL: {{^}}smrd_load_const3:
; SI: s_mov_b32 [[OFFSET:s[0-9]+]], 0xffffc
; SI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], [[OFFSET]]
; CI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x3ffff
; VIGFX9: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0xffffc
define amdgpu_ps void @smrd_load_const3(<4 x i32> addrspace(2)* inreg %arg, <4 x i32> addrspace(2)* inreg %arg1, <32 x i8> addrspace(2)* inreg %arg2, i32 inreg %arg3, <2 x i32> %arg4, <2 x i32> %arg5, <2 x i32> %arg6, <3 x i32> %arg7, <2 x i32> %arg8, <2 x i32> %arg9, <2 x i32> %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, float %arg17, float %arg18, float %arg19) #0 {
main_body:
  %tmp = getelementptr <4 x i32>, <4 x i32> addrspace(2)* %arg, i32 0
  %tmp20 = load <4 x i32>, <4 x i32> addrspace(2)* %tmp
  %tmp21 = call float @llvm.SI.load.const.v4i32(<4 x i32> %tmp20, i32 1048572)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %tmp21, float %tmp21, float %tmp21, float %tmp21, i1 true, i1 true) #0
  ret void
}

; SMRD load with an offset greater than the largest possible immediate on VI
; GCN-LABEL: {{^}}smrd_load_const4:
; SIVIGFX9: s_mov_b32 [[OFFSET:s[0-9]+]], 0x100000
; SIVIGFX9: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], [[OFFSET]]
; CI: s_buffer_load_dword s{{[0-9]}}, s[{{[0-9]:[0-9]}}], 0x40000
; GCN: s_endpgm
define amdgpu_ps void @smrd_load_const4(<4 x i32> addrspace(2)* inreg %arg, <4 x i32> addrspace(2)* inreg %arg1, <32 x i8> addrspace(2)* inreg %arg2, i32 inreg %arg3, <2 x i32> %arg4, <2 x i32> %arg5, <2 x i32> %arg6, <3 x i32> %arg7, <2 x i32> %arg8, <2 x i32> %arg9, <2 x i32> %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, float %arg17, float %arg18, float %arg19) #0 {
main_body:
  %tmp = getelementptr <4 x i32>, <4 x i32> addrspace(2)* %arg, i32 0
  %tmp20 = load <4 x i32>, <4 x i32> addrspace(2)* %tmp
  %tmp21 = call float @llvm.SI.load.const.v4i32(<4 x i32> %tmp20, i32 1048576)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %tmp21, float %tmp21, float %tmp21, float %tmp21, i1 true, i1 true) #0
  ret void
}

; GCN-LABEL: {{^}}smrd_sgpr_offset:
; GCN: s_buffer_load_dword s{{[0-9]}}, s[0:3], s4
define amdgpu_ps float @smrd_sgpr_offset(<4 x i32> inreg %desc, i32 inreg %offset) #0 {
main_body:
  %r = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %offset)
  ret float %r
}

; GCN-LABEL: {{^}}smrd_vgpr_offset:
; GCN: buffer_load_dword v{{[0-9]}}, v0, s[0:3], 0 offen ;
define amdgpu_ps float @smrd_vgpr_offset(<4 x i32> inreg %desc, i32 %offset) #0 {
main_body:
  %r = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %offset)
  ret float %r
}

; GCN-LABEL: {{^}}smrd_vgpr_offset_imm:
; GCN-NEXT: %bb.

; SICIVI-NEXT: buffer_load_dword v{{[0-9]}}, v0, s[0:3], 0 offen offset:4095 ;

; GFX9-NEXT: v_add_u32_e32 [[ADD:v[0-9]+]], 0xfff, v0
; GFX9-NEXT: buffer_load_dword v{{[0-9]}}, [[ADD]], s[0:3], 0 offen ;
define amdgpu_ps float @smrd_vgpr_offset_imm(<4 x i32> inreg %desc, i32 %offset) #0 {
main_body:
  %off = add i32 %offset, 4095
  %r = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %off)
  ret float %r
}

; GCN-LABEL: {{^}}smrd_vgpr_offset_imm_too_large:
; GCN-NEXT: %bb.
; GCN-NEXT: v_add_{{i|u}}32_e32 v0, {{(vcc, )?}}0x1000, v0
; GCN-NEXT: buffer_load_dword v{{[0-9]}}, v0, s[0:3], 0 offen ;
define amdgpu_ps float @smrd_vgpr_offset_imm_too_large(<4 x i32> inreg %desc, i32 %offset) #0 {
main_body:
  %off = add i32 %offset, 4096
  %r = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %off)
  ret float %r
}

; GCN-LABEL: {{^}}smrd_imm_merged:
; GCN-NEXT: %bb.
; SICI-NEXT: s_buffer_load_dwordx4 s[{{[0-9]}}:{{[0-9]}}], s[0:3], 0x1
; SICI-NEXT: s_buffer_load_dwordx2 s[{{[0-9]}}:{{[0-9]}}], s[0:3], 0x7
; VI-NEXT: s_buffer_load_dwordx4 s[{{[0-9]}}:{{[0-9]}}], s[0:3], 0x4
; VI-NEXT: s_buffer_load_dwordx2 s[{{[0-9]}}:{{[0-9]}}], s[0:3], 0x1c
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
; GFX9-NEXT: s_buffer_load_dword s{{[0-9]}}
define amdgpu_ps void @smrd_imm_merged(<4 x i32> inreg %desc) #0 {
main_body:
  %r1 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 4)
  %r2 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 8)
  %r3 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 12)
  %r4 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 16)
  %r5 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 28)
  %r6 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 32)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %r1, float %r2, float %r3, float %r4, i1 true, i1 true) #0
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %r5, float %r6, float undef, float undef, i1 true, i1 true) #0
  ret void
}

; GCN-LABEL: {{^}}smrd_vgpr_merged:
; GCN-NEXT: %bb.

; SICIVI-NEXT: buffer_load_dwordx4 v[{{[0-9]}}:{{[0-9]}}], v0, s[0:3], 0 offen offset:4
; SICIVI-NEXT: buffer_load_dwordx2 v[{{[0-9]}}:{{[0-9]}}], v0, s[0:3], 0 offen offset:28

; GFX9: buffer_load_dword
; GFX9: buffer_load_dword
; GFX9: buffer_load_dword
; GFX9: buffer_load_dword
; GFX9: buffer_load_dword
; GFX9: buffer_load_dword
define amdgpu_ps void @smrd_vgpr_merged(<4 x i32> inreg %desc, i32 %a) #0 {
main_body:
  %a1 = add i32 %a, 4
  %a2 = add i32 %a, 8
  %a3 = add i32 %a, 12
  %a4 = add i32 %a, 16
  %a5 = add i32 %a, 28
  %a6 = add i32 %a, 32
  %r1 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a1)
  %r2 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a2)
  %r3 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a3)
  %r4 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a4)
  %r5 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a5)
  %r6 = call float @llvm.SI.load.const.v4i32(<4 x i32> %desc, i32 %a6)
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %r1, float %r2, float %r3, float %r4, i1 true, i1 true) #0
  call void @llvm.amdgcn.exp.f32(i32 0, i32 15, float %r5, float %r6, float undef, float undef, i1 true, i1 true) #0
  ret void
}

declare void @llvm.amdgcn.exp.f32(i32, i32, float, float, float, float, i1, i1) #0
declare float @llvm.SI.load.const.v4i32(<4 x i32>, i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
