/*
Copyright 2026 William Bundy, all rights reserved.

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

This is all the math/mathy utility code I've needed over the last few years. 
It's only designed to work with modern clang. 

Contains:
* ext_vector_type vectors for most types
* impmelementations of several math functions (sqrt) using x86/ARM intrinsics
* conversion helpers
* scalar helpers (i32abs, f32abs, i32clamp etc)
* 2D vector functions (f2dot, f2mag, etc)
* a bunch of intersection/collision primitives (aabb/box/circle/line stuff)
* some other geometry code I've needed
* 3D matrix setup
* Random number gen using xoroshiro** and splitmix64
* Some hashing helpers for various types using murmur3

Everything in here is updated as needed, so there might be conversions or
other things you consider essential that are missing just because I haven't
run into them.

Everything is marked always_inline for dramatically better performance in 
debug builds.

Status:
Pretty well tested, but I'm always tinkering with it, so there might be issues
*/

#pragma once
#include <stdint.h>
#define GAMEMATH_INLINE __attribute__((always_inline))

#define m_sqrt2  1.4142135623730951
#define m_invsqrt2  0.7071067811865476
#define m_tau  6.283185307179586
#define m_todegrees  57.29577951308232
#define m_toradians  0.017453292519943295
#define m_e 2.718281828459045



#if defined(__aarch64__)
#define WB_GAMEMATH__ARM 1
#elif  defined(__x86_64)
#define WB_GAMEMATH__X86 1
#endif

typedef float float2 __attribute__((ext_vector_type((2))));
typedef float float4 __attribute__((ext_vector_type((4))));

#define _INT2_DEFINED_ 1
typedef int32_t int2 __attribute__((ext_vector_type((2))));
typedef int32_t int4 __attribute__((ext_vector_type((4))));
typedef uint32_t uint2 __attribute__((ext_vector_type((2))));
typedef uint32_t uint4 __attribute__((ext_vector_type((4))));

typedef int16_t short2 __attribute__((ext_vector_type(2)));
typedef int16_t short4 __attribute__((ext_vector_type(4)));
typedef uint16_t ushort2 __attribute__((ext_vector_type(2)));
typedef uint16_t ushort4 __attribute__((ext_vector_type(4)));

GAMEMATH_INLINE
float2 int2_to_float(int2 a)
{
	return (float2){
		(float)a[0],
		(float)a[1],
	};
}


GAMEMATH_INLINE
float4 int4_to_float(int4 a)
{
	return (float4){
		(float)a[0],
		(float)a[1],
		(float)a[2],
		(float)a[3],
	};
}

GAMEMATH_INLINE
float2 short2_to_float(short2 a)
{
	return (float2){
		(float)a[0],
		(float)a[1],
	};
}

GAMEMATH_INLINE
float4 short4_to_float(short4 a)
{
	return (float4){
		(float)a[0],
		(float)a[1],
		(float)a[2],
		(float)a[3],
	};
}

GAMEMATH_INLINE
float2 uint2_to_float(uint2 a)
{
	return (float2){
		(float)a[0],
		(float)a[1],
	};
}

GAMEMATH_INLINE
float4 uint4_to_float(uint4 a)
{
	return (float4){
		(float)a[0],
		(float)a[1],
		(float)a[2],
		(float)a[3],
	};
}

GAMEMATH_INLINE
float2 ushort2_to_float(ushort2 a)
{
	return (float2){
		(float)a[0],
		(float)a[1],
	};
}

GAMEMATH_INLINE
float4 ushort4_to_float(ushort4 a)
{
	return (float4){
		(float)a[0],
		(float)a[1],
		(float)a[2],
		(float)a[3],
	};
}

GAMEMATH_INLINE
int2 float2_to_int(float2 a)
{
	return (int2){
		(int)a[0],
		(int)a[1],
	};
}

GAMEMATH_INLINE
int4 float4_to_int(float4 a)
{
	return (int4){
		(int)a[0],
		(int)a[1],
		(int)a[2],
		(int)a[3],
	};
}

GAMEMATH_INLINE
short2 float2_to_short(float2 a)
{
	return (short2){
		(short)a[0],
		(short)a[1],
	};
}

GAMEMATH_INLINE
short4 float4_to_short(float4 a)
{
	return (short4){
		(short)a[0],
		(short)a[1],
		(short)a[2],
		(short)a[3],
	};
}

GAMEMATH_INLINE
uint2 int2_to_uint(int2 a)
{
	return (uint2){
		(uint32_t)a[0],
		(uint32_t)a[1],
	};
}

GAMEMATH_INLINE
uint4 int4_to_uint(int4 a)
{
	return (uint4){
		(uint32_t)a[0],
		(uint32_t)a[1],
		(uint32_t)a[2],
		(uint32_t)a[3],
	};
}

GAMEMATH_INLINE
ushort2 short2_to_ushort(short2 a)
{
	return (ushort2){
		(uint16_t)a[0],
		(uint16_t)a[1],
	};
}

GAMEMATH_INLINE
ushort4 short4_to_ushort(short4 a)
{
	return (ushort4){
		(uint16_t)a[0],
		(uint16_t)a[1],
		(uint16_t)a[2],
		(uint16_t)a[3],
	};
}

GAMEMATH_INLINE
uint2 uint2_to_int(uint2 a)
{
	return (uint2){
		(uint32_t)a[0],
		(uint32_t)a[1],
	};
}

GAMEMATH_INLINE
uint4 uint4_to_int(uint4 a)
{
	return (uint4){
		(uint32_t)a[0],
		(uint32_t)a[1],
		(uint32_t)a[2],
		(uint32_t)a[3],
	};
}

GAMEMATH_INLINE
short2 ushort2_to_short(ushort2 a)
{
	return (short2){
		(int16_t)a[0],
		(int16_t)a[1],
	};
}

GAMEMATH_INLINE
short4 ushort4_to_short(ushort4 a)
{
	return (short4){
		(int16_t)a[0],
		(int16_t)a[1],
		(int16_t)a[2],
		(int16_t)a[3],
	};
}

