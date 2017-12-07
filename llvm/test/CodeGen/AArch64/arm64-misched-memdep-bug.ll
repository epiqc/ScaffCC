; REQUIRES: asserts
; RUN: llc < %s -mtriple=arm64-linux-gnu -mcpu=cortex-a57 -enable-misched -verify-misched -debug-only=machine-scheduler -o - 2>&1 > /dev/null | FileCheck %s
;
; Test for bug in misched memory dependency calculation.
;
; CHECK: ********** MI Scheduling **********
; CHECK: misched_bug:%bb.0 entry
; CHECK: SU(2):   %2<def> = LDRWui %0, 1; mem:LD4[%ptr1_plus1] GPR32:%2 GPR64common:%0
; CHECK:   Successors:
; CHECK-NEXT:    SU(5): Data Latency=4 Reg=%2
; CHECK-NEXT:    SU(4): Ord  Latency=0
; CHECK: SU(3):   STRWui %wzr, %0, 0; mem:ST4[%ptr1] GPR64common:%0
; CHECK:   Successors:
; CHECK: SU(4): Ord  Latency=0
; CHECK: SU(4):   STRWui %wzr, %1, 0; mem:ST4[%ptr2] GPR64common:%1
; CHECK: SU(5):   %w0<def> = COPY %2; GPR32:%2
; CHECK: ** ScheduleDAGMI::schedule picking next node
define i32 @misched_bug(i32* %ptr1, i32* %ptr2) {
entry:
  %ptr1_plus1 = getelementptr inbounds i32, i32* %ptr1, i64 1
  %val1 = load i32, i32* %ptr1_plus1, align 4
  store i32 0, i32* %ptr1, align 4
  store i32 0, i32* %ptr2, align 4
  ret i32 %val1
}
