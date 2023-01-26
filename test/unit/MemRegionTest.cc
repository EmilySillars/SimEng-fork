#include <sys/mman.h>

#include "gtest/gtest.h"
#include "simeng/kernel/MemRegion.hh"

using namespace simeng::kernel;

namespace {

TEST(MemRegionTest, UpdateBrkRegion) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);
  ASSERT_EQ(memRegion.getBrk(), 0);
  ASSERT_EQ(memRegion.updateBrkRegion(1000), 4096);
  ASSERT_EQ(memRegion.updateBrkRegion(0), 4096);
  ASSERT_EQ(memRegion.updateBrkRegion(10365), 12288);
  ASSERT_EQ(memRegion.updateBrkRegion(81910), 81920);
}

TEST(MemRegionTest, UpdateBrkRegionOnAddrGreaterThanHeapSize) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);
  EXPECT_EXIT(
      { memRegion.updateBrkRegion(81925); }, ::testing::ExitedWithCode(1),
      "Attemped to allocate more memory than is available to the process.");
}

TEST(MemRegionTest, MmapRegionNoStartAddr) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);
}

TEST(MemRegionTest, MultipleMmapRegionNoStartAddr) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);

  retAddr = memRegion.mmapRegion(0, 4000, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 4096);

  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + (4096 * 2));

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + (4096 * 4));

  ASSERT_EQ(memRegion.getVMASize(), 4);
}

TEST(MemRegionTest, MmapRegionStartAddr) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);

  retAddr = memRegion.mmapRegion(mmapStart + 8192, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 8192);
}

TEST(MemRegionTest, MmapRegionUnalignedStartAddr) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);

  retAddr = memRegion.mmapRegion(mmapStart + 8100, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 8192);
}

TEST(MemRegionTest, MmapRegionAllocatesBetweenVmas) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);

  retAddr = memRegion.mmapRegion(mmapStart + 12288, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 12288);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 4096);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 8192);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 12288 + 4096);

  ASSERT_EQ(memRegion.getVMASize(), 5);
}

TEST(MemRegionTest, MmapRegionCorrectlyAllocatesOverlappingVmas) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);

  retAddr = memRegion.mmapRegion(mmapStart + 12288, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 12288);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 4096);

  // Address range mmapStart + 4096, mmapStart + 4096 + 4096) has already been
  // mapped. If hint is provided then allocate at a page aligned address equal
  // or greater than hint.
  retAddr = memRegion.mmapRegion(mmapStart + 4096, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart + 8192);
  ASSERT_GE(retAddr, mmapStart + 4096);

  ASSERT_EQ(memRegion.getVMASize(), 4);
}
/*
 * [-addr]
 * [-vma-)
 */

TEST(MemRegionTest, UnmapVmaHead) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(retAddr, mmapStart);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  uint64_t delSize = memRegion.unmapRegion(mmapStart, 4096);
  ASSERT_EQ(delSize, 4096);
  EXPECT_TRUE(memRegion.getVMAHead() == NULL);
  ASSERT_EQ(memRegion.getVMASize(), 0);
}

/*
 * [-----addr-----]
 * [-vma-)->[-vma-)
 */
TEST(MemRegionTest, UnmapVmaStartGreaterThanPageSize1) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  uint64_t delSize = memRegion.unmapRegion(mmapStart, 8192);
  ASSERT_EQ(delSize, 8192);
  EXPECT_TRUE(memRegion.getVMAHead() == NULL);
  ASSERT_EQ(memRegion.getVMASize(), 0);
}
/*
 * [-----addr-----]
 * [-vma-)->[-vma-)->[-vma-)
 */

TEST(MemRegionTest, UnmapVmaStartGreaterThanPageSize2) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart, 8192);
  ASSERT_EQ(delSize, 8192);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, retAddr);
}

/*
 * [--------addr--------]
 * [-vma-)->[-vma-)->[--vma--)
 */
TEST(MemRegionTest, UnmapOverlappingVmaStartGreaterThanPageSize) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart, 12288);
  ASSERT_EQ(delSize, 12288);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart + 12288);
  EXPECT_TRUE(memRegion.getVMAHead()->vm_next == NULL);
}
/*
 *          [-addr]
 * [-vma-)->[-vma-)->[-vma-)
 */

TEST(MemRegionTest, UnmapContainedInMiddleOfVmaList) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 4096, 4096);
  ASSERT_EQ(delSize, 4096);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 2);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* tail = memRegion.getVMAHead()->vm_next;
  ASSERT_EQ(tail->vm_start, mmapStart + 8192);
  EXPECT_TRUE(tail->vm_next == NULL);
}
/*
 *          [---addr---]
 * [-vma-)->[-vma-)->[--vma--)
 */
TEST(MemRegionTest, UnmapContainedVmaAndOverlapStart) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 4096, 8192);
  ASSERT_EQ(delSize, 8192);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 2);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* tail = memRegion.getVMAHead()->vm_next;
  ASSERT_EQ(tail->vm_start, mmapStart + 12288);
  EXPECT_TRUE(tail->vm_next == NULL);
}

/*
 *              [----addr----]
 * [-vma-)->[--vma--)->[-vma-)
 */
