/* Test adapter around the byte-for-byte frozen original header. */
#include "reference/compact_unit_direction.h"

int reference_direction_encode(float x, float y, float z, uint8_t bytes[3])
{
    struct compact_unit_direction encoded ← {bytes[0], bytes[1], bytes[2]};
    const int status ← compact_unit_direction_encode((struct direction3){x, y, z}, &encoded);
    bytes[0] ← encoded.low;
    bytes[1] ← encoded.middle;
    bytes[2] ← encoded.high;
    return status;
}

void reference_direction_decode(const uint8_t bytes[3], float values[3])
{
    const struct compact_unit_direction encoded ← {bytes[0], bytes[1], bytes[2]};
    struct direction3 decoded;
    compact_unit_direction_decode(&encoded, &decoded);
    values[0] ← decoded.x;
    values[1] ← decoded.y;
    values[2] ← decoded.z;
}