GAMEMATH_INLINE
ushort2 int2_to_ushort(int2 a)
{
	return (ushort2){
		(uint16_t)a[0],
		(uint16_t)a[1],
	};
}

GAMEMATH_INLINE
ushort4 int4_to_ushort(int4 a)
{
	return (ushort4){
		(uint16_t)a[0],
		(uint16_t)a[1],
		(uint16_t)a[2],
		(uint16_t)a[3],
	};
}

GAMEMATH_INLINE
int2 short2_to_int(short2 a)
{
	return (int2){
		(int32_t)a[0],
		(int32_t)a[1],
	};
}

GAMEMATH_INLINE
int4 short4_to_int(short4 a)
{
	return (int4){
		(int32_t)a[0],
		(int32_t)a[1],
		(int32_t)a[2],
		(int32_t)a[3],
	};
}


#if defined(WB_GAMEMATH__ARM)
#include <arm_neon.h>
// TODO(will) benchmark these, not sure how arm likes its vector registers
// could be that using the 2-wide instructions is dramatically
// worse than using the 4-wide? not sure

GAMEMATH_INLINE
float f32sqrt(float f)
{
	float2 ff = f;
	return vsqrt_f32(ff)[0];
}

// reminder: accuracy here is bad
GAMEMATH_INLINE
float f32rsqrt(float f)
{
	float2 ff = f;
	return vrsqrte_f32(ff)[0];
}

GAMEMATH_INLINE
float2 f2sqrt(float2 f)
{
	return vsqrt_f32(f);
}

// reminder: accuracy here is bad
GAMEMATH_INLINE
float2 f2rsqrt(float2 f)
{
	return vrsqrte_f32(f);
}

GAMEMATH_INLINE
float4 f4sqrt(float4 f)
{
	return vsqrtq_f32(f);
}

// reminder: accuracy here is bad
GAMEMATH_INLINE
float4 f4rsqrt(float4 f)
{
	return vrsqrteq_f32(f);
}

//FIXME putting these here because arm_neon 
// doesn't expose frintm and frintp for single
// floats (should be vrndms_f32 and vrndps_f32)
float floorf(float);
float ceilf(float);

GAMEMATH_INLINE
float f32floor(float f)
{
	//return vrndms_f32(f);
	return floorf(f);
}

GAMEMATH_INLINE
float f32round(float f)
{
	return vrndns_f32(f);
}

GAMEMATH_INLINE
float f32ceil(float f)
{
	//return vrndps_f32(f);
	return ceilf(f);
}

GAMEMATH_INLINE
float2 f2floor(float2 f)
{
	return vrndm_f32(f);
}

GAMEMATH_INLINE
float2 f2round(float2 f)
{
	return vrndn_f32(f);
}

GAMEMATH_INLINE
float2 f2ceil(float2 f)
{
	return vrndp_f32(f);
}


GAMEMATH_INLINE
float4 f4floor(float4 f)
{
	return vrndmq_f32(f);
}

GAMEMATH_INLINE
float4 f4round(float4 f)
{
	return vrndnq_f32(f);
}

GAMEMATH_INLINE
float4 f4ceil(float4 f)
{
	return vrndpq_f32(f);
}

#endif


#if defined(WB_GAMEMATH__X86)
#include <emmintrin.h>
#include <smmintrin.h>

GAMEMATH_INLINE
float f32sqrt(float f)
{
	__m128 ff = _mm_set_ss(f);
	return _mm_sqrt_ss(ff)[0];
}

// reminder: accuracy here is bad
GAMEMATH_INLINE
float f32rsqrt(float f)
{
	__m128 ff = _mm_set_ss(f);
	return _mm_rsqrt_ss(ff)[0];
}

GAMEMATH_INLINE
float4 f4sqrt(float4 f)
{
	return _mm_sqrt_ps(f);
}


// reminder: accuracy here is bad
GAMEMATH_INLINE
float4 f4rsqrt(float4 f)
{
	return _mm_rsqrt_ps(f);
}

GAMEMATH_INLINE
float2 f2sqrt(float2 f)
{
	return ((float4)_mm_sqrt_ps(f.xyxy)).xy;
}

// reminder: accuracy here is bad
GAMEMATH_INLINE
float2 f2rsqrt(float2 f)
{
	return ((float4)_mm_rsqrt_ps(f.xyxy)).xy;
}


GAMEMATH_INLINE
float f32floor(float f)
{
	__m128 ff = _mm_set_ss(f);
	return _mm_floor_ss(ff, ff)[0];
}

GAMEMATH_INLINE
float f32round(float f)
{
	__m128 ff = _mm_set_ss(f);
	return _mm_round_ss(ff, ff, _MM_ROUND_NEAREST)[0];
}

GAMEMATH_INLINE
float f32ceil(float f)
{
	__m128 ff = _mm_set_ss(f);
	return _mm_ceil_ss(ff, ff)[0];
}

GAMEMATH_INLINE
float4 f4floor(float4 f)
{
	return _mm_floor_ps(f);
}

GAMEMATH_INLINE
float4 f4round(float4 f)
{
	return _mm_round_ps(f, _MM_ROUND_NEAREST);
}

GAMEMATH_INLINE
float4 f4ceil(float4 f)
{
	return _mm_ceil_ps(f);
}

// probably good enough?
GAMEMATH_INLINE
float2 f2floor(float2 f)
{
	return ((float4)_mm_floor_ps(f.xyxy)).xy;
}

GAMEMATH_INLINE
float2 f2round(float2 f)
{
	return ((float4)_mm_round_ps(f.xyxy, _MM_ROUND_NEAREST)).xy;
}

GAMEMATH_INLINE
float2 f2ceil(float2 f)
{
	return ((float4)_mm_ceil_ps(f.xyxy)).xy;
}
#endif

// I've checked pretty much all of these in godbolt
// and as far as I can tell they give the best codegen

