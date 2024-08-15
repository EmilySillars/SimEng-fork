#include <algorithm>
#include <limits>

#include "AArch64RegressionTest.hh"

namespace {

using InstSme = AArch64RegressionTest;

#if SIMENG_LLVM_VERSION >= 14
TEST_P(InstSme, mova_tileToVec) {
  // 8-bit
  initialHeapData_.resize(SVL / 4);
  uint8_t* heap8 = reinterpret_cast<uint8_t*>(initialHeapData_.data());
  std::vector<uint8_t> src8 = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
                               0x98, 0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01};
  fillHeap<uint8_t>(heap8, src8, SVL / 4);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    ptrue p0.b
    pfalse p1.b
    zip1 p1.b, p0.b, p1.b

    mov w12, #0
    dup z0.b, #1
    dup z1.b, #2
    dup z2.b, #3
    dup z3.b, #4

    # Horizontal
    ld1b {za0h.b[w12, #0]}, p0/z, [x0]
    mova z0.b, p0/m, za0h.b[w12, #0]
    mova z1.b, p1/m, za0h.b[w12, #0]

    # Vertical
    ld1b {za0v.b[w12, #3]}, p0/z, [x0]
    # mova z2.b, p0/m, za0v.b[w12, #3]
    # mova z3.b, p1/m, za0v.b[w12, #3]
  )");
  CHECK_NEON(0, uint8_t,
             fillNeon<uint8_t>({0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
                                0x98, 0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01},
                               SVL / 8));
  CHECK_NEON(1, uint8_t,
             fillNeon<uint8_t>({0xDE, 2, 0xBE, 2, 0x12, 2, 0x56, 2, 0x98, 2,
                                0x54, 2, 0xAB, 2, 0xEF, 2},
                               SVL / 8));
  // CHECK_NEON(2, uint8_t,
  //            fillNeon<uint8_T>({0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56,
  //            0x78,
  //                               0x98, 0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF,
  //                               0x01},
  //                              SVL / 8));
  // CHECK_NEON(3, uint8_t,
  //            fillNeon<uint8_T>({0xDE, 4, 0xBE, 4, 0x12, 4, 0x56, 4, 0x98, 4,
  //                               0x54, 4, 0xAB, 4, 0xEF, 4},
  //                              SVL / 8));

  // 16-bit
  initialHeapData_.resize(SVL / 4);
  uint16_t* heap16 = reinterpret_cast<uint16_t*>(initialHeapData_.data());
  std::vector<uint16_t> src16 = {0xDEAD, 0xBEEF, 0x1234, 0x5678,
                                 0x9876, 0x5432, 0xABCD, 0xEF01};
  fillHeap<uint16_t>(heap16, src16, SVL / 8);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    ptrue p0.h
    pfalse p1.b
    zip1 p1.h, p0.h, p1.h

    mov w12, #0
    dup z0.h, #1
    dup z1.h, #2
    dup z2.h, #3
    dup z3.h, #4

    # Horizontal
    ld1h {za0h.h[w12, #0]}, p0/z, [x0]
    mova z0.h, p0/m, za0h.h[w12, #0]
    mova z1.h, p1/m, za0h.h[w12, #0]

    # Vertical
    ld1h {za0v.h[w12, #3]}, p0/z, [x0]
    # mova z2.h, p0/m, za0v.h[w12, #3]
    # mova z3.h, p1/m, za0v.h[w12, #3]
  )");
  CHECK_NEON(0, uint16_t,
             fillNeon<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678, 0x9876, 0x5432,
                                 0xABCD, 0xEF01},
                                SVL / 8));
  CHECK_NEON(1, uint16_t,
             fillNeon<uint16_t>({0xDEAD, 2, 0x1234, 2, 0x9876, 2, 0xABCD, 2},
                                SVL / 8));
  // CHECK_NEON(2, uint16_t,
  //            fillNeon<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678, 0x9876,
  //            0x5432,
  //                                0xABCD, 0xEF01},
  //                               SVL / 8));
  // CHECK_NEON(3, uint16_t,
  //            fillNeon<uint16_t>({0xDEAD, 4, 0x1234, 4, 0x9876, 4, 0xABCD, 4},
  //                               SVL / 8));

  // 32-bit
  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32 = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src32 = {0xDEADBEEF, 0x12345678, 0x98765432,
                                 0xABCDEF01};
  fillHeap<uint32_t>(heap32, src32, SVL / 16);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    ptrue p0.s
    pfalse p1.b
    zip1 p1.s, p0.s, p1.s

    mov w12, #0
    dup z0.s, #1
    dup z1.s, #2
    dup z2.s, #3
    dup z3.s, #4

    # Horizontal
    ld1w {za0h.s[w12, #0]}, p0/z, [x0]
    mova z0.s, p0/m, za0h.s[w12, #0]
    mova z1.s, p1/m, za0h.s[w12, #0]

    # Vertical
    ld1w {za0v.s[w12, #3]}, p0/z, [x0]
    # mova z2.s, p0/m, za0v.s[w12, #3]
    # mova z3.s, p1/m, za0v.s[w12, #3]
  )");
  CHECK_NEON(0, uint32_t,
             fillNeon<uint32_t>(
                 {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01}, SVL / 8));
  CHECK_NEON(1, uint32_t,
             fillNeon<uint32_t>({0xDEADBEEF, 2, 0x98765432, 2}, SVL / 8));
  // CHECK_NEON(
  //     2, uint32_t,
  //     fillNeon<uint32_t>(
  //         {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01}, SVL /
  //         8));
  // CHECK_NEON(3, uint32_t,
  //            fillNeon<uint32_t>({0xDEADBEEF, 4, 0x98765432, 4}, SVL /
  //            8));

  // 64-bit
  initialHeapData_.resize(SVL / 4);
  uint64_t* heap64 = reinterpret_cast<uint64_t*>(initialHeapData_.data());
  std::vector<uint64_t> src64 = {0xDEADBEEF12345678, 0x98765432ABCDEF01};
  fillHeap<uint64_t>(heap64, src64, SVL / 32);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    ptrue p0.d
    pfalse p1.b
    zip1 p1.d, p0.d, p1.d

    mov w12, #0
    dup z0.d, #1
    dup z1.d, #2
    dup z2.d, #3
    dup z3.d, #4

    # Horizontal
    ld1d {za0h.d[w12, #0]}, p0/z, [x0]
    mova z0.d, p0/m, za0h.d[w12, #0]
    mova z1.d, p1/m, za0h.d[w12, #0]

    # Vertical
    ld1d {za0v.d[w12, #1]}, p0/z, [x0]
    # mova z2.d, p0/m, za0v.d[w12, #1]
    # mova z3.d, p1/m, za0v.d[w12, #1]
  )");
  CHECK_NEON(
      0, uint64_t,
      fillNeon<uint64_t>({0xDEADBEEF12345678, 0x98765432ABCDEF01}, SVL / 8));
  CHECK_NEON(1, uint64_t, fillNeon<uint64_t>({0xDEADBEEF12345678, 2}, SVL / 8));
  // CHECK_NEON(
  //     2, uint64_t,
  //     fillNeon<uint64_t>({0xDEADBEEF12345678, 0x98765432ABCDEF01}, SVL / 8));
  // CHECK_NEON(3, uint64_t, fillNeon<uint64_t>({0xDEADBEEF12345678, 4}, SVL /
  // 8));
}

TEST_P(InstSme, fmopa) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    fdup z1.s, #2.0
    fdup z2.s, #5.0
    ptrue p0.s
    ptrue p1.s

    zero {za}

    fmopa za0.s, p0/m, p1/m, z1.s, z2.s

    fdup z3.s, #3.0
    fdup z4.s, #8.0
    mov x0, #0
    mov x1, #8
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.s, xzr, x0

    fmopa za2.s, p0/m, p2/m, z3.s, z4.s
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, float,
                  fillNeon<float>({10.0f}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, float,
                  fillNeon<float>({24.0f}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    fdup z1.d, #2.0
    fdup z2.d, #5.0
    ptrue p0.d
    ptrue p1.d

    zero {za}

    fmopa za0.d, p0/m, p1/m, z1.d, z2.d

    fdup z3.d, #3.0
    fdup z4.d, #8.0
    mov x0, #0
    mov x1, #16
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.d, xzr, x0

    fmopa za2.d, p0/m, p2/m, z3.d, z4.d
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, double,
                  fillNeon<double>({10.0}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, double,
                  fillNeon<double>({24.0}, (SVL / 16)));
  }
}

TEST_P(InstSme, fmops) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    fdup z1.s, #2.0
    fdup z2.s, #5.0
    ptrue p0.s
    ptrue p1.s

    zero {za}

    fmops za0.s, p0/m, p1/m, z1.s, z2.s

    fdup z3.s, #3.0
    fdup z4.s, #8.0
    mov x0, #0
    mov x1, #8
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.s, xzr, x0

    fmops za2.s, p0/m, p2/m, z3.s, z4.s
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, float,
                  fillNeon<float>({-10.0f}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, float,
                  fillNeon<float>({-24.0f}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    fdup z1.d, #2.0
    fdup z2.d, #5.0
    ptrue p0.d
    ptrue p1.d

    zero {za}

    fmops za0.d, p0/m, p1/m, z1.d, z2.d

    fdup z3.d, #3.0
    fdup z4.d, #8.0
    mov x0, #0
    mov x1, #16
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.d, xzr, x0

    fmops za2.d, p0/m, p2/m, z3.d, z4.d
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, double,
                  fillNeon<double>({-10.0}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, double,
                  fillNeon<double>({-24.0}, (SVL / 16)));
  }
}

TEST_P(InstSme, ld1b) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint8_t* heap8 = reinterpret_cast<uint8_t*>(initialHeapData_.data());
  std::vector<uint8_t> src = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
                              0x98, 0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01};
  fillHeap<uint8_t>(heap8, src, SVL / 4);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.b
    mov w12, #1
    # Load and broadcast values from heap
    ld1b {za0h.b[w12, 0]}, p0/z, [x0, x1]
    ld1b {za0h.b[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #2
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.b, xzr, x1
    mov w12, #15
    ld1b {za0h.b[w12, 0]}, p1/z, [x0, x2]
  )");
  CHECK_MAT_ROW(
      ARM64_REG_ZAB0, 1, uint8_t,
      fillNeon<uint8_t>({0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98, 0x76,
                         0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01, 0xDE},
                        SVL / 8));
  CHECK_MAT_ROW(
      ARM64_REG_ZAB0, 3, uint8_t,
      fillNeon<uint8_t>({0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98,
                         0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01},
                        SVL / 8));
  CHECK_MAT_ROW(ARM64_REG_ZAB0, 15, uint8_t,
                fillNeonCombined<uint8_t>(
                    {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98, 0x76,
                     0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01},
                    {0}, SVL / 8));

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint8_t* heap8_vert = reinterpret_cast<uint8_t*>(initialHeapData_.data());
  std::vector<uint8_t> src_vert = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34,
                                   0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
                                   0xAB, 0xCD, 0xEF, 0x01};
  fillHeap<uint8_t>(heap8_vert, src_vert, SVL / 4);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.b
    mov w12, #1
    # Load and broadcast values from heap
    ld1b {za0v.b[w12, 0]}, p0/z, [x0, x1]
    ld1b {za0v.b[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #2
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.b, xzr, x1
    mov w12, #15
    ld1b {za0v.b[w12, 0]}, p1/z, [x0, x2]
  )");
  CHECK_MAT_COL(
      ARM64_REG_ZAB0, 1, uint8_t,
      fillNeon<uint8_t>({0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98, 0x76,
                         0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01, 0xDE},
                        SVL / 8));
  CHECK_MAT_COL(
      ARM64_REG_ZAB0, 3, uint8_t,
      fillNeon<uint8_t>({0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98,
                         0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01},
                        SVL / 8));
  CHECK_MAT_COL(ARM64_REG_ZAB0, 15, uint8_t,
                fillNeonCombined<uint8_t>(
                    {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x98, 0x76,
                     0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01},
                    {0}, SVL / 8));
}

TEST_P(InstSme, ld1d) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint64_t* heap64 = reinterpret_cast<uint64_t*>(initialHeapData_.data());
  std::vector<uint64_t> src = {0xDEADBEEF12345678, 0x98765432ABCDEF01};
  fillHeap<uint64_t>(heap64, src, SVL / 32);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.d
    mov w12, #0
    # Load and broadcast values from heap
    ld1d {za0h.d[w12, 0]}, p0/z, [x0, x1, lsl #3]
    ld1d {za0h.d[w12, 1]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #16
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.d, xzr, x1
    ld1d {za1h.d[w12, 1]}, p1/z, [x0, x2, lsl #3]
  )");
  CHECK_MAT_ROW(
      ARM64_REG_ZAD0, 0, uint64_t,
      fillNeon<uint64_t>({0x98765432ABCDEF01, 0xDEADBEEF12345678}, SVL / 8));
  CHECK_MAT_ROW(
      ARM64_REG_ZAD0, 1, uint64_t,
      fillNeon<uint64_t>({0xDEADBEEF12345678, 0x98765432ABCDEF01}, SVL / 8));
  CHECK_MAT_ROW(ARM64_REG_ZAD1, 1, uint64_t,
                fillNeonCombined<uint64_t>(
                    {0xDEADBEEF12345678, 0x98765432ABCDEF01}, {0}, SVL / 8));

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint64_t* heap64_vert = reinterpret_cast<uint64_t*>(initialHeapData_.data());
  std::vector<uint64_t> src_vert = {0xDEADBEEF12345678, 0x98765432ABCDEF01};
  fillHeap<uint64_t>(heap64_vert, src_vert, SVL / 32);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.d
    mov w12, #0
    # Load and broadcast values from heap
    ld1d {za0v.d[w12, 0]}, p0/z, [x0, x1, lsl #3]
    ld1d {za0v.d[w12, 1]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #16
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.d, xzr, x1
    ld1d {za1v.d[w12, 1]}, p1/z, [x0, x2, lsl #3]
  )");
  CHECK_MAT_COL(
      ARM64_REG_ZAD0, 0, uint64_t,
      fillNeon<uint64_t>({0x98765432ABCDEF01, 0xDEADBEEF12345678}, SVL / 8));
  CHECK_MAT_COL(
      ARM64_REG_ZAD0, 1, uint64_t,
      fillNeon<uint64_t>({0xDEADBEEF12345678, 0x98765432ABCDEF01}, SVL / 8));
  CHECK_MAT_COL(ARM64_REG_ZAD1, 1, uint64_t,
                fillNeonCombined<uint64_t>(
                    {0xDEADBEEF12345678, 0x98765432ABCDEF01}, {0}, SVL / 8));
}

TEST_P(InstSme, ld1h) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint16_t* heap16 = reinterpret_cast<uint16_t*>(initialHeapData_.data());
  std::vector<uint16_t> src = {0xDEAD, 0xBEEF, 0x1234, 0x5678,
                               0x9876, 0x5432, 0xABCD, 0xEF01};
  fillHeap<uint16_t>(heap16, src, SVL / 8);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.h
    mov w12, #1
    # Load and broadcast values from heap
    ld1h {za0h.h[w12, 0]}, p0/z, [x0, x1, lsl #1]
    ld1h {za0h.h[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #4
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.h, xzr, x1
    ld1h {za1h.h[w12, 0]}, p1/z, [x0, x2, lsl #1]
  )");
  CHECK_MAT_ROW(ARM64_REG_ZAH0, 1, uint16_t,
                fillNeon<uint16_t>({0xBEEF, 0x1234, 0x5678, 0x9876, 0x5432,
                                    0xABCD, 0xEF01, 0xDEAD},
                                   SVL / 8));
  CHECK_MAT_ROW(ARM64_REG_ZAH0, 3, uint16_t,
                fillNeon<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678, 0x9876,
                                    0x5432, 0xABCD, 0xEF01},
                                   SVL / 8));
  CHECK_MAT_ROW(ARM64_REG_ZAH1, 1, uint16_t,
                fillNeonCombined<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678,
                                            0x9876, 0x5432, 0xABCD, 0xEF01},
                                           {0}, SVL / 8));

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint16_t* heap16_vert = reinterpret_cast<uint16_t*>(initialHeapData_.data());
  std::vector<uint16_t> src_vert = {0xDEAD, 0xBEEF, 0x1234, 0x5678,
                                    0x9876, 0x5432, 0xABCD, 0xEF01};
  fillHeap<uint16_t>(heap16_vert, src_vert, SVL / 8);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.h
    mov w12, #1
    # Load and broadcast values from heap
    ld1h {za0v.h[w12, 0]}, p0/z, [x0, x1, lsl #1]
    ld1h {za0v.h[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #4
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.h, xzr, x1
    ld1h {za1v.h[w12, 0]}, p1/z, [x0, x2, lsl #1]
  )");
  CHECK_MAT_COL(ARM64_REG_ZAH0, 1, uint16_t,
                fillNeon<uint16_t>({0xBEEF, 0x1234, 0x5678, 0x9876, 0x5432,
                                    0xABCD, 0xEF01, 0xDEAD},
                                   SVL / 8));
  CHECK_MAT_COL(ARM64_REG_ZAH0, 3, uint16_t,
                fillNeon<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678, 0x9876,
                                    0x5432, 0xABCD, 0xEF01},
                                   SVL / 8));
  CHECK_MAT_COL(ARM64_REG_ZAH1, 1, uint16_t,
                fillNeonCombined<uint16_t>({0xDEAD, 0xBEEF, 0x1234, 0x5678,
                                            0x9876, 0x5432, 0xABCD, 0xEF01},
                                           {0}, SVL / 8));
}

TEST_P(InstSme, ld1w) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32 = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src = {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01};
  fillHeap<uint32_t>(heap32, src, SVL / 16);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.s
    mov w12, #1
    # Load and broadcast values from heap
    ld1w {za0h.s[w12, 0]}, p0/z, [x0, x1, lsl #2]
    ld1w {za0h.s[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #8
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.s, xzr, x1
    ld1w {za1h.s[w12, 0]}, p1/z, [x0, x2, lsl #2]
  )");
  CHECK_MAT_ROW(
      ARM64_REG_ZAS0, 1, uint64_t,
      fillNeon<uint64_t>({0x9876543212345678, 0xDEADBEEFABCDEF01}, SVL / 8));
  CHECK_MAT_ROW(
      ARM64_REG_ZAS0, 3, uint64_t,
      fillNeon<uint64_t>({0x12345678DEADBEEF, 0xABCDEF0198765432}, SVL / 8));
  CHECK_MAT_ROW(ARM64_REG_ZAS1, 1, uint64_t,
                fillNeonCombined<uint64_t>(
                    {0x12345678DEADBEEF, 0xABCDEF0198765432}, {0}, SVL / 8));

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32_vert = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src_vert = {0xDEADBEEF, 0x12345678, 0x98765432,
                                    0xABCDEF01};
  fillHeap<uint32_t>(heap32_vert, src_vert, SVL / 16);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x1, #1
    ptrue p0.s
    mov w12, #1
    # Load and broadcast values from heap
    ld1w {za0v.s[w12, 0]}, p0/z, [x0, x1, lsl #2]
    ld1w {za0v.s[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #8
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.s, xzr, x1
    ld1w {za1v.s[w12, 0]}, p1/z, [x0, x2, lsl #2]
  )");
  CHECK_MAT_COL(ARM64_REG_ZAS0, 1, uint32_t,
                fillNeon<uint32_t>(
                    {0x12345678, 0x98765432, 0xABCDEF01, 0xDEADBEEF}, SVL / 8));
  CHECK_MAT_COL(ARM64_REG_ZAS0, 3, uint32_t,
                fillNeon<uint32_t>(
                    {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01}, SVL / 8));
  CHECK_MAT_COL(
      ARM64_REG_ZAS1, 1, uint32_t,
      fillNeonCombined<uint32_t>(
          {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01}, {0}, SVL / 8));
}

TEST_P(InstSme, smopa) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    smopa za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #7
    dup z4.b, #4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    smopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    smopa za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #7
    dup z4.h, #4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    smopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({112}, (SVL / 16)));
  }
}

TEST_P(InstSme, smops) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    smops za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #7
    dup z4.b, #4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    smops za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({-112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    smops za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #7
    dup z4.h, #4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    smops za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({-112}, (SVL / 16)));
  }
}

TEST_P(InstSme, st1b) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint8_t* heap8 = reinterpret_cast<uint8_t*>(initialHeapData_.data());
  std::vector<uint8_t> src = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
                              0x98, 0x76, 0x54, 0x32, 0xAB, 0xCD, 0xEF, 0x01};
  fillHeap<uint8_t>(heap8, src, SVL / 4);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.b

    mov w12, #0
    ld1b {za0h.b[w12, 0]}, p0/z, [x0, x1]
    ld1b {za0h.b[w12, 3]}, p0/z, [x0, x1]
    st1b {za0h.b[w12, 0]}, p0, [sp, x1]
    st1b {za0h.b[w12, 3]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 8); i++) {
    EXPECT_EQ(
        getMemoryValue<uint8_t>(process_->getInitialStackPointer() - 4095 + i),
        src[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>((SVL / 8) + i), src[i % 16]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #16
    ptrue p0.b
    pfalse p1.b
    zip1 p1.b, p0.b, p1.b
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1b {za0h.b[w12, 0]}, p0/z, [x0, x3]
    # Store all 0s to memory
    st1b {za0h.b[w12, 5]}, p0, [x5]
    # Store odd indexed elements to memory
    st1b {za0h.b[w12, 0]}, p1, [x5]

    # Load entire row
    ld1b {za0h.b[w13, 1]}, p0/z, [x0, x3]
    # Store all 0s to memory
    st1b {za0h.b[w12, 5]}, p0, [x6, x3]
    # Store odd indexed elements to memory
    st1b {za0h.b[w13, 1]}, p1, [x6, x3]
  )");
  for (uint64_t i = 0; i < (SVL / 8); i += 2) {
    EXPECT_EQ(getMemoryValue<uint8_t>(400 + i), src[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>(400 + (i + 1)), 0);
    EXPECT_EQ(getMemoryValue<uint8_t>(800 + 16 + i), src[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>(800 + 16 + (i + 1)), 0);
  }

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint8_t* heap8_vert = reinterpret_cast<uint8_t*>(initialHeapData_.data());
  std::vector<uint8_t> src_vert = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34,
                                   0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
                                   0xAB, 0xCD, 0xEF, 0x01};
  fillHeap<uint8_t>(heap8_vert, src_vert, SVL / 4);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.b

    mov w12, #0
    ld1b {za0v.b[w12, 0]}, p0/z, [x0, x1]
    ld1b {za0v.b[w12, 1]}, p0/z, [x0, x1]
    st1b {za0v.b[w12, 0]}, p0, [sp, x1]
    st1b {za0v.b[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 8); i++) {
    EXPECT_EQ(
        getMemoryValue<uint8_t>(process_->getInitialStackPointer() - 4095 + i),
        src_vert[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>((SVL / 8) + i), src_vert[i % 16]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #16
    ptrue p0.b
    pfalse p1.b
    zip1 p1.b, p0.b, p1.b
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1b {za0v.b[w12, 0]}, p0/z, [x0, x3]
    # Store all 0s to memory
    st1b {za0v.b[w12, 5]}, p0, [x5]
    # Store odd indexed elements to memory
    st1b {za0v.b[w12, 0]}, p1, [x5]

    # Load entire row
    ld1b {za0v.b[w13, 1]}, p0/z, [x0, x3]
    # Store all 0s to memory
    st1b {za0v.b[w12, 5]}, p0, [x6, x3]
    # Store odd indexed elements to memory
    st1b {za0v.b[w13, 1]}, p1, [x6, x3]
  )");
  for (uint64_t i = 0; i < (SVL / 8); i += 2) {
    EXPECT_EQ(getMemoryValue<uint8_t>(400 + i), src[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>(400 + (i + 1)), 0);
    EXPECT_EQ(getMemoryValue<uint8_t>(800 + 16 + i), src[i % 16]);
    EXPECT_EQ(getMemoryValue<uint8_t>(800 + 16 + (i + 1)), 0);
  }
}

TEST_P(InstSme, st1d) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint64_t* heap64 = reinterpret_cast<uint64_t*>(initialHeapData_.data());
  std::vector<uint64_t> src = {0xDEADBEEF12345678, 0x98765432ABCDEF01};
  fillHeap<uint64_t>(heap64, src, SVL / 32);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.d

    mov w12, #0
    ld1d {za0h.d[w12, 0]}, p0/z, [x0, x1, lsl #3]
    ld1d {za1h.d[w12, 1]}, p0/z, [x0, x1, lsl #3]
    st1d {za0h.d[w12, 0]}, p0, [sp, x1, lsl #3]
    st1d {za1h.d[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    EXPECT_EQ(getMemoryValue<uint64_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 8)),
              src[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>((SVL / 8) + (i * 8)), src[i % 2]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #2
    ptrue p0.d
    pfalse p1.b
    zip1 p1.d, p0.d, p1.d
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1d {za3h.d[w12, 0]}, p0/z, [x0, x3, lsl #3]
    # Store all 0s to memory
    st1d {za0h.d[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1d {za3h.d[w12, 0]}, p1, [x5]

    # Load entire row
    ld1d {za1h.d[w13, 1]}, p0/z, [x0, x3, lsl #3]
    # Store all 0s to memory
    st1d {za0h.d[w12, 0]}, p0, [x6, x3, lsl #3]
    # Store odd indexed elements to memory
    st1d {za1h.d[w13, 1]}, p1, [x6, x3, lsl #3]
  )");
  for (uint64_t i = 0; i < (SVL / 64); i += 2) {
    EXPECT_EQ(getMemoryValue<uint64_t>(400 + (i * 8)), src[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>(400 + ((i + 1) * 8)), 0);
    EXPECT_EQ(getMemoryValue<uint64_t>(800 + 16 + (i * 8)), src[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>(800 + 16 + ((i + 1) * 8)), 0);
  }

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint64_t* heap64_vert = reinterpret_cast<uint64_t*>(initialHeapData_.data());
  std::vector<uint64_t> src_vert = {0xDEADBEEF12345678, 0x98765432ABCDEF01};
  fillHeap<uint64_t>(heap64_vert, src_vert, SVL / 32);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.d

    mov w12, #0
    ld1d {za0v.d[w12, 0]}, p0/z, [x0, x1, lsl #3]
    ld1d {za1v.d[w12, 1]}, p0/z, [x0, x1, lsl #3]
    st1d {za0v.d[w12, 0]}, p0, [sp, x1, lsl #3]
    st1d {za1v.d[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    EXPECT_EQ(getMemoryValue<uint64_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 8)),
              src_vert[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>((SVL / 8) + (i * 8)), src_vert[i % 2]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #2
    ptrue p0.d
    pfalse p1.b
    zip1 p1.d, p0.d, p1.d
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1d {za3v.d[w12, 0]}, p0/z, [x0, x3, lsl #3]
    # Store all 0s to memory
    st1d {za0v.d[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1d {za3v.d[w12, 0]}, p1, [x5]

    # Load entire row
    ld1d {za1v.d[w13, 1]}, p0/z, [x0, x3, lsl #3]
    # Store all 0s to memory
    st1d {za0v.d[w12, 0]}, p0, [x6, x3, lsl #3]
    # Store odd indexed elements to memory
    st1d {za1v.d[w13, 1]}, p1, [x6, x3, lsl #3]
  )");
  for (uint64_t i = 0; i < (SVL / 64); i += 2) {
    EXPECT_EQ(getMemoryValue<uint64_t>(400 + (i * 8)), src_vert[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>(400 + ((i + 1) * 8)), 0);
    EXPECT_EQ(getMemoryValue<uint64_t>(800 + 16 + (i * 8)), src_vert[i % 2]);
    EXPECT_EQ(getMemoryValue<uint64_t>(800 + 16 + ((i + 1) * 8)), 0);
  }
}

TEST_P(InstSme, st1h) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint16_t* heap16 = reinterpret_cast<uint16_t*>(initialHeapData_.data());
  std::vector<uint16_t> src = {0xDEAD, 0xBEEF, 0x1234, 0x5678,
                               0x9876, 0x5432, 0xABCD, 0xEF01};
  fillHeap<uint16_t>(heap16, src, SVL / 8);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.h

    mov w12, #0
    ld1h {za0h.h[w12, 0]}, p0/z, [x0, x1, lsl #1]
    ld1h {za1h.h[w12, 1]}, p0/z, [x0, x1, lsl #1]
    st1h {za0h.h[w12, 0]}, p0, [sp, x1, lsl #1]
    st1h {za1h.h[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 16); i++) {
    EXPECT_EQ(getMemoryValue<uint16_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 2)),
              src[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>((SVL / 8) + (i * 2)), src[i % 8]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #8
    ptrue p0.h
    pfalse p1.b
    zip1 p1.h, p0.h, p1.h
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1h {za0h.h[w12, 0]}, p0/z, [x0, x3, lsl #1]
    # Store all 0s to memory
    st1h {za1h.h[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1h {za0h.h[w12, 0]}, p1, [x5]

    # Load entire row
    ld1h {za0h.h[w13, 1]}, p0/z, [x0, x3, lsl #1]
    # Store all 0s to memory
    st1h {za1h.h[w12, 0]}, p0, [x6, x3, lsl #1]
    # Store odd indexed elements to memory
    st1h {za0h.h[w13, 1]}, p1, [x6, x3, lsl #1]
  )");
  for (uint64_t i = 0; i < (SVL / 16); i += 2) {
    EXPECT_EQ(getMemoryValue<uint16_t>(400 + (i * 2)), src[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>(400 + ((i + 1) * 2)), 0);
    EXPECT_EQ(getMemoryValue<uint16_t>(800 + 16 + (i * 2)), src[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>(800 + 16 + ((i + 1) * 2)), 0);
  }

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint16_t* heap16_vert = reinterpret_cast<uint16_t*>(initialHeapData_.data());
  std::vector<uint16_t> src_vert = {0xDEAD, 0xBEEF, 0x1234, 0x5678,
                                    0x9876, 0x5432, 0xABCD, 0xEF01};
  fillHeap<uint16_t>(heap16_vert, src_vert, SVL / 8);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.h

    mov w12, #0
    ld1h {za0v.h[w12, 0]}, p0/z, [x0, x1, lsl #1]
    ld1h {za1v.h[w12, 1]}, p0/z, [x0, x1, lsl #1]
    st1h {za0v.h[w12, 0]}, p0, [sp, x1, lsl #1]
    st1h {za1v.h[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 16); i++) {
    EXPECT_EQ(getMemoryValue<uint16_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 2)),
              src_vert[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>((SVL / 8) + (i * 2)), src_vert[i % 8]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #8
    ptrue p0.h
    pfalse p1.b
    zip1 p1.h, p0.h, p1.h
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1h {za0v.h[w12, 0]}, p0/z, [x0, x3, lsl #1]
    # Store all 0s to memory
    st1h {za1v.h[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1h {za0v.h[w12, 0]}, p1, [x5]

    # Load entire row
    ld1h {za0v.h[w13, 1]}, p0/z, [x0, x3, lsl #1]
    # Store all 0s to memory
    st1h {za1v.h[w12, 0]}, p0, [x6, x3, lsl #1]
    # Store odd indexed elements to memory
    st1h {za0v.h[w13, 1]}, p1, [x6, x3, lsl #1]
  )");
  for (uint64_t i = 0; i < (SVL / 16); i += 2) {
    EXPECT_EQ(getMemoryValue<uint16_t>(400 + (i * 2)), src[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>(400 + ((i + 1) * 2)), 0);
    EXPECT_EQ(getMemoryValue<uint16_t>(800 + 16 + (i * 2)), src[i % 8]);
    EXPECT_EQ(getMemoryValue<uint16_t>(800 + 16 + ((i + 1) * 2)), 0);
  }
}

TEST_P(InstSme, st1w) {
  // Horizontal
  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32 = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src = {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01};
  fillHeap<uint32_t>(heap32, src, SVL / 16);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.s

    mov w12, #0
    ld1w {za0h.s[w12, 0]}, p0/z, [x0, x1, lsl #2]
    ld1w {za1h.s[w12, 1]}, p0/z, [x0, x1, lsl #2]
    st1w {za0h.s[w12, 0]}, p0, [sp, x1, lsl #2]
    st1w {za1h.s[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    EXPECT_EQ(getMemoryValue<uint32_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 4)),
              src[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>((SVL / 8) + (i * 4)), src[i % 4]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #4
    ptrue p0.s
    pfalse p1.b
    zip1 p1.s, p0.s, p1.s
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1w {za3h.s[w12, 0]}, p0/z, [x0, x3, lsl #2]
    # Store all 0s to memory
    st1w {za0h.s[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1w {za3h.s[w12, 0]}, p1, [x5]

    # Load entire row
    ld1w {za1h.s[w13, 1]}, p0/z, [x0, x3, lsl #2]
    # Store all 0s to memory
    st1w {za0h.s[w12, 0]}, p0, [x6, x3, lsl #2]
    # Store odd indexed elements to memory
    st1w {za1h.s[w13, 1]}, p1, [x6, x3, lsl #2]
  )");
  for (uint64_t i = 0; i < (SVL / 32); i += 2) {
    EXPECT_EQ(getMemoryValue<uint32_t>(400 + (i * 4)), src[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>(400 + ((i + 1) * 4)), 0);
    EXPECT_EQ(getMemoryValue<uint32_t>(800 + 16 + (i * 4)), src[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>(800 + 16 + ((i + 1) * 4)), 0);
  }

  // Vertical
  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32_vert = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src_vert = {0xDEADBEEF, 0x12345678, 0x98765432,
                                    0xABCDEF01};
  fillHeap<uint32_t>(heap32_vert, src_vert, SVL / 16);

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    sub sp, sp, #4095
    mov x1, #0
    mov x4, #0
    addvl x4, x4, #1
    ptrue p0.s

    mov w12, #0
    ld1w {za0v.s[w12, 0]}, p0/z, [x0, x1, lsl #2]
    ld1w {za1v.s[w12, 1]}, p0/z, [x0, x1, lsl #2]
    st1w {za0v.s[w12, 0]}, p0, [sp, x1, lsl #2]
    st1w {za1v.s[w12, 1]}, p0, [x4]
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    EXPECT_EQ(getMemoryValue<uint32_t>(process_->getInitialStackPointer() -
                                       4095 + (i * 4)),
              src_vert[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>((SVL / 8) + (i * 4)), src_vert[i % 4]);
  }

  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    zero {za}

    mov x3, #4
    ptrue p0.s
    pfalse p1.b
    zip1 p1.s, p0.s, p1.s
    mov x5, #400
    mov x6, #800

    mov w12, #0
    mov w13, #1
    # Load entire row
    ld1w {za3v.s[w12, 0]}, p0/z, [x0, x3, lsl #2]
    # Store all 0s to memory
    st1w {za0v.s[w12, 0]}, p0, [x5]
    # Store odd indexed elements to memory
    st1w {za3v.s[w12, 0]}, p1, [x5]

    # Load entire row
    ld1w {za1v.s[w13, 1]}, p0/z, [x0, x3, lsl #2]
    # Store all 0s to memory
    st1w {za0v.s[w12, 0]}, p0, [x6, x3, lsl #2]
    # Store odd indexed elements to memory
    st1w {za1v.s[w13, 1]}, p1, [x6, x3, lsl #2]
  )");
  for (uint64_t i = 0; i < (SVL / 32); i += 2) {
    EXPECT_EQ(getMemoryValue<uint32_t>(400 + (i * 4)), src_vert[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>(400 + ((i + 1) * 4)), 0);
    EXPECT_EQ(getMemoryValue<uint32_t>(800 + 16 + (i * 4)), src_vert[i % 4]);
    EXPECT_EQ(getMemoryValue<uint32_t>(800 + 16 + ((i + 1) * 4)), 0);
  }
}

TEST_P(InstSme, str) {
  RUN_AARCH64(R"(
    smstart

    zero {za}

    dup z0.b, #2
    dup z1.b, #5
    ptrue p0.b
    ptrue p1.b

    # Fill first 32-bit ZA tile with 40 in every element
    umopa za0.s, p0/m, p1/m, z0.b, z1.b

    dup z0.b, #1
    dup z1.b, #5

    # Fill third 32-bit ZA tile with 20 in every element
    umopa za2.s, p0/m, p1/m, z0.b, z1.b

    mov x2, #600
    mov w12, #0

    # ZA sub tiles are interleaved, so 0th, 4th, 8th... rows will be for za0.s
    # 2nd, 6th, 10th ... rows will be for za2.s
    str za[w12, #0], [x2]
    str za[w12, #1], [x2, #1, mul vl]
    str za[w12, #2], [x2, #2, mul vl]
    str za[w12, #3], [x2, #3, mul vl]
    
    # Store 8th row (3rd row of za0.s)
    add w12, w12, #8
    mov x3, #0
    addvl x3, x3, #4
    add x2, x2, x3
    str za[w12, #0], [x2]

    # Store 10th row (3rd row of za2.s)
    add w12, w12, #2
    mov x3, #0
    addvl x3, x3, #1
    add x2, x2, x3
    str za[w12, #0], [x2]
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, uint32_t,
                  fillNeon<uint32_t>({40}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS1, i, uint32_t,
                  fillNeon<uint32_t>({0}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, uint32_t,
                  fillNeon<uint32_t>({20}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS3, i, uint32_t,
                  fillNeon<uint32_t>({0}, (SVL / 8)));
  }
  const uint64_t SVL_bytes = SVL / 8;
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    const uint64_t off = i * sizeof(uint32_t);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + off), 40);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + SVL_bytes + off), 0);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + (2 * SVL_bytes) + off), 20);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + (3 * SVL_bytes) + off), 0);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + (4 * SVL_bytes) + off), 40);
    EXPECT_EQ(getMemoryValue<uint32_t>(600 + (5 * SVL_bytes) + off), 20);
  }
}

TEST_P(InstSme, sumopa) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #-8
    dup z2.b, #3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    sumopa za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #-7
    dup z4.b, #4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    sumopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({-112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is signed, z2 is unsigned so will become 255
    dup z1.b, #3
    dup z2.b, #-1
    ptrue p0.b
    ptrue p1.b

    zero {za}

    sumopa za0.s, p0/m, p1/m, z1.b, z2.b

    # z3 is signed, z4 is unsigned so will become 254
    dup z3.b, #7
    dup z4.b, #-2
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    sumopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({3060}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({7112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #-8
    dup z2.h, #3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    sumopa za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #-7
    dup z4.h, #4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    sumopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({-112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is signed, z2 is unsigned so will become 65535
    dup z1.h, #3
    dup z2.h, #-1
    ptrue p0.h
    ptrue p1.h

    zero {za}

    sumopa za0.d, p0/m, p1/m, z1.h, z2.h

    # z3 is signed, z4 is unsigned so will become 65534
    dup z3.h, #7
    dup z4.h, #-2
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    sumopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({786420}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({1834952}, (SVL / 16)));
  }
}

TEST_P(InstSme, sumops) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #-8
    dup z2.b, #3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    sumops za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #-7
    dup z4.b, #4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    sumops za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is signed, z2 is unsigned so will become 255
    dup z1.b, #3
    dup z2.b, #-1
    ptrue p0.b
    ptrue p1.b

    zero {za}

    sumops za0.s, p0/m, p1/m, z1.b, z2.b

    # z3 is signed, z4 is unsigned so will become 254
    dup z3.b, #7
    dup z4.b, #-2
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    sumops za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({-3060}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({-7112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #-8
    dup z2.h, #3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    sumops za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #-7
    dup z4.h, #4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    sumops za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is signed, z2 is unsigned so will become 255
    dup z1.h, #3
    dup z2.h, #-1
    ptrue p0.h
    ptrue p1.h

    zero {za}

    sumops za0.d, p0/m, p1/m, z1.h, z2.h

    # z3 is signed, z4 is unsigned so will become 254
    dup z3.h, #7
    dup z4.h, #-2
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    sumops za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({-786420}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({-1834952}, (SVL / 16)));
  }
}

TEST_P(InstSme, umopa) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    umopa za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #7
    dup z4.b, #4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    umopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, uint32_t,
                  fillNeon<uint32_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, uint32_t,
                  fillNeon<uint32_t>({112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    umopa za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #7
    dup z4.h, #4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    umopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, uint64_t,
                  fillNeon<uint64_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, uint64_t,
                  fillNeon<uint64_t>({112}, (SVL / 16)));
  }
}

TEST_P(InstSme, umops) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #3
    dup z3.b, #2
    ptrue p0.b
    ptrue p1.b

    zero {za}

    umopa za0.s, p0/m, p1/m, z1.b, z2.b
    umops za0.s, p0/m, p1/m, z1.b, z3.b

    dup z3.b, #7
    dup z4.b, #4
    dup z5.b, #3
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    umopa za2.s, p0/m, p2/m, z3.b, z4.b
    umops za2.s, p0/m, p2/m, z3.b, z5.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, uint32_t,
                  fillNeon<uint32_t>({32}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, uint32_t,
                  fillNeon<uint32_t>({28}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #3
    dup z3.h, #2
    ptrue p0.h
    ptrue p1.h

    zero {za}

    umopa za0.d, p0/m, p1/m, z1.h, z2.h
    umops za0.d, p0/m, p1/m, z1.h, z3.h

    dup z3.h, #7
    dup z4.h, #4
    dup z5.h, #3
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    umopa za2.d, p0/m, p2/m, z3.h, z4.h
    umops za2.d, p0/m, p2/m, z3.h, z5.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, uint64_t,
                  fillNeon<uint64_t>({32}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, uint64_t,
                  fillNeon<uint64_t>({28}, (SVL / 16)));
  }
}

TEST_P(InstSme, usmopa) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #-3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    usmopa za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #7
    dup z4.b, #-4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    usmopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({-112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is unsigned so will become 253, z2 is signed
    dup z1.b, #-3
    dup z2.b, #2
    ptrue p0.b
    ptrue p1.b

    zero {za}

    usmopa za0.s, p0/m, p1/m, z1.b, z2.b

    # z3 is unsigned so will become 254, z4 is unsigned
    dup z3.b, #-2
    dup z4.b, #7
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    usmopa za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({2024}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({7112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #-3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    usmopa za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #7
    dup z4.h, #-4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    usmopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({-96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({-112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is unsigned so will become 65533, z2 is unsigned
    dup z1.h, #-3
    dup z2.h, #2
    ptrue p0.h
    ptrue p1.h

    zero {za}

    usmopa za0.d, p0/m, p1/m, z1.h, z2.h

    # z3 is unsigned so will become 65534, z4 is signed
    dup z3.h, #-2
    dup z4.h, #7
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    usmopa za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({524264}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({1834952}, (SVL / 16)));
  }
}

TEST_P(InstSme, usmops) {
  // 32-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.b, #8
    dup z2.b, #-3
    ptrue p0.b
    ptrue p1.b

    zero {za}

    usmops za0.s, p0/m, p1/m, z1.b, z2.b

    dup z3.b, #7
    dup z4.b, #-4
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    usmops za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is unsigned so will become 253, z2 is signed
    dup z1.b, #-3
    dup z2.b, #2
    ptrue p0.b
    ptrue p1.b

    zero {za}

    usmops za0.s, p0/m, p1/m, z1.b, z2.b

    # z3 is unsigned so will become 254, z4 is signed
    dup z3.b, #-2
    dup z4.b, #7
    mov x0, #0
    mov x1, #2
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.b, xzr, x0

    usmops za2.s, p0/m, p2/m, z3.b, z4.b
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, int32_t,
                  fillNeon<int32_t>({-2024}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, int32_t,
                  fillNeon<int32_t>({-7112}, (SVL / 16)));
  }

  // 64-bit
  RUN_AARCH64(R"(
    smstart

    dup z1.h, #8
    dup z2.h, #-3
    ptrue p0.h
    ptrue p1.h

    zero {za}

    usmops za0.d, p0/m, p1/m, z1.h, z2.h

    dup z3.h, #7
    dup z4.h, #-4
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    usmops za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({96}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({112}, (SVL / 16)));
  }

  RUN_AARCH64(R"(
    smstart

    # z1 is unsigned so will become 65533, z2 is signed
    dup z1.h, #-3
    dup z2.h, #2
    ptrue p0.h
    ptrue p1.h

    zero {za}

    usmops za0.d, p0/m, p1/m, z1.h, z2.h

    # z3 is unsigned so will become 65534, z4 is signed
    dup z3.h, #-2
    dup z4.h, #7
    mov x0, #0
    mov x1, #4
    addvl x0, x0, #1
    udiv x0, x0, x1
    whilelo p2.h, xzr, x0

    usmops za2.d, p0/m, p2/m, z3.h, z4.h
  )");
  for (uint64_t i = 0; i < (SVL / 64); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAD0, i, int64_t,
                  fillNeon<int64_t>({-524264}, (SVL / 8)));
    CHECK_MAT_ROW(ARM64_REG_ZAD2, i, int64_t,
                  fillNeon<int64_t>({-1834952}, (SVL / 16)));
  }
}

TEST_P(InstSme, zero) {
  RUN_AARCH64(R"(
    smstart

    zero {za}
  )");
  for (uint64_t i = 0; i < (SVL / 8); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZA, i, uint64_t, fillNeon<uint64_t>({0}, SVL / 8));
  }

  initialHeapData_.resize(SVL / 4);
  uint32_t* heap32_vert = reinterpret_cast<uint32_t*>(initialHeapData_.data());
  std::vector<uint32_t> src_vert = {0xDEADBEEF, 0x12345678, 0x98765432,
                                    0xABCDEF01};
  fillHeap<uint32_t>(heap32_vert, src_vert, SVL / 16);
  RUN_AARCH64(R"(
    # Get heap address
    mov x0, 0
    mov x8, 214
    svc #0

    smstart

    mov x1, #1
    ptrue p0.s
    mov w12, #1
    # Load and broadcast values from heap
    ld1w {za0v.s[w12, 0]}, p0/z, [x0, x1, lsl #2]
    ld1w {za1v.s[w12, 2]}, p0/z, [x0]

    # Test for inactive lanes
    mov x1, #0
    mov x3, #8
    # TODO change to addsvl when implemented
    addvl x1, x1, #1
    udiv x1, x1, x3
    mov x2, #0
    whilelo p1.s, xzr, x1
    ld1w {za2v.s[w12, 0]}, p1/z, [x0, x2, lsl #2]

    zero {za0.s, za2.s}
  )");
  for (uint64_t i = 0; i < (SVL / 32); i++) {
    CHECK_MAT_ROW(ARM64_REG_ZAS0, i, uint32_t,
                  fillNeon<uint32_t>({0}, SVL / 8));
    CHECK_MAT_ROW(ARM64_REG_ZAS2, i, uint32_t,
                  fillNeon<uint32_t>({0}, SVL / 8));
  }
  CHECK_MAT_COL(ARM64_REG_ZAS1, 3, uint32_t,
                fillNeon<uint32_t>(
                    {0xDEADBEEF, 0x12345678, 0x98765432, 0xABCDEF01}, SVL / 8));
}

INSTANTIATE_TEST_SUITE_P(AArch64, InstSme,
                         ::testing::ValuesIn(genCoreTypeSVLPairs(EMULATION)),
                         paramToString);
#else
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(InstSme);
#endif

}  // namespace