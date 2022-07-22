/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsgnb/adt/bounded_bitset.h"
#include "srsgnb/support/test_utils.h"

using namespace srsgnb;

void test_bit_operations()
{
  TESTASSERT_EQ(0, mask_lsb_ones<uint8_t>(0));
  TESTASSERT_EQ(0, mask_msb_ones<uint8_t>(0));
  TESTASSERT_EQ(0, mask_lsb_ones<uint32_t>(0));
  TESTASSERT_EQ(0, mask_msb_ones<uint32_t>(0));
  TESTASSERT_EQ(0b11111111, mask_lsb_zeros<uint8_t>(0));
  TESTASSERT_EQ(0b11111111, mask_msb_zeros<uint8_t>(0));
  TESTASSERT((uint32_t)-1 == mask_lsb_zeros<uint32_t>(0));
  TESTASSERT((uint32_t)-1 == mask_msb_zeros<uint32_t>(0));

  TESTASSERT(0b11 == mask_lsb_ones<uint8_t>(2));
  TESTASSERT(0b11000000 == mask_msb_ones<uint8_t>(2));
  TESTASSERT(0b11111100 == mask_lsb_zeros<uint8_t>(2));
  TESTASSERT(0b00111111 == mask_msb_zeros<uint8_t>(2));

  TESTASSERT(0b11111111 == mask_lsb_ones<uint8_t>(8));
  TESTASSERT(0b1111 == mask_lsb_ones<uint8_t>(4));

  TESTASSERT(find_first_lsb_one<uint8_t>(0) == std::numeric_limits<uint8_t>::digits);
  TESTASSERT(find_first_msb_one<uint8_t>(0) == std::numeric_limits<uint8_t>::digits);
  TESTASSERT(find_first_lsb_one<uint8_t>(1) == 0);
  TESTASSERT(find_first_msb_one<uint8_t>(1) == 0);
  TESTASSERT(find_first_lsb_one<uint8_t>(0b11) == 0);
  TESTASSERT(find_first_lsb_one<uint8_t>(0b10) == 1);
  TESTASSERT(find_first_msb_one<uint8_t>(0b11) == 1);
  TESTASSERT(find_first_msb_one<uint8_t>(0b10) == 1);
  TESTASSERT(find_first_lsb_one<uint32_t>(0b11) == 0);
  TESTASSERT(find_first_lsb_one<uint32_t>(0b10) == 1);
  TESTASSERT(find_first_msb_one<uint32_t>(0b11) == 1);
  TESTASSERT(find_first_msb_one<uint32_t>(0b10) == 1);
}

void test_zero_bitset()
{
  bounded_bitset<25> mask;
  bounded_bitset<25> mask2(23);

  TESTASSERT(mask.max_size() == 25);
  TESTASSERT(mask.size() == 0);
  TESTASSERT(mask.count() == 0);
  TESTASSERT(mask.none());
  TESTASSERT(not mask.any());
  TESTASSERT(mask.all());
  TESTASSERT(mask != mask2);

  TESTASSERT(mask2.max_size() == 25);
  TESTASSERT(mask2.size() == 23);
  TESTASSERT(mask2.count() == 0);
  TESTASSERT(mask2.none());
  TESTASSERT(not mask2.any());
  TESTASSERT(not mask2.all());

  mask = mask2;
  TESTASSERT(mask == mask2);
}

void test_ones_bitset()
{
  bounded_bitset<25> mask;

  // Flipping empty bitset is noop
  TESTASSERT(mask.none() and mask.all() and not mask.any());
  mask.flip();
  TESTASSERT(mask.none() and mask.all() and not mask.any());

  // Flipping zeros bitset with size>0 will set all bits to one
  mask.resize(23);
  TESTASSERT(mask.none() and not mask.all() and not mask.any());
  mask.flip();
  TESTASSERT(not mask.none() and mask.all() and mask.any());
}