// f32
GAMEMATH_INLINE
int f2cmp(float2 a, float2 b)
{
	return (a.x == b.x) ? ((a.y < b.y) ? 0 : 1) : ((a.x < b.x) ? 0 : 1);
}

GAMEMATH_INLINE
float f32min(float a, float b)
{
	return a < b ? a : b;
}

GAMEMATH_INLINE
float f32max(float a, float b)
{
	return a > b ? a : b;
}

GAMEMATH_INLINE
float f32clamp(float x, float lo, float hi)
{
	return f32min(f32max(x, lo), hi);
}

GAMEMATH_INLINE
float f32lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

// this compiles to a single vandps
// x < 0 ? -x : x complies to vxorps, vmaxss
// so I'm keeping this one
GAMEMATH_INLINE
float f32abs(float x)
{
	union {
		float f; uint32_t i;
	} u = {.f = x};
	u.i &= ~(1u<<31);
	return u.f;
}

GAMEMATH_INLINE 
float f32sign(float a)
{
	return a < 0 ? -1 : 1;
}

// float2
GAMEMATH_INLINE
float2 f2min(float2 a, float2 b)
{
	return (float2){
		f32min(a[0], b[0]),
		f32min(a[1], b[1]),
	};
}


GAMEMATH_INLINE
float2 f2max(float2 a, float2 b)
{
	return (float2){
		f32max(a[0], b[0]),
		f32max(a[1], b[1]),
	};
}

GAMEMATH_INLINE
float2 f2clamp(float2 x, float2 lo, float2 hi)
{
	return f2min(f2max(x, lo), hi);
}

GAMEMATH_INLINE
float2 f2lerp(float2 a, float2 b, float t)
{
	return a + t * (b - a);
}

GAMEMATH_INLINE
float2 f2lerp2(float2 a, float2 b, float2 t)
{
	return a + t * (b - a);
}

GAMEMATH_INLINE
float2 f2abs(float2 a)
{
	return (float2){
		f32abs(a[0]),
		f32abs(a[1]),
	};
}

GAMEMATH_INLINE
float2 f2sign(float2 a)
{
	return (float2){
		f32sign(a[0]),
		f32sign(a[1]),
	};
}

// f4
GAMEMATH_INLINE
float4 f4min(float4 a, float4 b)
{
	return (float4){
		f32min(a[0], b[0]),
		f32min(a[1], b[1]),
		f32min(a[2], b[2]),
		f32min(a[2], b[3]),
	};
}


GAMEMATH_INLINE
float4 f4max(float4 a, float4 b)
{
	return (float4){
		f32max(a[0], b[0]),
		f32max(a[1], b[1]),
		f32max(a[2], b[2]),
		f32max(a[3], b[3]),
	};
}

GAMEMATH_INLINE
float4 f4clamp(float4 x, float4 lo, float4 hi)
{
	return f4min(f4max(x, lo), hi);
}

GAMEMATH_INLINE
float4 f4lerp(float4 a, float4 b, float t)
{
	return a + t * (b - a);
}

GAMEMATH_INLINE
float4 f4lerp2(float4 a, float4 b, float2 t)
{
	float4 v;
	v.xy = f2lerp(a.xy, b.xy, t.x);
	v.zw = f2lerp(a.zw, b.zw, t.y);
	return v;
}

GAMEMATH_INLINE
float4 f4lerp4(float4 a, float4 b, float4 t)
{
	return a + t * (b - a);
}

GAMEMATH_INLINE
float4 f4abs(float4 a)
{
	return (float4){
		f32abs(a[0]),
		f32abs(a[1]),
		f32abs(a[2]),
		f32abs(a[3]),
	};
}

GAMEMATH_INLINE
float4 f4sign(float4 a)
{
	return (float4){
		f32sign(a[0]),
		f32sign(a[1]),
		f32sign(a[2]),
		f32sign(a[3]),
	};
}

GAMEMATH_INLINE
float4 f4bounds(float2 a, float2 b)
{
	float4 r;
	r.xy = f2min(a, b);
	r.zw = f2max(a, b);
	return r;
}

GAMEMATH_INLINE
float4 f4boundsPoint(float4 region, float2 p)
{
	float4 r;
	r.xy = f2min(region.xy, p);
	r.zw = f2max(region.zw, p);
	return r;
}

// i32
GAMEMATH_INLINE
int i32min(int a, int b)
{
	return a < b ? a : b;
}

GAMEMATH_INLINE
int i32max(int a, int b)
{
	return a > b ? a : b;
}

GAMEMATH_INLINE
int i32clamp(int x, int lo, int hi)
{
	return i32min(i32max(x, lo), hi);
}

GAMEMATH_INLINE
int i32lerp(int a, int b, float t)
{
	return a + (int)(t * (float)(b - a));
}

GAMEMATH_INLINE
int i32abs(int x)
{
	return x < 0 ? -x : x;
}

GAMEMATH_INLINE 
int i32sign(int x)
{
	return x < 0 ? -1 : 1;
}

// int2
GAMEMATH_INLINE
int2 i2min(int2 a, int2 b)
{
	return (int2){
		i32min(a[0], b[0]),
		i32min(a[1], b[1]),
	};
}


GAMEMATH_INLINE
int2 i2max(int2 a, int2 b)
{
	return (int2){
		i32max(a[0], b[0]),
		i32max(a[1], b[1]),
	};
}

GAMEMATH_INLINE
int2 i2clamp(int2 x, int2 lo, int2 hi)
{
	return i2min(i2max(x, lo), hi);
}

GAMEMATH_INLINE
int2 i2lerp(int2 a, int2 b, float t)
{
	return (int2){
		i32lerp(a[0], b[0], t),
		i32lerp(a[1], b[1], t),
	};
}

GAMEMATH_INLINE
int2 i2abs(int2 a)
{
	return (int2){
		i32abs(a[0]),
		i32abs(a[1]),
	};
}

GAMEMATH_INLINE
int2 i2sign(int2 a)
{
	return (int2){
		i32sign(a[0]),
		i32sign(a[1]),
	};
}

