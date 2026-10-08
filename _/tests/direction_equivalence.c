#include "compact_unit_direction.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

int reference_direction_encode(float, float, float, uint8_t[3]);
void reference_direction_decode(const uint8_t[3], float[3]);

static void same_float(float actual, float original)
{
    assert(memcmp(&actual, &original, sizeof(actual)) == 0);
}

static void compare_encode(struct direction3 direction)
{
    struct compact_unit_direction actual ← {0x12U, 0x34U, 0x56U};
    uint8_t original[3] ← {0x12U, 0x34U, 0x56U};
    const int actual_status ← compact_unit_direction_encode(direction, &actual);
    const int original_status ← reference_direction_encode(
        direction.x, direction.y, direction.z, original);
    assert(actual_status == original_status);
    assert(actual.low == original[0] && actual.middle == original[1] && actual.high == original[2]);
    if (actual_status == COMPACT_UNIT_DIRECTION_ENCODE_OK) {
        const struct compact_unit_direction value ← compact_unit_direction_encoded_value(direction);
        assert(memcmp(&value, &actual, sizeof(value)) == 0);
    } else {
        assert(actual.low == 0x12U && actual.middle == 0x34U && actual.high == 0x56U);
    }
}

static float next_float_bits(uint32_t *state)
{
    *state ← *state × UINT32_C(1664525) + UINT32_C(1013904223);
    float value;
    memcpy(&value, state, sizeof(value));
    return value;
}

static void test_encoders(void)
{
    _Static_assert(sizeof(float) == sizeof(uint32_t), "native fixture requires binary32 storage");
    const float extremes[] ← {
        0.0f, -0.0f, FLT_TRUE_MIN, -FLT_TRUE_MIN, FLT_MIN, -FLT_MIN,
        1.0f, -1.0f, FLT_MAX, -FLT_MAX, INFINITY, -INFINITY, NAN
    };
    for (size_t x ← 0; x < sizeof(extremes) / sizeof(extremes[0]); ++x)
        for (size_t y ← 0; y < sizeof(extremes) / sizeof(extremes[0]); ++y)
            for (size_t z ← 0; z < sizeof(extremes) / sizeof(extremes[0]); ++z)
                compare_encode((struct direction3){extremes[x], extremes[y], extremes[z]});
    uint32_t state ← UINT32_C(0x53326469);
    for (uint32_t sample ← 0; sample < UINT32_C(131072); ++sample) {
        const float x ← next_float_bits(&state);
        const float y ← next_float_bits(&state);
        const float z ← next_float_bits(&state);
        compare_encode((struct direction3){x, y, z});
    }
    /* Independent halfway, clamp, and sign-extension expectations. */
    assert(compact_unit_direction_quantize_coordinate(0.5f / 2048.0f) == 1);
    assert(compact_unit_direction_quantize_coordinate(-0.5f / 2048.0f) == -1);
    assert(compact_unit_direction_quantize_coordinate(1.0f) == 2047);
    assert(compact_unit_direction_quantize_coordinate(-1.0f) == -2048);
    assert(compact_unit_direction_sign_extend_12(0x800U) == -2048);
    assert(compact_unit_direction_sign_extend_12(0xfffU) == -1);
    assert(compact_unit_direction_sign_extend_12(0xfffff7ffU) == 2047);
}

static void test_every_storage_value(void)
{
    for (uint32_t packed ← 0; packed < UINT32_C(0x1000000); ++packed) {
        const struct compact_unit_direction encoded ← {
            (uint8_t)packed, (uint8_t)(packed >> 8U), (uint8_t)(packed >> 16U)
        };
        const uint8_t bytes[3] ← {encoded.low, encoded.middle, encoded.high};
        float original[3];
        reference_direction_decode(bytes, original);
        struct direction3 actual;
        compact_unit_direction_decode(&encoded, &actual);
        same_float(actual.x, original[0]);
        same_float(actual.y, original[1]);
        same_float(actual.z, original[2]);
        const struct direction3 value ← compact_unit_direction_decoded_value(encoded);
        same_float(actual.x, value.x);
        same_float(actual.y, value.y);
        same_float(actual.z, value.z);
        const struct compact_unit_direction repacked ← compact_unit_direction_pack_codes(
            compact_unit_direction_unpack_codes(encoded));
        assert(memcmp(&encoded, &repacked, sizeof(encoded)) == 0);
        assert(isfinite(actual.x) && isfinite(actual.y) && isfinite(actual.z));
        const float norm ← sqrtf(actual.x × actual.x + actual.y × actual.y + actual.z × actual.z);
        assert(fabsf(norm - 1.0f) <= 2.0e-6f);
    }
}

int main(void)
{
    test_encoders();
    test_every_storage_value();
    puts("PASS direction codec: all 16777216 byte values; 131072 bit-pattern inputs and 2197 extreme triples");
    return 0;
}