void test_bitset_constructors()
{
  // Test initializer list.
  bounded_bitset<25> mask1 = {1, 1, 1, 1, 0};
  TESTASSERT(mask1.size() == 5);
  TESTASSERT(mask1.test(0));
  TESTASSERT(mask1.test(1));
  TESTASSERT(mask1.test(3));
  TESTASSERT(not mask1.test(4));

  // Test iterator.
  std::array<bool, 5> data = {1, 1, 1, 1, 0};
  bounded_bitset<25>  mask2(data.begin(), data.end());
  TESTASSERT(mask2.size() == 5);
  TESTASSERT(mask2.test(0));
  TESTASSERT(mask2.test(1));
  TESTASSERT(mask2.test(3));
  TESTASSERT(not mask2.test(4));
}

void test_bitset_set()
{
  bounded_bitset<25> mask(23);
  mask.set(10);

  TESTASSERT(mask.any());
  TESTASSERT(not mask.all());
  TESTASSERT(not mask.test(0));
  TESTASSERT(mask.test(10));
  mask.flip();
  TESTASSERT(not mask.test(10));
  TESTASSERT(mask.test(0));
}

void test_bitset_bitwise_oper()
{
  bounded_bitset<25> mask(23);
  bounded_bitset<25> mask2(23);

  mask.set(10);
  TESTASSERT(mask != mask2);
  mask2 |= mask;
  TESTASSERT(mask == mask2);

  mask.set(11);
  mask2 &= mask;
  TESTASSERT(mask != mask2);
  TESTASSERT(mask2.test(10) and not mask2.test(11));

#if EXCEPTIONS_ENABLED
  bool caught = false;
  mask2.resize(24);
  try {
    mask2 |= mask;
  } catch (bad_type_access& c) {
    printf("Received exception \"%s\" (as expected)\n", c.what());
    caught = true;
  }
  TESTASSERT(caught);
#endif
}