// int4

GAMEMATH_INLINE
int4 i4from2(int2 xy, int2 zw)
{
	int4 v;
	v.xy = xy;
	v.zw = zw;
	return v;
}

GAMEMATH_INLINE
int4 i4min(int4 a, int4 b)
{
	return (int4){
		i32min(a[0], b[0]),
		i32min(a[1], b[1]),
		i32min(a[2], b[2]),
		i32min(a[2], b[3]),
	};
}


GAMEMATH_INLINE
int4 i4max(int4 a, int4 b)
{
	return (int4){
		i32max(a[0], b[0]),
		i32max(a[1], b[1]),
		i32max(a[2], b[2]),
		i32max(a[3], b[3]),
	};
}

GAMEMATH_INLINE
int4 i4clamp(int4 x, int4 lo, int4 hi)
{
	return i4min(i4max(x, lo), hi);
}

GAMEMATH_INLINE
int4 i4lerp(int4 a, int4 b, int t)
{
	return (int4){
		i32lerp(a[0], b[0], t),
		i32lerp(a[1], b[1], t),
		i32lerp(a[2], b[2], t),
		i32lerp(a[3], b[3], t),
	};
}

GAMEMATH_INLINE
int4 i4abs(int4 a)
{
	return (int4){
		i32abs(a[0]),
		i32abs(a[1]),
		i32abs(a[2]),
		i32abs(a[3]),
	};
}

GAMEMATH_INLINE
int4 i4sign(int4 a)
{
	return (int4){
		i32sign(a[0]),
		i32sign(a[1]),
		i32sign(a[2]),
		i32sign(a[3]),
	};
}

GAMEMATH_INLINE
int4 i4bounds(int2 a, int2 b)
{
	int4 r;
	r.xy = i2min(a, b);
	r.zw = i2max(a, b);
	return r;
}

GAMEMATH_INLINE
int4 i4boundsRect(int2 a, int2 b)
{
	int4 r;
	r.xy = i2min(a, b);
	r.zw = i2max(a, b);
	r.zw -= r.xy;
	return r;
}



GAMEMATH_INLINE
int4 i4boundsPoint(int4 region, int2 p)
{
	int4 r;
	r.xy = i2min(region.xy, p);
	r.zw = i2max(region.zw, p);
	return r;
}

// float2 extras

GAMEMATH_INLINE
float f2dot(float2 a, float2 b)
{
	float2 v = a * b;
	return v.x + v.y;
}

GAMEMATH_INLINE
float f2cross(float2 a, float2 b)
{
	float2 c = a.xy * b.yx;
	return c.x - c.y;
}

GAMEMATH_INLINE 
float f2crossOrigin(float2 a, float2 b, float2 origin)
{
	a -= origin;
	b -= origin;
	return f2cross(a, b);
}

GAMEMATH_INLINE
float f2mag2(float2 a)
{
	return f2dot(a, a);
}

GAMEMATH_INLINE
int f2near(float2 a, float2 b, float dist)
{
	return f2mag2(b - a) < dist * dist;
}

GAMEMATH_INLINE 
float f2invMag(float2 a)
{
	float2 b = a * a;
	return f32rsqrt(b.x + b.y);
}

GAMEMATH_INLINE
float f2mag(float2 a)
{
	float2 b = a * a;
	return f32sqrt(b.x + b.y);
}

GAMEMATH_INLINE
float2 f2normalize(float2 a)
{
	if(a.x*a.x + a.y*a.y == 0) return (float2){0, 0};
	return a * f2invMag(a);
}

GAMEMATH_INLINE
float2 f2normalizeAndMag(float2 a, float* magOut)
{
	if(a.x*a.x + a.y*a.y == 0) return (float2){0, 0};
	float mag = f2mag(a);
	if(magOut) *magOut = mag;
	// so... this won't actually return the same results as 
	// f2normalize, f32rsqrt is lower precision than f32sqrt
	// on intel/sse, at least
	return a / mag;
}

GAMEMATH_INLINE
float2 f2clampMag(float2 a, float len)
{
	return (f2mag2(a) > len * len) ? (f2normalize(a) * len) : a;
}

GAMEMATH_INLINE
float2 f2perp(float2 a)
{
	return (float2){-a.y, a.x};
}

GAMEMATH_INLINE
float2 f2rot(float2 a, float2 r)
{
	return (float2) {
		a.x * r.x + a.y * -r.y,
		a.x * r.y + a.y * r.x
	};
}

#define f2rotate f2rot

GAMEMATH_INLINE
float2 f2swap(float2 a)
{
	return (float2){a.y, a.x};
}


GAMEMATH_INLINE
float2 f2project(float2 a, float2 b)
{
	return b * (f2dot(a, b) / f2dot(b, b));
}
 
#include <box2d/math_functions.h>
typedef struct Transform
{
	union {
		struct {
			union {
				float2 pos;
				struct {float x, y;};
			};
			union {
				float2 rot;
				struct {float c, s;};
			};
		};
		b2Transform xf;
	};
} Transform;
static_assert(sizeof(Transform) == 16);
static_assert(sizeof(b2Transform) == 16);

GAMEMATH_INLINE
Transform transformBy(Transform child, Transform parent)
{
	child.rot = f2rotate(child.rot, parent.rot);
	child.pos = f2rotate(child.pos, parent.rot);
	child.pos += parent.pos;
	return child;
}

GAMEMATH_INLINE
Transform untransformBy(Transform child, Transform parent)
{
	parent.rot.y *= -1;
	child.pos -= parent.pos;
	child.pos = f2rotate(child.pos, parent.rot);
	child.rot = f2rotate(child.rot, parent.rot);
	return child;
}

// "float3" extras
// for alignment reasons, these use float4s

GAMEMATH_INLINE
float f3dot(float4 a, float4 b)
{
	float4 v = a * b;
	return v.x + v.y + v.z;
}

