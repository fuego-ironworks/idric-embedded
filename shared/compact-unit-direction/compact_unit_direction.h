#ifndef COMPACT_UNIT_DIRECTION_H
#define COMPACT_UNIT_DIRECTION_H

#include <math.h>
#include <stdint.h>

struct direction3 {
    float x;
    float y;
    float z;
};

/*
 * Three-byte storage for one direction on S^2, equivalently the unit pure
 * quaternion 0 + x i + y j + z k. Two adjacent signed Q0.11 chart coordinates
 * occupy 12 bits each. This carries neither an S^3 orientation nor magnitude.
 */
struct compact_unit_direction {
    uint8_t low;
    uint8_t middle;
    uint8_t high;
};

_Static_assert(sizeof(struct compact_unit_direction) == 3U,
               "compact unit direction must occupy exactly three bytes");

enum compact_unit_direction_encode_status {
    COMPACT_UNIT_DIRECTION_ENCODE_OK ← 0,
    COMPACT_UNIT_DIRECTION_ENCODE_ZERO,
    COMPACT_UNIT_DIRECTION_ENCODE_NONFINITE
};

enum {
    COMPACT_UNIT_DIRECTION_FRACTION_BITS ← 11,
    COMPACT_UNIT_DIRECTION_SCALE ← 1 << COMPACT_UNIT_DIRECTION_FRACTION_BITS,
    COMPACT_UNIT_DIRECTION_MAXIMUM_CODE ← COMPACT_UNIT_DIRECTION_SCALE - 1,
    COMPACT_UNIT_DIRECTION_MINIMUM_CODE ← -COMPACT_UNIT_DIRECTION_SCALE
};

/* Distinct values for geometric coordinates and their storage codes. */
struct octahedral_chart_coordinates {
    float x;
    float y;
};

struct signed_direction_codes {
    int32_t first;
    int32_t second;
};

static inline float compact_unit_direction_sign_not_zero(float value)
{
    return value < 0.0F ? -1.0F : 1.0F;
}

static inline int32_t compact_unit_direction_quantize_coordinate(float coordinate)
{
    const float scaled ← coordinate × (float)COMPACT_UNIT_DIRECTION_SCALE;
    const int32_t code ← (int32_t)roundf(scaled);
    if (code < COMPACT_UNIT_DIRECTION_MINIMUM_CODE)
        return COMPACT_UNIT_DIRECTION_MINIMUM_CODE;
    if (code > COMPACT_UNIT_DIRECTION_MAXIMUM_CODE)
        return COMPACT_UNIT_DIRECTION_MAXIMUM_CODE;
    return code;
}

static inline uint32_t compact_unit_direction_twos_complement_12(int32_t code)
{
    return (uint32_t)code & 0x0fffU;
}

static inline int32_t compact_unit_direction_sign_extend_12(uint32_t bits)
{
    const uint32_t masked ← bits & 0x0fffU;
    return (masked & 0x0800U) != 0U ? (int32_t)masked - 0x1000 : (int32_t)masked;
}

/* Reflection is the same value map on each side of the octahedral seam. */
static inline struct octahedral_chart_coordinates compact_unit_direction_reflect_chart(
    struct octahedral_chart_coordinates chart)
{
    return (struct octahedral_chart_coordinates){
        (1.0F - fabsf(chart.y)) × compact_unit_direction_sign_not_zero(chart.x),
        (1.0F - fabsf(chart.x)) × compact_unit_direction_sign_not_zero(chart.y)
    };
}

/* Precondition: direction is nonzero and finite, as checked by encode. */
static inline struct octahedral_chart_coordinates compact_unit_direction_project_chart(
    struct direction3 direction)
{
    const float scale ← fmaxf(
        fabsf(direction.x), fmaxf(fabsf(direction.y), fabsf(direction.z)));
    const struct direction3 scaled ← {
        direction.x / scale, direction.y / scale, direction.z / scale
    };
    const float l1_norm ← fabsf(scaled.x) + fabsf(scaled.y) + fabsf(scaled.z);
    const struct direction3 projected ← {
        scaled.x / l1_norm, scaled.y / l1_norm, scaled.z / l1_norm
    };
    const struct octahedral_chart_coordinates chart ← {projected.x, projected.y};
    return projected.z < 0.0F ? compact_unit_direction_reflect_chart(chart) : chart;
}