TEST(MemRegionTest, UnmapOverlapStartAndContained) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 8192, 8192);
  ASSERT_EQ(delSize, 8192);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 2);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* tail = memRegion.getVMAHead()->vm_next;
  ASSERT_EQ(tail->vm_start, mmapStart + 4096);
  ASSERT_EQ(tail->vm_end, mmapStart + 8192);
  EXPECT_TRUE(tail->vm_next == NULL);
}

/*
 *      [--------addr--------]
 * [--vma--)->[--vma--)->[--vma--)
 */
TEST(MemRegionTest, UnmapContained) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 4096, 4096 * 4);
  ASSERT_EQ(delSize, 4096 * 4);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 2);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* tail = memRegion.getVMAHead()->vm_next;
  ASSERT_EQ(tail->vm_start, mmapStart + (4096 * 5));
  ASSERT_EQ(tail->vm_end, mmapStart + (4096 * 6));
  EXPECT_TRUE(tail->vm_next == NULL);
}

/*
 *              [-addr]
 * [--vma--)->[---vma---)->[--vma--)
 */
TEST(MemRegionTest, UnmapContainsMiddle) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 12288, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 8192, 4096);
  ASSERT_EQ(delSize, 4096);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 4);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* vma = memRegion.getVMAFromAddr(mmapStart + 4096);
  EXPECT_TRUE(vma != NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + 8192);

  vma = memRegion.getVMAFromAddr(mmapStart + 12288);
  EXPECT_TRUE(vma != NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + 16384);

  vma = memRegion.getVMAFromAddr(mmapStart + 16384);
  EXPECT_TRUE(vma != NULL);
  EXPECT_TRUE(vma->vm_next == NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + (4096 * 5));
}

/*
 *            [-addr]
 * [--vma--)->[--vma--)->[--vma--)
 */
TEST(MemRegionTest, UnmapContainsStart) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 4096, 4096);
  ASSERT_EQ(delSize, 4096);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 3);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* vma = memRegion.getVMAFromAddr(mmapStart + 4096);
  EXPECT_TRUE(vma == NULL);

  vma = memRegion.getVMAFromAddr(mmapStart + 8192);
  EXPECT_TRUE(vma != NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + 12288);

  vma = memRegion.getVMAFromAddr(mmapStart + 12288);
  EXPECT_TRUE(vma != NULL);
  EXPECT_TRUE(vma->vm_next == NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + (4096 * 4));
}

/*
 *              [-addr]
 * [--vma--)->[--vma--)->[--vma--)
 */
TEST(MemRegionTest, UnmapContainsEnd) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_EQ(retAddr, mmapStart + 4096);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 8192, 4096);
  ASSERT_EQ(delSize, 4096);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 3);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* vma = memRegion.getVMAFromAddr(mmapStart + 8192);
  EXPECT_TRUE(vma == NULL);

  vma = memRegion.getVMAFromAddr(mmapStart + 4096);
  EXPECT_TRUE(vma != NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + 8192);

  vma = memRegion.getVMAFromAddr(mmapStart + 12288);
  EXPECT_TRUE(vma != NULL);
  EXPECT_TRUE(vma->vm_next == NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + (4096 * 4));
}

TEST(MemRegionTest, UnmapOverlaps) {
  uint64_t heapStart = 0;
  uint64_t heapSize = 81920;
  uint64_t mmapStart = 86016;
  uint64_t mmapSize = 163840;
  uint64_t stackStart = 294912;
  uint64_t stackSize = 40960;
  uint64_t size = stackStart;
  // heapEnd = 81920;
  // mmapEnd = 249856;
  // stackEnd = 253952;

  MemRegion memRegion = MemRegion(stackSize, heapSize, mmapSize, size, 4096,
                                  stackStart, heapStart, mmapStart, stackStart);

  uint64_t retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 1);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);

  retAddr = memRegion.mmapRegion(0, 8192, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 2);

  retAddr = memRegion.mmapRegion(0, 4096, 0, MAP_PRIVATE, NULL);
  ASSERT_NE(retAddr, 0);
  ASSERT_EQ(memRegion.getVMASize(), 3);

  uint64_t delSize = memRegion.unmapRegion(mmapStart + 4096, 8192);
  ASSERT_EQ(delSize, 8192);
  EXPECT_TRUE(memRegion.getVMAHead() != NULL);
  ASSERT_EQ(memRegion.getVMASize(), 3);
  ASSERT_EQ(memRegion.getVMAHead()->vm_start, mmapStart);

  VMA* vma = memRegion.getVMAFromAddr(mmapStart + 4096);
  EXPECT_TRUE(vma == NULL);

  vma = memRegion.getVMAFromAddr(mmapStart + 8192);
  EXPECT_TRUE(vma == NULL);

  vma = memRegion.getVMAFromAddr(mmapStart + 12288);
  EXPECT_TRUE(vma != NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + 16384);

  vma = memRegion.getVMAFromAddr(mmapStart + 16384);
  EXPECT_TRUE(vma != NULL);
  EXPECT_TRUE(vma->vm_next == NULL);
  ASSERT_EQ(vma->size, 4096);
  ASSERT_EQ(vma->vm_end, mmapStart + (4096 * 5));
}

}  // namespace
