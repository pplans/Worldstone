/**
 * @file BitStreamTests.cpp
 */
#include <BitStream.h>
#include <SystemUtils.h>
#include "doctest.h"

using WorldStone::BitStream;
using WorldStone::Utils::signExtend;

/**Test that SignExtend is giving the right values
 * @testimpl{WorldStone::Utils::signExtend(),SignExtend}
 */
TEST_CASE("SignExtend")
{
// clang-format off
    SUBCASE("Positive input")
    {
        CHECK(signExtend<int32_t,13>(         0) ==          0);
        CHECK(signExtend<int32_t,32>(0x16641337) == 0x16641337);
        CHECK(signExtend<int32_t,32>(0x7FFFFFFF) == 0x7FFFFFFF);
        CHECK(signExtend<int32_t, 2>(      0b11) ==         -1);
    }
    SUBCASE("Negative input")
    {
        CHECK(signExtend<int32_t, 5>(   0b10110) ==        -10);
        CHECK(signExtend<int32_t, 8>(      0xFF) ==         -1);
        CHECK(signExtend<int32_t,32>(0xFFFFFFFF) ==         -1);
        CHECK(signExtend<int32_t,32>(0xDEADBEEF) == 0xDEADBEEF);
        CHECK(signExtend<int32_t,16>(0xDEADBEEF) == 0xFFFFBEEF);
        CHECK(signExtend<int32_t,16>(0x16641337) ==     0x1337);
        CHECK(signExtend<int32_t, 8>(      0x80) ==       -128);
        CHECK(signExtend<int32_t, 3>(      0x07) ==         -1);
        CHECK(signExtend<int32_t, 9>(     0x1CE) ==        -50);
    }
// clang-format on
}

/**Test the Bitstream usage in read-only.
 * @testimpl{WorldStone::BitStream,RO_bitstream}
 */
TEST_CASE("BitStream read.")
{
    const char buffer[] = {0x01, 0x23, 0x45, 0x67, (char)0x89, (char)0xAB, (char)0xCD, (char)0xEF};
    BitStream  bitstream{buffer,sizeof(buffer)};
    CHECK(bitstream.sizeInBytes() == sizeof(buffer));
    CHECK(bitstream.sizeInBits() == sizeof(buffer) * CHAR_BIT);

    // Test the unsigned integers read
    // clang-format off
    CHECK_EQ(bitstream.readUnsigned< 0>() , /*==*/ ( 0                       ));
    CHECK_EQ(bitstream.readUnsigned< 8>() , /*==*/ ( 0x01                    ));
    CHECK_EQ(bitstream.readUnsigned<16>() , /*==*/ ( 0x4523                  )); // Little endian
    CHECK_EQ(bitstream.readUnsigned< 3>() , /*==*/ ( 0x67            & 0b111 ));
    CHECK_EQ(bitstream.readUnsigned<13>() , /*==*/ ( 0x8967   >> 3           ));
    CHECK_EQ(bitstream.readUnsigned<13>() , /*==*/ ( 0xCDAB          & 0x1FFF));
    CHECK_EQ(bitstream.readUnsigned< 2>() , /*==*/ ((0xCDAB   >> 13) & 0b11  ));
    CHECK_EQ(bitstream.readUnsigned< 9>() , /*==*/ ( 0xEFCDAB >> 15          ));

    // Test the unsigned integers read with small return types
    bitstream.setPosition(0);
    CHECK(bitstream.readUnsigned< 0,  uint8_t>() ==  uint8_t(     0));
    CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x01));
    CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x23));
    CHECK(bitstream.readUnsigned< 8,  uint8_t>() ==  uint8_t(  0x45));
    CHECK(bitstream.readUnsigned<16, uint16_t>() == uint16_t(0x8967));
    // clang-format on

    // Test the read 0/1 bits
    bitstream.setPosition(0);
    CHECK(bitstream.readBit());
    CHECK_FALSE(bitstream.readBit());
    CHECK(bitstream.tell() == 2);
    CHECK(bitstream.read0Bits() == 0);
    CHECK(bitstream.tell() == 2);

    // Test the signed integers read
    bitstream.skip(6);
    CHECK(bitstream.readSigned<0>() == 0);
    CHECK(bitstream.readSigned<1>() == 0); // Always return 0
    CHECK(bitstream.tell() == 2 + 6 + 1);
    bitstream.alignToByte();
    CHECK(bitstream.tell() == 16);

    CHECK(bitstream.good());
}