GAMEMATH_INLINE
float4 f3cross(float4 a, float4 b)
{
	float4 ca = a.yxxx * b.zzyx;
	float4 cb = a.zzyx * b.yxxx;
	return ca - cb;
}

GAMEMATH_INLINE 
float4 f3crossOrigin(float4 a, float4 b, float4 origin)
{
	a -= origin;
	b -= origin;
	return f3cross(a, b);
}

GAMEMATH_INLINE
float f3mag2(float4 a)
{
	return f3dot(a, a);
}

GAMEMATH_INLINE
int f3near(float4 a, float4 b, float dist)
{
	return f3mag2(b - a) < dist * dist;
}

GAMEMATH_INLINE 
float f3invMag(float4 a)
{
	float4 b = a * a;
	return f32rsqrt(b.x + b.y + b.z);
}

GAMEMATH_INLINE
float f3mag(float4 a)
{
	float4 b = a * a;
	return f32sqrt(b.x + b.y + b.z);
}

GAMEMATH_INLINE
float4 f3normalize(float4 a)
{
	if(a.x*a.x + a.y*a.y == 0) return (float4){0, 0};
	return a * f3invMag(a);
}

GAMEMATH_INLINE
float4 f3clampMag(float4 a, float len)
{
	return (f3mag2(a) > len * len) ? (f3normalize(a) * len) : a;
}

GAMEMATH_INLINE
float4 f3project(float4 a, float4 b)
{
	return b * (f3dot(a, b) / f3dot(b, b));
}

// float4 extras, where possible

GAMEMATH_INLINE
float f4dot(float4 a, float4 b)
{
	float4 v = a * b;
	return v.x + v.y + v.z + v.w;
}

GAMEMATH_INLINE
float f4mag2(float4 a)
{
	return f4dot(a, a);
}

GAMEMATH_INLINE
int f4near(float4 a, float4 b, float dist)
{
	return f4mag2(b - a) < dist * dist;
}

GAMEMATH_INLINE 
float f4invMag(float4 a)
{
	float4 b = a * a;
	return f32rsqrt(b.x + b.y + b.z + b.w);
}

GAMEMATH_INLINE
float f4mag(float4 a)
{
	float4 b = a * a;
	return f32sqrt(b.x + b.y + b.z + b.w);
}

GAMEMATH_INLINE
float4 f4normalize(float4 a)
{
	if(a.x*a.x + a.y*a.y == 0) return (float4){0, 0};
	return a * f4invMag(a);
}

GAMEMATH_INLINE
float4 f4clampMag(float4 a, float len)
{
	return (f4mag2(a) > len * len) ? (f4normalize(a) * len) : a;
}

GAMEMATH_INLINE
float4 f4project(float4 a, float4 b)
{
	return b * (f4dot(a, b) / f4dot(b, b));
}

// geometry

GAMEMATH_INLINE
float4 rect_from_float2(float2 pos, float2 size)
{
	return (float4){pos.x, pos.y, size.x, size.y};
}

GAMEMATH_INLINE
bool irect_intersect(int4 a, int4 b)
{
	return !(a.x + a.z <= b.x || a.y + a.w <= b.y || b.x + b.z <= a.x || b.y + b.w <= a.y);
}

GAMEMATH_INLINE 
bool irect_contains(int4 a, float2 pf)
{	
	int2 p = {(int)p.x, (int)p.y};
	return p.x >= a.x && p.y >= a.y && p.x < a.x + a.z && p.y < a.y + a.w;
}

GAMEMATH_INLINE 
bool irect_icontains(int4 a, int2 p)
{	
	return p.x >= a.x && p.y >= a.y && p.x < a.x + a.z && p.y < a.y + a.w;
}

GAMEMATH_INLINE
bool rect_contains(float4 a, float2 p)
{
	return p.x >= a.x && p.y >= a.y && p.x < a.x + a.z && p.y < a.y + a.w;
}

GAMEMATH_INLINE
bool rect_contains_inclusive(float4 a, float2 p)
{
	return p.x >= a.x && p.y >= a.y && p.x <= a.x + a.z && p.y <= a.y + a.w;
}

GAMEMATH_INLINE
bool rect_contains_rect(float4 a, float4 b)
{
	return rect_contains_inclusive(a, b.xy) && rect_contains_inclusive(a, b.xy + b.zw);
}

GAMEMATH_INLINE
int rect_contains_any(float4* a, int n, float2 p, float expand)
{
	for(int i = 0; i < n; ++i)
	{
		float4 f = a[i];
		f.xy -= expand;
		f.zw += expand * 2;
		if(rect_contains(f, p)) return 1;
	}
	return 0;
}

GAMEMATH_INLINE
float4 aabb_from_float2(float2 lo, float2 hi)
{
	return (float4){lo.x, lo.y, hi.x, hi.y};
}

GAMEMATH_INLINE 
bool aabb_intersect(float2 atl, float2 abr, float2 btl, float2 bbr)
{
	float4 u = {abr.x, abr.y, bbr.x, bbr.y};
	float4 v = {btl.x, btl.y, atl.x, atl.y};
	int4 cmp = u <= v;
	return !(cmp[0] || cmp[1] || cmp[2] || cmp[3]);
}


GAMEMATH_INLINE 
bool aabb_f4_intersect(float4 a, float4 b)
{
	float2 atl = a.xy;
	float2 abr = a.zw;
	float2 btl = b.xy;
	float2 bbr = b.zw;
	float4 u = {abr.x, abr.y, bbr.x, bbr.y};
	float4 v = {btl.x, btl.y, atl.x, atl.y};
	int4 cmp = u <= v;
	return !(cmp[0] || cmp[1] || cmp[2] || cmp[3]);
}

GAMEMATH_INLINE 
bool rect_intersect(float4 a, float4 b)
{
	float2 atl = a.xy;
	float2 btl = b.xy;
	float2 abr = a.xy + a.zw;
	float2 bbr = b.xy + b.zw;
	return aabb_intersect(atl, abr, btl, bbr);
}

