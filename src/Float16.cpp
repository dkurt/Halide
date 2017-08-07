#include "Float16.h"
#include "Error.h"

namespace Halide
{

uint16_t from_f32(float x)
{
    union
    {
        unsigned u;
        float f;
    } in;
    in.f = x;

    unsigned abs_f = in.u & 0x7fffffff;
    unsigned t = (abs_f >> 13) - 0x1c000;
    unsigned sign = (in.u & 0x80000000) >> 16;
    unsigned e = abs_f & 0x7f800000;

    t = e < 0x38800000 ? 0 : t; // Flush-to-zero
    t = e < 0x47000000 ? t : abs_f > 0x7f800000 ? t - 0x1c000 : 0x7c00; // Clamp-to-max
    t = (e == 0 ? 0 : t) | sign;

    return (uint16_t)t;
}

float16_t::float16_t(float value, RoundingMode) : data(from_f32(value)) {}
float16_t::float16_t(double value, RoundingMode) : data(from_f32((float)value)) {}

float16_t::operator float() const
{
    union
    {
        int u;
        float f;
    } out;

    uint16_t data = this->data;
    unsigned t = ((data & 0x7fff) << 13) + 0x38000000; // extract and adjust mantissa and exponent
    unsigned sign = (data & 0x8000) << 16; // extract and shift the sign bit
    unsigned e = data & 0x7c00;
    out.u = (e >= 0x7c00 ? t + 0x38000000 : e == 0 ? 0 : t) | sign; // convert denormals to 0's
    return out.f;
}

float16_t::operator double() const
{
    return (float)*this;
}

}
