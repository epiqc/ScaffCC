; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -consthoist -S < %s | FileCheck %s
target triple = "x86_64--"

; We don't want to convert constant divides because the benefit from converting
; them to a mul in the backend is larget than constant materialization savings.
define void @signed_const_division(i64 %in1, i64 %in2, i64* %addr) {
; CHECK-LABEL: @signed_const_division(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-NEXT:    [[L1:%.*]] = phi i64 [ [[RES1:%.*]], [[LOOP]] ], [ [[IN1:%.*]], [[ENTRY:%.*]] ]
; CHECK-NEXT:    [[L2:%.*]] = phi i64 [ [[RES2:%.*]], [[LOOP]] ], [ [[IN2:%.*]], [[ENTRY]] ]
; CHECK-NEXT:    [[RES1]] = sdiv i64 [[L1]], 4294967296
; CHECK-NEXT:    store volatile i64 [[RES1]], i64* [[ADDR:%.*]]
; CHECK-NEXT:    [[RES2]] = srem i64 [[L2]], 4294967296
; CHECK-NEXT:    store volatile i64 [[RES2]], i64* [[ADDR]]
; CHECK-NEXT:    [[AGAIN:%.*]] = icmp eq i64 [[RES1]], [[RES2]]
; CHECK-NEXT:    br i1 [[AGAIN]], label [[LOOP]], label [[END:%.*]]
; CHECK:       end:
; CHECK-NEXT:    ret void
;
entry:
  br label %loop

loop:
  %l1 = phi i64 [%res1, %loop], [%in1, %entry]
  %l2 = phi i64 [%res2, %loop], [%in2, %entry]
  %res1 = sdiv i64 %l1, 4294967296
  store volatile i64 %res1, i64* %addr
  %res2 = srem i64 %l2, 4294967296
  store volatile i64 %res2, i64* %addr
  %again = icmp eq i64 %res1, %res2
  br i1 %again, label %loop, label %end

end:
  ret void
}

define void @unsigned_const_division(i64 %in1, i64 %in2, i64* %addr) {
; CHECK-LABEL: @unsigned_const_division(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-NEXT:    [[L1:%.*]] = phi i64 [ [[RES1:%.*]], [[LOOP]] ], [ [[IN1:%.*]], [[ENTRY:%.*]] ]
; CHECK-NEXT:    [[L2:%.*]] = phi i64 [ [[RES2:%.*]], [[LOOP]] ], [ [[IN2:%.*]], [[ENTRY]] ]
; CHECK-NEXT:    [[RES1]] = udiv i64 [[L1]], 4294967296
; CHECK-NEXT:    store volatile i64 [[RES1]], i64* [[ADDR:%.*]]
; CHECK-NEXT:    [[RES2]] = urem i64 [[L2]], 4294967296
; CHECK-NEXT:    store volatile i64 [[RES2]], i64* [[ADDR]]
; CHECK-NEXT:    [[AGAIN:%.*]] = icmp eq i64 [[RES1]], [[RES2]]
; CHECK-NEXT:    br i1 [[AGAIN]], label [[LOOP]], label [[END:%.*]]
; CHECK:       end:
; CHECK-NEXT:    ret void
;

entry:
  br label %loop

loop:
  %l1 = phi i64 [%res1, %loop], [%in1, %entry]
  %l2 = phi i64 [%res2, %loop], [%in2, %entry]
  %res1 = udiv i64 %l1, 4294967296
  store volatile i64 %res1, i64* %addr
  %res2 = urem i64 %l2, 4294967296
  store volatile i64 %res2, i64* %addr
  %again = icmp eq i64 %res1, %res2
  br i1 %again, label %loop, label %end

end:
  ret void
}

define i32 @PR40934() {
; CHECK-LABEL: @PR40934(
; CHECK-NEXT:    ret i32 undef
; CHECK:       bb:
; CHECK-NEXT:    [[T2:%.*]] = call i32 (i64, ...) bitcast (i32 (...)* @d to i32 (i64, ...)*)(i64 7788015061)
; CHECK-NEXT:    [[T3:%.*]] = and i64 [[T3]], 7788015061
; CHECK-NEXT:    br label [[BB:%.*]]
;
  ret i32 undef

bb:
  %t2 = call i32 (i64, ...) bitcast (i32 (...)* @d to i32 (i64, ...)*)(i64 7788015061)
  %t3 = and i64 %t3, 7788015061
  br label %bb
}

declare i32 @d(...)

define i32 @PR40930() {
; CHECK-LABEL: @PR40930(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    [[TMP:%.*]] = alloca i32, align 4
; CHECK-NEXT:    br label [[BB1:%.*]]
; CHECK:       bb1:
; CHECK-NEXT:    br label [[BB2:%.*]]
; CHECK:       bb2:
; CHECK-NEXT:    br label [[BB2]]
; CHECK:       bb3:
; CHECK-NEXT:    [[TMP4:%.*]] = call i32 (i64, i64, ...) bitcast (i32 (...)* @c to i32 (i64, i64, ...)*)(i64 4208870971, i64 4208870971)
; CHECK-NEXT:    br label [[BB1]]
; CHECK:       bb5:
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, i32* [[TMP]], align 4
; CHECK-NEXT:    ret i32 [[TMP6]]
;
bb:
  %tmp = alloca i32, align 4
  br label %bb1

bb1:                                              ; preds = %bb3, %bb
  br label %bb2

bb2:                                              ; preds = %bb2, %bb1
  br label %bb2

bb3:                                              ; No predecessors!
  %tmp4 = call i32 (i64, i64, ...) bitcast (i32 (...)* @c to i32 (i64, i64, ...)*)(i64 4208870971, i64 4208870971)
  br label %bb1

bb5:                                              ; No predecessors!
  %tmp6 = load i32, i32* %tmp, align 4
  ret i32 %tmp6
}

declare i32 @c(...)
