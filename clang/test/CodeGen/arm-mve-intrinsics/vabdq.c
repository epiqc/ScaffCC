// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
// RUN: %clang_cc1 -triple thumbv8.1m.main-arm-none-eabi -target-feature +mve.fp -mfloat-abi hard -fallow-half-arguments-and-returns -O0 -disable-O0-optnone -S -emit-llvm -o - %s | opt -S -mem2reg | FileCheck %s
// RUN: %clang_cc1 -triple thumbv8.1m.main-arm-none-eabi -target-feature +mve.fp -mfloat-abi hard -fallow-half-arguments-and-returns -O0 -disable-O0-optnone -DPOLYMORPHIC -S -emit-llvm -o - %s | opt -S -mem2reg | FileCheck %s

#include <arm_mve.h>

// CHECK-LABEL: @test_vabdq_s8(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call <16 x i8> @llvm.arm.mve.vabd.v16i8(<16 x i8> [[A:%.*]], <16 x i8> [[B:%.*]], i32 0)
// CHECK-NEXT:    ret <16 x i8> [[TMP0]]
//
int8x16_t test_vabdq_s8(int8x16_t a, int8x16_t b)
{
#ifdef POLYMORPHIC
    return vabdq(a, b);
#else /* POLYMORPHIC */
    return vabdq_s8(a, b);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_u32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call <4 x i32> @llvm.arm.mve.vabd.v4i32(<4 x i32> [[A:%.*]], <4 x i32> [[B:%.*]], i32 1)
// CHECK-NEXT:    ret <4 x i32> [[TMP0]]
//
uint32x4_t test_vabdq_u32(uint32x4_t a, uint32x4_t b)
{
#ifdef POLYMORPHIC
    return vabdq(a, b);
#else /* POLYMORPHIC */
    return vabdq_u32(a, b);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_f32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = call <8 x half> @llvm.arm.mve.vabd.v8f16(<8 x half> [[A:%.*]], <8 x half> [[B:%.*]], i32 0)
// CHECK-NEXT:    ret <8 x half> [[TMP0]]
//
float16x8_t test_vabdq_f32(float16x8_t a, float16x8_t b)
{
#ifdef POLYMORPHIC
    return vabdq(a, b);
#else /* POLYMORPHIC */
    return vabdq_f16(a, b);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_m_u16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <8 x i1> @llvm.arm.mve.pred.i2v.v8i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <8 x i16> @llvm.arm.mve.abd.predicated.v8i16.v8i1(<8 x i16> [[A:%.*]], <8 x i16> [[B:%.*]], i32 1, <8 x i1> [[TMP1]], <8 x i16> [[INACTIVE:%.*]])
// CHECK-NEXT:    ret <8 x i16> [[TMP2]]
//
uint16x8_t test_vabdq_m_u16(uint16x8_t inactive, uint16x8_t a, uint16x8_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_m(inactive, a, b, p);
#else /* POLYMORPHIC */
    return vabdq_m_u16(inactive, a, b, p);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_m_s8(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <16 x i1> @llvm.arm.mve.pred.i2v.v16i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <16 x i8> @llvm.arm.mve.abd.predicated.v16i8.v16i1(<16 x i8> [[A:%.*]], <16 x i8> [[B:%.*]], i32 0, <16 x i1> [[TMP1]], <16 x i8> [[INACTIVE:%.*]])
// CHECK-NEXT:    ret <16 x i8> [[TMP2]]
//
int8x16_t test_vabdq_m_s8(int8x16_t inactive, int8x16_t a, int8x16_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_m(inactive, a, b, p);
#else /* POLYMORPHIC */
    return vabdq_m_s8(inactive, a, b, p);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_m_f32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <4 x i1> @llvm.arm.mve.pred.i2v.v4i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <4 x float> @llvm.arm.mve.abd.predicated.v4f32.v4i1(<4 x float> [[A:%.*]], <4 x float> [[B:%.*]], i32 0, <4 x i1> [[TMP1]], <4 x float> [[INACTIVE:%.*]])
// CHECK-NEXT:    ret <4 x float> [[TMP2]]
//
float32x4_t test_vabdq_m_f32(float32x4_t inactive, float32x4_t a, float32x4_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_m(inactive, a, b, p);
#else /* POLYMORPHIC */
    return vabdq_m_f32(inactive, a, b, p);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_x_u16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <8 x i1> @llvm.arm.mve.pred.i2v.v8i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <8 x i16> @llvm.arm.mve.abd.predicated.v8i16.v8i1(<8 x i16> [[A:%.*]], <8 x i16> [[B:%.*]], i32 1, <8 x i1> [[TMP1]], <8 x i16> undef)
// CHECK-NEXT:    ret <8 x i16> [[TMP2]]
//
uint16x8_t test_vabdq_x_u16(uint16x8_t a, uint16x8_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_x(a, b, p);
#else /* POLYMORPHIC */
    return vabdq_x_u16(a, b, p);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_x_u32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <4 x i1> @llvm.arm.mve.pred.i2v.v4i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <4 x i32> @llvm.arm.mve.abd.predicated.v4i32.v4i1(<4 x i32> [[A:%.*]], <4 x i32> [[B:%.*]], i32 1, <4 x i1> [[TMP1]], <4 x i32> undef)
// CHECK-NEXT:    ret <4 x i32> [[TMP2]]
//
uint32x4_t test_vabdq_x_u32(uint32x4_t a, uint32x4_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_x(a, b, p);
#else /* POLYMORPHIC */
    return vabdq_x_u32(a, b, p);
#endif /* POLYMORPHIC */
}

// CHECK-LABEL: @test_vabdq_x_f16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = zext i16 [[P:%.*]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = call <8 x i1> @llvm.arm.mve.pred.i2v.v8i1(i32 [[TMP0]])
// CHECK-NEXT:    [[TMP2:%.*]] = call <8 x half> @llvm.arm.mve.abd.predicated.v8f16.v8i1(<8 x half> [[A:%.*]], <8 x half> [[B:%.*]], i32 0, <8 x i1> [[TMP1]], <8 x half> undef)
// CHECK-NEXT:    ret <8 x half> [[TMP2]]
//
float16x8_t test_vabdq_x_f16(float16x8_t a, float16x8_t b, mve_pred16_t p)
{
#ifdef POLYMORPHIC
    return vabdq_x(a, b, p);
#else /* POLYMORPHIC */
    return vabdq_x_f16(a, b, p);
#endif /* POLYMORPHIC */
}