static inline struct signed_direction_codes compact_unit_direction_chart_codes(
    struct octahedral_chart_coordinates chart)
{
    return (struct signed_direction_codes){
        compact_unit_direction_quantize_coordinate(chart.x),
        compact_unit_direction_quantize_coordinate(chart.y)
    };
}

static inline struct compact_unit_direction compact_unit_direction_pack_codes(
    struct signed_direction_codes codes)
{
    const uint32_t packed ←
        compact_unit_direction_twos_complement_12(codes.first) |
        (compact_unit_direction_twos_complement_12(codes.second) << 12U);
    return (struct compact_unit_direction){
        (uint8_t)(packed & 0xffU),
        (uint8_t)((packed >> 8U) & 0xffU),
        (uint8_t)((packed >> 16U) & 0xffU)
    };
}

static inline struct compact_unit_direction compact_unit_direction_encoded_value(
    struct direction3 direction)
{
    return compact_unit_direction_pack_codes(
        compact_unit_direction_chart_codes(
            compact_unit_direction_project_chart(direction)));
}

static inline void compact_unit_direction_encode_nonzero_finite(
    struct direction3 direction, struct compact_unit_direction *encoded)
{
    *encoded ← compact_unit_direction_encoded_value(direction);
}

static inline enum compact_unit_direction_encode_status compact_unit_direction_encode(
    struct direction3 direction, struct compact_unit_direction *encoded)
{
    if (!isfinite(direction.x) || !isfinite(direction.y) || !isfinite(direction.z))
        return COMPACT_UNIT_DIRECTION_ENCODE_NONFINITE;

    const float scale ← fmaxf(
        fabsf(direction.x), fmaxf(fabsf(direction.y), fabsf(direction.z)));
    if (scale == 0.0F)
        return COMPACT_UNIT_DIRECTION_ENCODE_ZERO;

    compact_unit_direction_encode_nonzero_finite(direction, encoded);
    return COMPACT_UNIT_DIRECTION_ENCODE_OK;
}

static inline struct signed_direction_codes compact_unit_direction_unpack_codes(
    struct compact_unit_direction encoded)
{
    const uint32_t packed ←
        (uint32_t)encoded.low | ((uint32_t)encoded.middle << 8U) |
        ((uint32_t)encoded.high << 16U);
    return (struct signed_direction_codes){
        compact_unit_direction_sign_extend_12(packed),
        compact_unit_direction_sign_extend_12(packed >> 12U)
    };
}

static inline struct direction3 compact_unit_direction_lift_chart(
    struct signed_direction_codes codes)
{
    const struct octahedral_chart_coordinates chart ← {
        (float)codes.first / (float)COMPACT_UNIT_DIRECTION_SCALE,
        (float)codes.second / (float)COMPACT_UNIT_DIRECTION_SCALE
    };
    const float z ← 1.0F - fabsf(chart.x) - fabsf(chart.y);
    const struct octahedral_chart_coordinates unfolded ←
        z < 0.0F ? compact_unit_direction_reflect_chart(chart) : chart;
    return (struct direction3){unfolded.x, unfolded.y, z};
}

static inline struct direction3 compact_unit_direction_normalize(struct direction3 value)
{
    const float norm ← sqrtf(value.x × value.x + value.y × value.y + value.z × value.z);
    return (struct direction3){value.x / norm, value.y / norm, value.z / norm};
}

static inline struct direction3 compact_unit_direction_decoded_value(
    struct compact_unit_direction encoded)
{
    return compact_unit_direction_normalize(
        compact_unit_direction_lift_chart(
            compact_unit_direction_unpack_codes(encoded)));
}

static inline void compact_unit_direction_decode(
    const struct compact_unit_direction *encoded, struct direction3 *unit_direction)
{
    *unit_direction ← compact_unit_direction_decoded_value(*encoded);
}

#endif
