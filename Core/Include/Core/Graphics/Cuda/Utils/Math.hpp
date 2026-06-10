#pragma once
#include <cuda_runtime.h>
#include <helper_math.h>
#include <math_constants.h>

namespace Core::Graphics::Cuda::Math
{
	inline constexpr float Pi = CUDART_PI_F;
	inline constexpr float InvPi = 1.0f / CUDART_PI_F;
	inline constexpr float TwoPi = 2.0f * CUDART_PI_F;
	inline constexpr float InvTwoPi = 1.0f / TwoPi;

	__host__ __device__ __forceinline__ float At(float2 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			default:
				return 0.0f;
		}
	}

	__host__ __device__ __forceinline__ float At(float3 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			default:
				return 0.0f;
		}
	}

	__host__ __device__ __forceinline__ float At(float4 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			case 3:
				return number.w;
			default:
				return 0.0f;
		}
	}

	template<typename T>
	__device__ __forceinline__ T Min(T a, T b)
	{
		return a < b ? a : b;
	}
	
	template<typename T>
	__device__ __forceinline__ T Max(T a, T b)
	{
		return a > b ? a : b;
	}
	
	__host__ __device__ __forceinline__ float MinComponent(float3 vector)
	{
		return fminf(vector.x, fminf(vector.y, vector.z));
	}

	__host__ __device__ __forceinline__ float MinComponent(float4 vector)
	{
		return fminf(vector.x, fminf(vector.y, fminf(vector.z, vector.w)));
	}

	__host__ __device__ __forceinline__ float MaxComponent(float3 vector)
	{
		return fmaxf(vector.x, fmaxf(vector.y, vector.z));
	}

	__host__ __device__ __forceinline__ float MaxComponent(float4 vector)
	{
		return fmaxf(vector.x, fmaxf(vector.y, fmaxf(vector.z, vector.w)));
	}

	struct Frame
	{
		float3 tangent;
		float3 bitangent;
		float3 normal;

		__host__ __device__ __forceinline__ float3 ToLocal(float3 direction) const
		{
			return make_float3(dot(direction, tangent), dot(direction, bitangent), dot(direction, normal));
		}

		__host__ __device__ __forceinline__ float3 FromLocal(float3 direction) const
		{
			return direction.x * tangent + direction.y * bitangent + direction.z * normal;
		}
	};

	__host__ __device__ __forceinline__ float3 SafeNormalize(float3 vector, float3 fallback)
	{
		float lengthSquared = dot(vector, vector);

		if (lengthSquared > 1e-16f)
		{
			return vector * rsqrtf(lengthSquared);
		}

		return fallback;
	}
	
	__host__ __device__ __forceinline__ Frame BuildFrame(float3 normal)
	{
		float3 tangent;
		if (fabsf(normal.x) > fabsf(normal.z))
		{
			tangent = normalize(make_float3(normal.y, -normal.x, 0.0f)); // cross shortcut - n x (0, 0, 1) = ( n.y, -n.x, 0 )
		}
		else
		{
			tangent = normalize(make_float3(0.0f, normal.z, -normal.y)); // cross shortcut - n x (1, 0, 0) = ( 0, n.z, -n.y )
		}

		float3 bitangent = cross(normal, tangent);
		return Frame{ tangent, bitangent, normal };
	}
	
	__host__ __device__ __forceinline__ Frame BuildFrame(float3 normal, float3 tangent, float tangentSign = 1.0f)
	{
		// maybe normal and tangent should be expected to be normalized and so the caller should be responsible for normalization (safe normalization if needed)
		// base tangent being parallel to normal can cause bitangent to be zero -> need to ensure normal is not parallel to tangent
		normal = SafeNormalize(normal, make_float3(0.0f, 0.0f, 1.0f));

		Math::Frame fallback = BuildFrame(normal);
		tangent = tangent - normal * dot(normal, tangent);
		tangent = SafeNormalize(tangent, fallback.tangent);

		float3 bitangent = cross(normal, tangent) * tangentSign;
		return Math::Frame{ tangent, bitangent, normal };
	}

	struct Complex 
	{
		__host__ __device__ __forceinline__ Complex(float real) : real(real), imaginary(0) {}
		__host__ __device__ __forceinline__ Complex(float real, float imaginary) : real(real), imaginary(imaginary) {}

		__host__ __device__ __forceinline__ Complex operator-() const { return {-real, -imaginary}; }
		__host__ __device__ __forceinline__ Complex operator+(Complex z) const { return {real + z.real, imaginary + z.imaginary}; }
		__host__ __device__ __forceinline__ Complex operator-(Complex z) const { return {real - z.real, imaginary - z.imaginary}; }
		__host__ __device__ __forceinline__ Complex operator*(Complex z) const {
			return {real * z.real - imaginary * z.imaginary, real * z.imaginary + imaginary * z.real};
		}

		__host__ __device__ __forceinline__ Complex operator/(Complex z) const {
			float scale = 1 / (z.real * z.real + z.imaginary * z.imaginary);
			return {scale * (real * z.real + imaginary * z.imaginary), scale * (imaginary * z.real - real * z.imaginary)};
		}
		
		friend __host__ __device__ __forceinline__ Complex operator+(float value, Complex z) {
			return Complex(value) + z;
		}

		friend __host__ __device__ __forceinline__ Complex operator-(float value, Complex z) {
			return Complex(value) - z;
		}

		friend __host__ __device__ __forceinline__ Complex operator*(float value, Complex z) {
			return Complex(value) * z;
		}

		friend __host__ __device__ __forceinline__ Complex operator/(float value, Complex z) {
			return Complex(value) / z;
		}

		float real, imaginary;
	};
	

	__host__ __device__ __forceinline__ float Normalize(const Complex& z) 
	{
		return z.real * z.real + z.imaginary * z.imaginary;
	}

	__host__ __device__ __forceinline__ float Abs(const Complex& z) 
	{
		return sqrtf(Normalize(z));
	}

	__host__ __device__ __forceinline__ Complex Sqrt(const Complex& z)
	 {
		float n = Abs(z);
		float t1 = sqrt(0.5f * (n + Abs(z.real)));
		float t2 = 0.5f * z.imaginary / t1;

		if (n == 0)
			return 0;

		if (z.real >= 0)
			return {t1, t2};
		else
			return {fabsf(t2), copysignf(t1, z.imaginary)};
	}

	__host__ __device__ __forceinline__ bool SameHemisphere(float3 w, float3 wp)
	{
		return w.z * wp.z > 0.0f;
	}
	
	__host__ __device__ __forceinline__ float CosTheta(float3 w)
	{
		return w.z;
	}
	
	__host__ __device__ __forceinline__ float Cos2Theta(float3 w)
	{
		return w.z * w.z;
	}

	__host__ __device__ __forceinline__ float AbsCosTheta(float3 w)
	{
		return fabsf(w.z);
	}

	__host__ __device__ __forceinline__ float Sin2Theta(float3 w)
	{
		return fmaxf(0.0f, 1.0f - Cos2Theta(w));
	}

	__host__ __device__ __forceinline__ float SinTheta(float3 w)
	{
		return sqrtf(Sin2Theta(w));
	}

	__host__ __device__ __forceinline__ float TanTheta(float3 w)
	{
		return SinTheta(w) / CosTheta(w);
	}
	
	__host__ __device__ __forceinline__ float Tan2Theta(float3 w)
	{
		return Sin2Theta(w) / Cos2Theta(w);
	}

	__host__ __device__ __forceinline__ float CosPhi(float3 w) 
	{
		float sinTheta = SinTheta(w);
		return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1.f, 1.f);
	}

	__host__ __device__ __forceinline__ float SinPhi(float3 w) 
	{
		float sinTheta = SinTheta(w);
		return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1.f, 1.f);
	}
	
	__host__ __device__ __forceinline__ float Sqr(float value)
	{
		return value * value;
	}
	
	__host__ __device__ __forceinline__ float SafeSqrt(float value)
	{
		return sqrtf(fmaxf(0.0f, value));
	}
}