GAMEMATH_INLINE 
bool box_intersect(float4 a, float4 b)
{
	float2 atl = a.xy - a.zw;
	float2 btl = b.xy - b.zw;
	float2 abr = a.xy + a.zw;
	float2 bbr = b.xy + b.zw;
	return aabb_intersect(atl, abr, btl, bbr);
}

GAMEMATH_INLINE
bool box_contains(float4 a, float2 p)
{
	float2 atl = a.xy - a.zw;
	float2 abr = a.xy + a.zw;
	return p.x >= atl.x && p.y >= atl.y && p.x < abr.x && p.y < abr.y;
}

GAMEMATH_INLINE 
float2 box_overlap(float4 a, float4 b)
{
	float2 d = b.xy - a.xy;
	float2 sd = f2sign(d);
	float2 s = (a.zw + b.zw) - f2abs(d);
	float2 as = f2abs(s);

	s.x = as.x > as.y ? 0 : s.x;
	s.y = as.y > as.x ? 0 : s.y;

	s *= sd;

	return s;
}

GAMEMATH_INLINE
float4 aabb_union(float4 a, float4 b)
{
	float4 bb;
	bb.xy = f2min(a.xy, b.xy);
	bb.zw = f2max(a.zw, b.zw);

	return bb;
}


GAMEMATH_INLINE
float4 aabb_union_normalize(float4 a, float4 b)
{
	float4 bb;
	bb.xy = f2min(f2min(f2min(a.xy, b.xy), a.zw), b.zw);
	bb.zw = f2max(f2max(f2max(a.zw, b.zw), a.xy), b.xy);

	return bb;
}

GAMEMATH_INLINE
bool aabb_contains(float4 bb, float2 p)
{
	return p.x > bb.x && p.y > bb.y && p.x < bb.z && p.y < bb.w;
}

GAMEMATH_INLINE
bool obox_contains(float4 r, float2 rot, float2 p)
{
	float2 offset = r.xy;
	float2 ip = f2rot(p - offset, f2perp(rot));
	ip += offset;

	return box_contains(r, ip);
}

GAMEMATH_INLINE 
float2 circle_overlap(float4 a, float4 b, float2* normal)
{
	float2 dist = b.xy - a.xy;
	float2 dist2 = dist * dist;
	float mag2 = dist2.x + dist2.y;

	if(mag2 == 0)
	{
		dist.y -= 1;
		dist2 = dist * dist;
		mag2 = dist2.x + dist2.y;
	}

	float rad = a.z + b.z;
	float rad2 = rad * rad;

	int cmp = mag2 < rad2 && mag2 > (1.0f / 2048.0f);
	float mag = 1.0 / (f32sqrt(mag2));
	float2 ret = (rad - 1.0f/mag) * (dist * mag);
	*normal = dist * mag;
	float2 choice[2] = {{0, 0}, ret};

	return choice[cmp];
}

GAMEMATH_INLINE
float2 box_circle_overlap(float4 box, float4 circle, float2* normal)
{
	float2 closest = f2clamp(
		circle.xy,
		box.xy - box.zw,
		box.xy + box.zw);
	float2 dist = closest - circle.xy;
	float2 dist2 = dist * dist;
	float mag2 = dist2.x + dist2.y;
	if(mag2 == 0)
	{
		dist.y -= 1;
		dist2 = dist * dist;
		mag2 = dist2.x + dist2.y;
	}
	float rad2 = circle.z * circle.z;

	int cmp = mag2 < rad2 && mag2 > (1.0f / 2048.0f);
	float mag = 1.0 / f32sqrt(mag2);
	float2 ret = closest - (circle.xy + (dist * mag) * circle.z);
	*normal = dist * mag;

	float2 choice[] = {{0, 0}, ret};

	return choice[cmp];
}

GAMEMATH_INLINE
float4 vertsCalcExtents(float2* verts, int numVerts)
{
	float4 extents;
	extents.xy = verts[0];
	extents.zw = verts[0];
	for(int i = 1; i < numVerts; ++i)
	{
		extents = f4boundsPoint(extents, verts[i]);
	}

	return extents;
}


GAMEMATH_INLINE
float2 vertCalcCenter(float2* verts, int numVerts)
{
	float2 center = 0;
	float fnum = (float)numVerts;
	for(int i = 0; i < numVerts; ++i)
	{
		center += verts[i] / fnum;
	}
	return center;
}

GAMEMATH_INLINE
void vertsMove(float2* verts, int numVerts, float2 diff)
{
	for(int i = 0; i < numVerts; ++i)
	{
		verts[i] += diff;
	}
}

GAMEMATH_INLINE
bool vertsContainsPoint(float2* verts, int numVerts, float2 p)
{
	bool inside = false; 
	float2 v1 = verts[0], v2;
	for(int i = 1; i <= numVerts; ++i)
	{
		v2 = verts[i == numVerts ? 0 : i];
		if(p.y > f32min(v1.y, v2.y))
		{
			if(p.y <= f32max(v1.y, v2.y))
			{
				if(p.x <= f32max(v1.x, v2.x))
				{
					float2 d = v2 - v1;
					float xint = (p.y - v1.y) * d.x / d.y + v1.x;
					if(v1.x == v2.x || p.x <= xint)
					{
						inside = !inside;
					}
				}
			}
		}
		v1 = v2;
	}
	return inside;
}


GAMEMATH_INLINE
bool vertsContainsPointBB(float2* verts, int numVerts, float2 p, float4 bb)
{
	if(p.x < bb.x || p.y < bb.y || p.x > bb.z || p.y > bb.w) return false;
	int inside = 0; 
	float2 v1 = verts[0], v2;
	for(int i = 1; i <= numVerts; ++i)
	{
		//v2 = verts[i % numVerts];
		v2 = verts[i == numVerts ? 0 : i];
		if(p.y > f32min(v1.y, v2.y))
		{
			if(p.y <= f32max(v1.y, v2.y))
			{
				if(p.x <= f32max(v1.x, v2.x))
				{
					float2 d = v2 - v1;
					float xint = (p.y - v1.y) * d.x / d.y + v1.x;
					if(v1.x == v2.x || p.x <= xint)
					{
						inside++;
					}
				}
			}
		}
		v1 = v2;
	}
	return (inside & 1) == 1;
}