void test_bitset_print()
{
  {
    bounded_bitset<100> bitset(100);
    bitset.set(0);
    bitset.set(5);

    TESTASSERT(fmt::format("{:x}", bitset) == "0000000000000000000000021");
    TESTASSERT(fmt::format("{:b}", bitset) ==
               "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100001");

    bitset.set(99);
    TESTASSERT(fmt::format("{:x}", bitset) == "8000000000000000000000021");
    TESTASSERT(fmt::format("{:b}", bitset) ==
               "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100001");
  }

  {
    bounded_bitset<100> bitset(25);
    bitset.set(0);
    bitset.set(4);

    TESTASSERT(fmt::format("{:x}", bitset) == "0000011");
    TESTASSERT(fmt::format("{:b}", bitset) == "0000000000000000000010001");

    bitset.set(24);
    TESTASSERT(fmt::format("{:x}", bitset) == "1000011");
    TESTASSERT(fmt::format("{:b}", bitset) == "1000000000000000000010001");
  }

  {
    bounded_bitset<100, true> bitset(100);
    bitset.set(0);
    bitset.set(5);

    TESTASSERT(fmt::format("{:x}", bitset) == "8400000000000000000000000");
    TESTASSERT(fmt::format("{:b}", bitset) ==
               "1000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    bitset.set(99);
    TESTASSERT(fmt::format("{:x}", bitset) == "8400000000000000000000001");
    TESTASSERT(fmt::format("{:b}", bitset) ==
               "1000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001");
  }
}

void test_bitset_resize()
{
  {
    bounded_bitset<100> bitset;
    TESTASSERT(bitset.none() and bitset.size() == 0);

    bitset.resize(100);
    TESTASSERT(bitset.none() and bitset.size() == 100);
    bitset.fill(0, 100);
    TESTASSERT(bitset.all() and bitset.size() == 100);

    bitset.resize(25);
    TESTASSERT(bitset.to_uint64() == 0x1ffffff);
    TESTASSERT(bitset.all() and bitset.size() == 25); // keeps the data it had for the non-erased bits

    bitset.resize(100);
    TESTASSERT(bitset.count() == 25 and bitset.size() == 100);
  }

  {
    // TEST: Reverse case
    bounded_bitset<100, true> bitset;
    TESTASSERT(bitset.none() and bitset.size() == 0);

    bitset.resize(100);
    TESTASSERT(bitset.none() and bitset.size() == 100);
    bitset.fill(0, 100);
    TESTASSERT(bitset.all() and bitset.size() == 100);

    bitset.resize(25);
    TESTASSERT(bitset.to_uint64() == 0x1ffffff);
    TESTASSERT(bitset.all() and bitset.size() == 25); // keeps the data it had for the non-erased bits

    bitset.resize(100);
    TESTASSERT(bitset.count() == 25 and bitset.size() == 100);
  }
}

template <bool reversed>
void test_bitset_find()
{
  {
    bounded_bitset<25, reversed> bitset(6);

    // 0b000000
    TESTASSERT(bitset.find_lowest(0, bitset.size(), false) == 0);
    TESTASSERT(bitset.find_lowest(0, bitset.size(), true) == -1);

    // 0b000100
    bitset.set(2);
    TESTASSERT(bitset.find_lowest(0, 6) == 2);
    TESTASSERT(bitset.find_lowest(0, 6, false) == 0);
    TESTASSERT(bitset.find_lowest(3, 6) == -1);
    TESTASSERT(bitset.find_lowest(3, 6, false) == 3);

    // 0b000101
    bitset.set(0);
    TESTASSERT(bitset.find_lowest(0, 6) == 0);
    TESTASSERT(bitset.find_lowest(0, 6, false) == 1);
    TESTASSERT(bitset.find_lowest(3, 6) == -1);
    TESTASSERT(bitset.find_lowest(3, 6, false) == 3);

    // 0b100101
    bitset.set(5);
    TESTASSERT(bitset.find_lowest(0, 6) == 0);
    TESTASSERT(bitset.find_lowest(0, 6, false) == 1);
    TESTASSERT(bitset.find_lowest(3, 6) == 5);

    // 0b111111
    bitset.fill(0, 6);
    TESTASSERT(bitset.find_lowest(0, 6) == 0);
    TESTASSERT(bitset.find_lowest(0, 6, false) == -1);
    TESTASSERT(bitset.find_lowest(3, 6, false) == -1);
  }
  {
    bounded_bitset<100, reversed> bitset(95);

    // 0b0...0
    TESTASSERT(bitset.find_lowest(0, bitset.size()) == -1);

    // 0b1000...
    bitset.set(94);
    TESTASSERT(bitset.find_lowest(0, 93) == -1);
    TESTASSERT(bitset.find_lowest(0, bitset.size()) == 94);

    // 0b1000...010
    bitset.set(1);
    TESTASSERT(bitset.find_lowest(0, bitset.size()) == 1);
    TESTASSERT(bitset.find_lowest(1, bitset.size()) == 1);
    TESTASSERT(bitset.find_lowest(2, bitset.size()) == 94);

    // 0b11..11
    bitset.fill(0, bitset.size());
    TESTASSERT(bitset.find_lowest(0, bitset.size()) == 0);
    TESTASSERT(bitset.find_lowest(5, bitset.size()) == 5);
  }
  {
    bounded_bitset<100, reversed> bitset(95);

    // 0b0...0
    TESTASSERT(bitset.find_lowest() == -1);

    // 0b1000...
    bitset.set(94);
    TESTASSERT(bitset.find_lowest() == 94);

    // 0b1000...010
    bitset.set(1);
    TESTASSERT(bitset.find_lowest() == 1);

    // 0b11..11
    bitset.fill(0, bitset.size());
    TESTASSERT(bitset.find_lowest() == 0);
  }
  {
    bounded_bitset<100, reversed> bitset(95);

    // 0b0...0
    TESTASSERT(bitset.find_highest() == -1);

    // 0b1000...
    bitset.set(94);
    TESTASSERT_EQ(bitset.find_highest(), 94);

    // 0b1000...010
    bitset.set(1);
    TESTASSERT(bitset.find_highest() == 94);

    // 0b11..11
    bitset.fill(0, bitset.size());
    TESTASSERT(bitset.find_highest() == 94);
  }
}

void test_bitset_contiguous()
{
  // Test contiguous condition 1. No bit set.
  {
    TESTASSERT(bounded_bitset<10>({0, 0, 0, 0}).is_contiguous());
  }
  // Test contiguous condition 2. One bit set.
  {
    TESTASSERT(bounded_bitset<10>({1, 0, 0, 0}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 0, 1, 0}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 1, 0, 0}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 0, 0, 1}).is_contiguous());
  }
  // Test contiguous condition 3. All set bits are contiguous.
  {
    TESTASSERT(bounded_bitset<10>({1, 1, 0, 0}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({1, 1, 1, 0}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({1, 1, 1, 1}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 1, 1, 1}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 0, 1, 1}).is_contiguous());
    TESTASSERT(bounded_bitset<10>({0, 1, 1, 0}).is_contiguous());
  }
  // Not contiguous.
  {
    TESTASSERT(not bounded_bitset<10>({1, 0, 1, 1}).is_contiguous());
    TESTASSERT(not bounded_bitset<10>({1, 1, 0, 1}).is_contiguous());
    TESTASSERT(not bounded_bitset<10>({1, 0, 1, 1}).is_contiguous());
    TESTASSERT(not bounded_bitset<10>({1, 1, 0, 1}).is_contiguous());
    TESTASSERT(not bounded_bitset<10>({1, 0, 0, 1}).is_contiguous());
  }
}

