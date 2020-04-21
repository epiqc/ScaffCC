; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=ppc32 | FileCheck %s

declare  i32 @llvm.umul.fix.sat.i32(i32, i32, i32)

define i32 @func1(i32 %x, i32 %y) nounwind {
; CHECK-LABEL: func1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    li 5, -1
; CHECK-NEXT:    mulhwu. 6, 3, 4
; CHECK-NEXT:    mullw 3, 3, 4
; CHECK-NEXT:    bclr 12, 2, 0
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    ori 3, 5, 0
; CHECK-NEXT:    blr
  %tmp = call i32 @llvm.umul.fix.sat.i32(i32 %x, i32 %y, i32 0)
  ret i32 %tmp
}

define i32 @func2(i32 %x, i32 %y) nounwind {
; CHECK-LABEL: func2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    mulhwu 6, 3, 4
; CHECK-NEXT:    li 5, -1
; CHECK-NEXT:    cmplwi 6, 1
; CHECK-NEXT:    mullw 3, 3, 4
; CHECK-NEXT:    rotlwi 3, 3, 31
; CHECK-NEXT:    rlwimi 3, 6, 31, 0, 0
; CHECK-NEXT:    bc 12, 1, .LBB1_1
; CHECK-NEXT:    blr
; CHECK-NEXT:  .LBB1_1:
; CHECK-NEXT:    addi 3, 5, 0
; CHECK-NEXT:    blr
  %tmp = call i32 @llvm.umul.fix.sat.i32(i32 %x, i32 %y, i32 1)
  ret i32 %tmp
}