GAMEMATH_INLINE
int vertsConvexHull(float2* verts, int numVerts, float2* outVerts, int maxOutVerts)
{
	if(numVerts <= 3) 
	{
		for(int i = 0; i < numVerts; ++i)
		{
			outVerts[i] = verts[i];
		}
		return numVerts;
	}

	int k = 0;

	for(int i = 1; i < numVerts; ++i)
	{
		float2 a = verts[i];
		int j = i;
		while(j > 0 && f2cmp(verts[j-1], a))
		{
			verts[j] = verts[j-1];
			j--;
		}
		verts[j] = a;
	}

	for(int i = 0; i < numVerts; ++i)
	{
		while(k >= 2 && f2crossOrigin(outVerts[k-1], verts[i], outVerts[k-2]) <= 0) k--;
		if(k < maxOutVerts) outVerts[k++] = verts[i];
	}

	for(int i = numVerts, t = k + 1; i > 0; i--)
	{
		while(k >= t && f2crossOrigin(outVerts[k-1], verts[i-1], outVerts[k-2]) <= 0) k--;
		if(k < maxOutVerts) outVerts[k++] = verts[i-1];
	}
	return k - 1;
}

// TODO(will) vertsIntersect (SAT), vertsClosestPoint
// TODO(will) rtriIntersect, rtriClosestPoint 
// TODO(will) lineIntersect, lineClosestPoint (w/ radius for capsule?)

GAMEMATH_INLINE
float2 lineClosestPoint(float2 a, float2 b, float2 p, float* time)
{
	float2 ab = b - a;
	float t = f2dot(p - a, ab) / f2mag2(ab);
	t = f32clamp(t, 0, 1);
	if(time) *time = t;
	return a + t * ab;
}

GAMEMATH_INLINE 
float dist2ToLine(float2 a, float2 b, float2 p)
{
	float2 ab = b - a;
	float2 ap = p - a;
	float2 bp = p - b;
	float e = f2dot(ap, ab);
	if(e <= 0) return f2mag2(ap);
	float f = f2mag2(ab);
	if(e >= f) return f2mag2(bp);
	return f2mag2(ap) - e * e / f;
}

GAMEMATH_INLINE
float2 f2mulMat2x2(float2 a, float4 m)
{
	return (float2) {
		f2dot(a, m.xy),
		f2dot(a, m.zw)
	};
}



GAMEMATH_INLINE
void ident4x4(float* mat)
{
	mat[0] = 1.0f;
	mat[1] = 0.0f;
	mat[2] = 0.0f;
	mat[3] = 0.0f;

	mat[4] = 0.0f;
	mat[5] = 1.0f;
	mat[6] = 0.0f;
	mat[7] = 0.0f;

	mat[8] = 0.0f;
	mat[9] = 0.0f;
	mat[10] = 1.0f;
	mat[11] = 0.0f;

	mat[12] = 0.0f;
	mat[13] = 0.0f;
	mat[14] = 0.0f;
	mat[15] = 1.0f;
}

GAMEMATH_INLINE
void ortho4x4(float* ortho, float4 screen, float nearplane, float farplane)
{
	ortho[0] = 2.0f / (screen.z - screen.x);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = -1.0f * (screen.x + screen.z) / (screen.z - screen.x);

	ortho[4] = 0;
	ortho[5] = 2.0f / (screen.y - screen.w);
	ortho[6] = 0;
	ortho[7] = -1 * (screen.y + screen.w) / (screen.y - screen.w);

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = (-2.0f / (farplane - nearplane));
	ortho[11] = (-1.0f * (farplane + nearplane) / (farplane - nearplane));

	ortho[12] = 0;
	ortho[13] = 0;
	ortho[14] = 0;
	ortho[15] = 1.0f;
}

GAMEMATH_INLINE
void perspective4x4(float* ortho, float2 screen, float nearplane, float farplane)
{
	ortho[0] = 2.0f * nearplane / (screen.x);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = 2.0f * nearplane / (screen.y);
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = farplane / (nearplane - farplane);
	ortho[11] = -1;

	ortho[12] = 0;
	ortho[13] = 0;
	ortho[14] = nearplane * farplane / (nearplane - farplane);
	ortho[15] = 0;
}

GAMEMATH_INLINE
void perspective4x4fov(float* ortho, float fovyRads, float aspect, float nearplane, float farplane)
{
	//float yscale = 1.0f / tanf(fovyRads / 2);
	float yscale = 1.0f / (fovyRads / 2);
	float xscale = yscale = aspect;

	ortho[0] = xscale;
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = 0;

	ortho[4] = 0;
	ortho[5] = yscale;
	ortho[6] = 0;
	ortho[7] = 0;

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = farplane / (nearplane - farplane);
	ortho[11] = -1;

	ortho[12] = 0;
	ortho[13] = 0;
	ortho[14] = nearplane * farplane / (nearplane - farplane);
	ortho[15] = 0;
}

// TODO(will) matmul, lookat, euler angles, quaternions

// random number generator

typedef struct rng_state
{
	uint64_t s[4];
} rng_state;

GAMEMATH_INLINE
uint64_t rng_u64rand(rng_state* rng)
{
	//this is xoshiro256** with rotateLeft inlined
	//I looked for a pcg64 implementation but couldn't
	//find one, and all the FUD seems to ignore this
	//generator, so here it is
	uint64_t ss = rng->s[1] * 5;
	uint64_t result = ((ss << 7) | (ss >> (64 - 7))) * 9;
	uint64_t t = rng->s[1] << 17;
	rng->s[2] ^= rng->s[0];
	rng->s[3] ^= rng->s[1];
	rng->s[1] ^= rng->s[2];
	rng->s[0] ^= rng->s[3];
	rng->s[2] ^= t;
	rng->s[3] = ((rng->s[3] << 45) | (rng->s[3] >> (64 - 45)));
	return result;
}