void test_bitset_push_back()
{
  {
    bounded_bitset<10> bitset;
    bitset.push_back(1);
    bitset.push_back(0);
    bitset.push_back(1);
    bitset.push_back(1);
    TESTASSERT_EQ(bitset, bounded_bitset<10>({1, 0, 1, 1}));
  }
  {
    bounded_bitset<10> bitset;
    bitset.push_back(0xbU, 4);
    TESTASSERT_EQ(bitset, bounded_bitset<10>({1, 0, 1, 1}));
  }
}

void test_bitset_slice()
{
  bounded_bitset<100> big_bitmap(95);
  big_bitmap.fill(5, 70);

  bounded_bitset<35> small_bitmap = big_bitmap.slice<35>(50, 80);
  TESTASSERT_EQ(30, small_bitmap.size());
  TESTASSERT_EQ(20, small_bitmap.count());
  TESTASSERT_EQ(1048575, small_bitmap.to_uint64());
}

void test_bitset_fold_and_accumulate()
{
  size_t fold_size = 20;

  bounded_bitset<105> big_bitmap(100);
  for (size_t i = 0; i < big_bitmap.size(); i += fold_size + 1) {
    big_bitmap.set(i);
  }

  bounded_bitset<6> fold_bitset = fold_and_accumulate<6>(big_bitmap, fold_size);
  TESTASSERT_EQ(big_bitmap.size() / fold_size, fold_bitset.count());
  TESTASSERT_EQ(fold_bitset.count(), big_bitmap.count());
  TESTASSERT(fold_bitset.is_contiguous());
  TESTASSERT_EQ(0, fold_bitset.find_lowest());
}

int main()
{
  test_bit_operations();
  test_zero_bitset();
  test_ones_bitset();
  test_bitset_constructors();
  test_bitset_set();
  test_bitset_bitwise_oper();
  test_bitset_print();
  test_bitset_resize();
  test_bitset_find<false>();
  test_bitset_find<true>();
  test_bitset_contiguous();
  test_bitset_push_back();
  test_bitset_slice();
  test_bitset_fold_and_accumulate();
  printf("Success\n");
  return 0;
}