// [0, UINT64_MAX] -> 0,1
GAMEMATH_INLINE
double u64_to_normalized_f64(uint64_t x)
{
	union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | x >> 12 };
		return u.d - 1.0;
}

GAMEMATH_INLINE
double rng_f64rand(rng_state* rng)
{
	uint64_t x = rng_u64rand(rng);
	return u64_to_normalized_f64(x);
}

GAMEMATH_INLINE
uint2 rng_u2rand(rng_state* rng)
{
	uint64_t r = rng_u64rand(rng);
	return (uint2){(uint32_t)(r & 0xFFFFFFFFu), (uint32_t)(r >> 32)};
}

GAMEMATH_INLINE
int2 rng_i2rand(rng_state* rng)
{
	uint2 r = rng_u2rand(rng);
	return *(int2*)&r;
}

GAMEMATH_INLINE
uint32_t rng_u32rand(rng_state* rng)
{
	return rng_u64rand(rng) >> 32;
}

GAMEMATH_INLINE
int i32rand(rng_state* rng)
{
	uint32_t r = rng_u64rand(rng) >> 32;
	return *(int*)&r;
}

// [0, UINT32_MAX] -> 0,1
GAMEMATH_INLINE
float u32_to_normalized_f32(uint32_t x)
{
	union { uint32_t i; float d; } u;
	u.i = (127 << 23) | (x >> 9);
	return u.d - 1.0;
}

GAMEMATH_INLINE
float rng_f32rand(rng_state* rng)
{
	return u32_to_normalized_f32(rng_u32rand(rng));
}

GAMEMATH_INLINE
float2 rng_f2rand(rng_state* rng)
{
	uint2 r = rng_u2rand(rng);
	return (float2){
		u32_to_normalized_f32(r.x),
		u32_to_normalized_f32(r.y)
	};
}

GAMEMATH_INLINE
float2 rng_f2randrange(rng_state* rng, float2 lo, float2 hi)
{
	return f2lerp2(lo, hi, rng_f2rand(rng));
}

GAMEMATH_INLINE
uint64_t splitmix64(uint64_t* x)
{
	*x += UINT64_C(0x9E3779B97F4A7C15);
	uint64_t z = *x;
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return z ^ (z >> 31);	
}

GAMEMATH_INLINE float2 f32rand(uint64_t* seed)
{
	uint64_t r = splitmix64(seed);
	uint2 u = (uint2){(uint32_t)(r & 0xFFFFFFFFu), (uint32_t)(r >> 32)};
	return (float2){
		u32_to_normalized_f32(u.x),
		u32_to_normalized_f32(u.y)
	};
}


GAMEMATH_INLINE
void init_rng(rng_state* rng, uint64_t seed)
{
	// pre-seed the seed with a big prime 
	// (xxhash prime64 number 4)
	seed += 0x85EBCA77C2B2AE63ULL;

	// "warm up" the seed, if we 
	// got 0 or another bad nmuber
	for(int i = 0; i < 8; ++i)
	{
		splitmix64(&seed);
	}

	// fill out the state with rng numbers
	for(int i = 0; i < 4; ++i)
	{
		rng->s[i] = splitmix64(&seed);
	}

	// warm up the rng_state by generating 
	// a bunch of rng numbers
	for(int i = 0; i < 16; ++i)
	{
		rng_u64rand(rng);
	}
}

// hashing

// murmur3 32-bit

// these are technically UB versions 
// because they use type punning to convert
// numerical types to uint32_t

uint32_t murmur3(const void *key, int len, uint32_t h) 
{
	// main body, work on 32-bit blocks at a time
	for (int i=0;i<len/4;i++) 
	{
		uint32_t k = ((uint32_t*) key)[i]*0xcc9e2d51;
		k = ((k << 15) | (k >> 17))*0x1b873593;
		h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	}

	// load/mix up to 3 remaining tail bytes into a tail block
	uint32_t t = 0;
	uint8_t *tail = ((uint8_t*) key) + 4*(len/4); 
	switch(len & 3) 
	{
		case 3: t ^= tail[2] << 16;
		case 2: t ^= tail[1] <<  8;
		case 1: { 
			t ^= tail[0] <<  0;
			h ^= ((0xcc9e2d51*t << 15) | (0xcc9e2d51*t >> 17))*0x1b873593;
		}
	}

	// finalization mix, including key length
	h = ((h^len) ^ ((h^len) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}

// specializations

uint32_t u32murmur3(uint32_t val, uint32_t h) 
{
	uint32_t k = val*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	h = ((h^4) ^ ((h^4) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}


uint32_t i32murmur3(int ival, uint32_t h) 
{
	uint32_t val = *(uint32_t*)&ival;
	uint32_t k = val*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	h = ((h^4) ^ ((h^4) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}

// useful for spatial hashing

uint32_t u2murmur3(uint2 val, uint32_t h) 
{
	uint32_t k;
	k = val.x*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	k = val.y*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	h = ((h^8) ^ ((h^8) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}

uint32_t f2murmur3(float2 val, uint32_t h) 
{
	float x = val.x, y = val.y;
	uint32_t k;
	k = (*(uint32_t*)&x)*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	k = (*(uint32_t*)&y)*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	h = ((h^8) ^ ((h^8) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}

uint32_t i2murmur3(int2 val, uint32_t h) 
{
	int x = val.x, y = val.y;
	uint32_t k;
	k = (*(uint32_t*)&x)*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	k = (*(uint32_t*)&y)*0xcc9e2d51;
	k = ((k << 15) | (k >> 17))*0x1b873593;
	h = (((h^k) << 13) | ((h^k) >> 19))*5 + 0xe6546b64;
	h = ((h^8) ^ ((h^8) >> 16))*0x85ebca6b;
	h = (h ^ (h >> 13))*0xc2b2ae35;
	return h ^ (h >> 16); 
}
