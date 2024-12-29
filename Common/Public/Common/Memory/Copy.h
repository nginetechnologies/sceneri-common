#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Math/CoreNumericTypes.h>

#define USE_VECTORIZED_COPY 0
#if USE_VECTORIZED_COPY
#include <Common/Math/Vectorization/Packed.h>
#include <Common/Memory/Prefetch.h>
#endif

#if PLATFORM_WINDOWS
extern "C"
{
	[[nodiscard]] void* __cdecl memcpy(void* dest, const void* src, size_t n);
	[[nodiscard]] void* __cdecl memmove(void* destination, const void* source, size_t num);
}
#endif

namespace ngine::Memory
{
	FORCE_INLINE NO_DEBUG void CopyWithoutOverlap(void* pDestination, const void* pSource, const size requestedSize) noexcept
	{
#if COMPILER_CLANG || COMPILER_GCC
		__builtin_memcpy(pDestination, pSource, requestedSize);
#else
		memcpy(pDestination, pSource, requestedSize);
#endif
	}

	template<size Size>
	FORCE_INLINE NO_DEBUG void CopyWithoutOverlap(void* pDestination, const void* pSource) noexcept
	{
#if COMPILER_CLANG || COMPILER_GCC
		__builtin_memcpy(pDestination, pSource, Size);
#else
		memcpy(pDestination, pSource, Size);
#endif
	}

	FORCE_INLINE NO_DEBUG void* CopyWithOverlap(void* pDestination, const void* pSource, const size size) noexcept
	{
#if COMPILER_CLANG || COMPILER_GCC
		return __builtin_memmove(pDestination, pSource, size);
#else
		return memmove(pDestination, pSource, size);
#endif
	}

#if USE_VECTORIZED_COPY
	inline static constexpr size CacheSize = 0x200000;

	namespace Internal
	{
		template<size Size, bool IsAligned>
		FORCE_INLINE void SmallCopy(unsigned char* pDestination, const unsigned char* pSource)
		{
			// TODO: Generalize this using Vectorization::Packed, select maximum packed type possible.

			constexpr size num256 = Size / sizeof(__m256i);
			for (size i = 0; i < num256; ++i)
			{
				if constexpr (IsAligned)
				{
					_mm256_store_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource)));
				}
				else
				{
					_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSource)));
				}

				pSource += sizeof(__m256i);
				pDestination += sizeof(__m256i);
			}

			constexpr size remainingSizeAfter256 = Size - num256 * sizeof(__m256i);
			constexpr size num128 = remainingSizeAfter256 / sizeof(__m128i);
			static_assert(num128 <= 1);
			if constexpr (num128 > 0)
			{
				if constexpr (IsAligned)
				{
					_mm_store_si128(reinterpret_cast<__m128i*>(pDestination), _mm_load_si128(reinterpret_cast<const __m128i*>(pSource)));
				}
				else
				{
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDestination), _mm_loadu_si128(reinterpret_cast<const __m128i*>(pSource)));
				}
				pSource += sizeof(__m128i);
				pDestination += sizeof(__m128i);
			}

			constexpr size remainingSizeAfter128 = remainingSizeAfter256 - num128 * sizeof(__m128i);
			constexpr size num64 = remainingSizeAfter128 / sizeof(uint64);
			static_assert(num64 <= 1);
			if constexpr (num64 > 0)
			{
				*reinterpret_cast<uint64*>(pDestination) = *reinterpret_cast<const uint64*>(pSource);
				pSource += sizeof(uint64);
				pDestination += sizeof(uint64);
			}

			constexpr size remainingSizeAfter64 = remainingSizeAfter128 - num64 * sizeof(uint64);
			constexpr size num32 = remainingSizeAfter64 / sizeof(uint32);
			static_assert(num32 <= 1);
			if constexpr (num32 > 0)
			{
				*reinterpret_cast<uint32*>(pDestination) = *reinterpret_cast<const uint32*>(pSource);
				pSource += sizeof(uint32);
				pDestination += sizeof(uint32);
			}

			constexpr size remainingSizeAfter32 = remainingSizeAfter64 - num32 * sizeof(uint32);
			constexpr size num16 = remainingSizeAfter32 / sizeof(uint16);
			static_assert(num16 <= 1);
			if constexpr (num16 > 0)
			{
				*reinterpret_cast<uint16*>(pDestination) = *reinterpret_cast<const uint16*>(pSource);
				pSource += sizeof(uint16);
				pDestination += sizeof(uint16);
			}

			constexpr size remainingSizeAfter16 = remainingSizeAfter32 - num16 * sizeof(uint16);
			constexpr size num8 = remainingSizeAfter16 / sizeof(uint8);
			static_assert(num8 <= 1);
			if constexpr (num8 > 0)
			{
				*reinterpret_cast<uint8*>(pDestination) = *reinterpret_cast<const uint8*>(pSource);
			}
		}

		enum class CopyType
		{
			// < 256
			Small,
			// <= CacheSize
			Medium,
			Big
		};

		template<CopyType CopyType_, size ElementAlignment, size ElementSize>
		FORCE_INLINE void CopyInternal(unsigned char* pDestination, const unsigned char* pSource, size dataSize)
		{
			static constexpr bool IsAligned = (ElementAlignment % 16) == 0;

			if constexpr (CopyType_ == CopyType::Small)
			{
#if RELEASE_BUILD
				switch (dataSize)
				{
					case 0:
						break;
					case 1:
						SmallCopy<1, IsAligned>(pDestination, pSource);
						break;
					case 2:
						SmallCopy<2, IsAligned>(pDestination, pSource);
						break;
					case 3:
						SmallCopy<3, IsAligned>(pDestination, pSource);
						break;
					case 4:
						SmallCopy<4, IsAligned>(pDestination, pSource);
						break;
					case 5:
						SmallCopy<5, IsAligned>(pDestination, pSource);
						break;
					case 6:
						SmallCopy<6, IsAligned>(pDestination, pSource);
						break;
					case 7:
						SmallCopy<7, IsAligned>(pDestination, pSource);
						break;
					case 8:
						SmallCopy<8, IsAligned>(pDestination, pSource);
						break;
					case 9:
						SmallCopy<9, IsAligned>(pDestination, pSource);
						break;
					case 10:
						SmallCopy<10, IsAligned>(pDestination, pSource);
						break;
					case 11:
						SmallCopy<11, IsAligned>(pDestination, pSource);
						break;
					case 12:
						SmallCopy<12, IsAligned>(pDestination, pSource);
						break;
					case 13:
						SmallCopy<13, IsAligned>(pDestination, pSource);
						break;
					case 14:
						SmallCopy<14, IsAligned>(pDestination, pSource);
						break;
					case 15:
						SmallCopy<15, IsAligned>(pDestination, pSource);
						break;
					case 16:
						SmallCopy<16, IsAligned>(pDestination, pSource);
						break;
					case 17:
						SmallCopy<17, IsAligned>(pDestination, pSource);
						break;
					case 18:
						SmallCopy<18, IsAligned>(pDestination, pSource);
						break;
					case 19:
						SmallCopy<19, IsAligned>(pDestination, pSource);
						break;
					case 20:
						SmallCopy<20, IsAligned>(pDestination, pSource);
						break;
					case 21:
						SmallCopy<21, IsAligned>(pDestination, pSource);
						break;
					case 22:
						SmallCopy<22, IsAligned>(pDestination, pSource);
						break;
					case 23:
						SmallCopy<23, IsAligned>(pDestination, pSource);
						break;
					case 24:
						SmallCopy<24, IsAligned>(pDestination, pSource);
						break;
					case 25:
						SmallCopy<25, IsAligned>(pDestination, pSource);
						break;
					case 26:
						SmallCopy<26, IsAligned>(pDestination, pSource);
						break;
					case 27:
						SmallCopy<27, IsAligned>(pDestination, pSource);
						break;
					case 28:
						SmallCopy<28, IsAligned>(pDestination, pSource);
						break;
					case 29:
						SmallCopy<29, IsAligned>(pDestination, pSource);
						break;
					case 30:
						SmallCopy<30, IsAligned>(pDestination, pSource);
						break;
					case 31:
						SmallCopy<31, IsAligned>(pDestination, pSource);
						break;
					case 32:
						SmallCopy<32, IsAligned>(pDestination, pSource);
						break;
					case 33:
						SmallCopy<33, IsAligned>(pDestination, pSource);
						break;
					case 34:
						SmallCopy<34, IsAligned>(pDestination, pSource);
						break;
					case 35:
						SmallCopy<35, IsAligned>(pDestination, pSource);
						break;
					case 36:
						SmallCopy<36, IsAligned>(pDestination, pSource);
						break;
					case 37:
						SmallCopy<37, IsAligned>(pDestination, pSource);
						break;
					case 38:
						SmallCopy<38, IsAligned>(pDestination, pSource);
						break;
					case 39:
						SmallCopy<39, IsAligned>(pDestination, pSource);
						break;
					case 40:
						SmallCopy<40, IsAligned>(pDestination, pSource);
						break;
					case 41:
						SmallCopy<41, IsAligned>(pDestination, pSource);
						break;
					case 42:
						SmallCopy<42, IsAligned>(pDestination, pSource);
						break;
					case 43:
						SmallCopy<43, IsAligned>(pDestination, pSource);
						break;
					case 44:
						SmallCopy<44, IsAligned>(pDestination, pSource);
						break;
					case 45:
						SmallCopy<45, IsAligned>(pDestination, pSource);
						break;
					case 46:
						SmallCopy<46, IsAligned>(pDestination, pSource);
						break;
					case 47:
						SmallCopy<47, IsAligned>(pDestination, pSource);
						break;
					case 48:
						SmallCopy<48, IsAligned>(pDestination, pSource);
						break;
					case 49:
						SmallCopy<49, IsAligned>(pDestination, pSource);
						break;
					case 50:
						SmallCopy<50, IsAligned>(pDestination, pSource);
						break;
					case 51:
						SmallCopy<51, IsAligned>(pDestination, pSource);
						break;
					case 52:
						SmallCopy<52, IsAligned>(pDestination, pSource);
						break;
					case 53:
						SmallCopy<53, IsAligned>(pDestination, pSource);
						break;
					case 54:
						SmallCopy<54, IsAligned>(pDestination, pSource);
						break;
					case 55:
						SmallCopy<55, IsAligned>(pDestination, pSource);
						break;
					case 56:
						SmallCopy<56, IsAligned>(pDestination, pSource);
						break;
					case 57:
						SmallCopy<57, IsAligned>(pDestination, pSource);
						break;
					case 58:
						SmallCopy<58, IsAligned>(pDestination, pSource);
						break;
					case 59:
						SmallCopy<59, IsAligned>(pDestination, pSource);
						break;
					case 60:
						SmallCopy<60, IsAligned>(pDestination, pSource);
						break;
					case 61:
						SmallCopy<61, IsAligned>(pDestination, pSource);
						break;
					case 62:
						SmallCopy<62, IsAligned>(pDestination, pSource);
						break;
					case 63:
						SmallCopy<63, IsAligned>(pDestination, pSource);
						break;
					case 64:
						SmallCopy<64, IsAligned>(pDestination, pSource);
						break;
					case 65:
						SmallCopy<65, IsAligned>(pDestination, pSource);
						break;
					case 66:
						SmallCopy<66, IsAligned>(pDestination, pSource);
						break;
					case 67:
						SmallCopy<67, IsAligned>(pDestination, pSource);
						break;
					case 68:
						SmallCopy<68, IsAligned>(pDestination, pSource);
						break;
					case 69:
						SmallCopy<69, IsAligned>(pDestination, pSource);
						break;
					case 70:
						SmallCopy<70, IsAligned>(pDestination, pSource);
						break;
					case 71:
						SmallCopy<71, IsAligned>(pDestination, pSource);
						break;
					case 72:
						SmallCopy<72, IsAligned>(pDestination, pSource);
						break;
					case 73:
						SmallCopy<73, IsAligned>(pDestination, pSource);
						break;
					case 74:
						SmallCopy<74, IsAligned>(pDestination, pSource);
						break;
					case 75:
						SmallCopy<75, IsAligned>(pDestination, pSource);
						break;
					case 76:
						SmallCopy<76, IsAligned>(pDestination, pSource);
						break;
					case 77:
						SmallCopy<77, IsAligned>(pDestination, pSource);
						break;
					case 78:
						SmallCopy<78, IsAligned>(pDestination, pSource);
						break;
					case 79:
						SmallCopy<79, IsAligned>(pDestination, pSource);
						break;
					case 80:
						SmallCopy<80, IsAligned>(pDestination, pSource);
						break;
					case 81:
						SmallCopy<81, IsAligned>(pDestination, pSource);
						break;
					case 82:
						SmallCopy<82, IsAligned>(pDestination, pSource);
						break;
					case 83:
						SmallCopy<83, IsAligned>(pDestination, pSource);
						break;
					case 84:
						SmallCopy<84, IsAligned>(pDestination, pSource);
						break;
					case 85:
						SmallCopy<85, IsAligned>(pDestination, pSource);
						break;
					case 86:
						SmallCopy<86, IsAligned>(pDestination, pSource);
						break;
					case 87:
						SmallCopy<87, IsAligned>(pDestination, pSource);
						break;
					case 88:
						SmallCopy<88, IsAligned>(pDestination, pSource);
						break;
					case 89:
						SmallCopy<89, IsAligned>(pDestination, pSource);
						break;
					case 90:
						SmallCopy<90, IsAligned>(pDestination, pSource);
						break;
					case 91:
						SmallCopy<91, IsAligned>(pDestination, pSource);
						break;
					case 92:
						SmallCopy<92, IsAligned>(pDestination, pSource);
						break;
					case 93:
						SmallCopy<93, IsAligned>(pDestination, pSource);
						break;
					case 94:
						SmallCopy<94, IsAligned>(pDestination, pSource);
						break;
					case 95:
						SmallCopy<95, IsAligned>(pDestination, pSource);
						break;
					case 96:
						SmallCopy<96, IsAligned>(pDestination, pSource);
						break;
					case 97:
						SmallCopy<97, IsAligned>(pDestination, pSource);
						break;
					case 98:
						SmallCopy<98, IsAligned>(pDestination, pSource);
						break;
					case 99:
						SmallCopy<99, IsAligned>(pDestination, pSource);
						break;
					case 100:
						SmallCopy<100, IsAligned>(pDestination, pSource);
						break;
					case 101:
						SmallCopy<101, IsAligned>(pDestination, pSource);
						break;
					case 102:
						SmallCopy<102, IsAligned>(pDestination, pSource);
						break;
					case 103:
						SmallCopy<103, IsAligned>(pDestination, pSource);
						break;
					case 104:
						SmallCopy<104, IsAligned>(pDestination, pSource);
						break;
					case 105:
						SmallCopy<105, IsAligned>(pDestination, pSource);
						break;
					case 106:
						SmallCopy<106, IsAligned>(pDestination, pSource);
						break;
					case 107:
						SmallCopy<107, IsAligned>(pDestination, pSource);
						break;
					case 108:
						SmallCopy<108, IsAligned>(pDestination, pSource);
						break;
					case 109:
						SmallCopy<109, IsAligned>(pDestination, pSource);
						break;
					case 110:
						SmallCopy<110, IsAligned>(pDestination, pSource);
						break;
					case 111:
						SmallCopy<111, IsAligned>(pDestination, pSource);
						break;
					case 112:
						SmallCopy<112, IsAligned>(pDestination, pSource);
						break;
					case 113:
						SmallCopy<113, IsAligned>(pDestination, pSource);
						break;
					case 114:
						SmallCopy<114, IsAligned>(pDestination, pSource);
						break;
					case 115:
						SmallCopy<115, IsAligned>(pDestination, pSource);
						break;
					case 116:
						SmallCopy<116, IsAligned>(pDestination, pSource);
						break;
					case 117:
						SmallCopy<117, IsAligned>(pDestination, pSource);
						break;
					case 118:
						SmallCopy<118, IsAligned>(pDestination, pSource);
						break;
					case 119:
						SmallCopy<119, IsAligned>(pDestination, pSource);
						break;
					case 120:
						SmallCopy<120, IsAligned>(pDestination, pSource);
						break;
					case 121:
						SmallCopy<121, IsAligned>(pDestination, pSource);
						break;
					case 122:
						SmallCopy<122, IsAligned>(pDestination, pSource);
						break;
					case 123:
						SmallCopy<123, IsAligned>(pDestination, pSource);
						break;
					case 124:
						SmallCopy<124, IsAligned>(pDestination, pSource);
						break;
					case 125:
						SmallCopy<125, IsAligned>(pDestination, pSource);
						break;
					case 126:
						SmallCopy<126, IsAligned>(pDestination, pSource);
						break;
					case 127:
						SmallCopy<127, IsAligned>(pDestination, pSource);
						break;
					case 128:
						SmallCopy<128, IsAligned>(pDestination, pSource);
						break;
					case 129:
						SmallCopy<129, IsAligned>(pDestination, pSource);
						break;
					case 130:
						SmallCopy<130, IsAligned>(pDestination, pSource);
						break;
					case 131:
						SmallCopy<131, IsAligned>(pDestination, pSource);
						break;
					case 132:
						SmallCopy<132, IsAligned>(pDestination, pSource);
						break;
					case 133:
						SmallCopy<133, IsAligned>(pDestination, pSource);
						break;
					case 134:
						SmallCopy<134, IsAligned>(pDestination, pSource);
						break;
					case 135:
						SmallCopy<135, IsAligned>(pDestination, pSource);
						break;
					case 136:
						SmallCopy<136, IsAligned>(pDestination, pSource);
						break;
					case 137:
						SmallCopy<137, IsAligned>(pDestination, pSource);
						break;
					case 138:
						SmallCopy<138, IsAligned>(pDestination, pSource);
						break;
					case 139:
						SmallCopy<139, IsAligned>(pDestination, pSource);
						break;
					case 140:
						SmallCopy<140, IsAligned>(pDestination, pSource);
						break;
					case 141:
						SmallCopy<141, IsAligned>(pDestination, pSource);
						break;
					case 142:
						SmallCopy<142, IsAligned>(pDestination, pSource);
						break;
					case 143:
						SmallCopy<143, IsAligned>(pDestination, pSource);
						break;
					case 144:
						SmallCopy<144, IsAligned>(pDestination, pSource);
						break;
					case 145:
						SmallCopy<145, IsAligned>(pDestination, pSource);
						break;
					case 146:
						SmallCopy<146, IsAligned>(pDestination, pSource);
						break;
					case 147:
						SmallCopy<147, IsAligned>(pDestination, pSource);
						break;
					case 148:
						SmallCopy<148, IsAligned>(pDestination, pSource);
						break;
					case 149:
						SmallCopy<149, IsAligned>(pDestination, pSource);
						break;
					case 150:
						SmallCopy<150, IsAligned>(pDestination, pSource);
						break;
					case 151:
						SmallCopy<151, IsAligned>(pDestination, pSource);
						break;
					case 152:
						SmallCopy<152, IsAligned>(pDestination, pSource);
						break;
					case 153:
						SmallCopy<153, IsAligned>(pDestination, pSource);
						break;
					case 154:
						SmallCopy<154, IsAligned>(pDestination, pSource);
						break;
					case 155:
						SmallCopy<155, IsAligned>(pDestination, pSource);
						break;
					case 156:
						SmallCopy<156, IsAligned>(pDestination, pSource);
						break;
					case 157:
						SmallCopy<157, IsAligned>(pDestination, pSource);
						break;
					case 158:
						SmallCopy<158, IsAligned>(pDestination, pSource);
						break;
					case 159:
						SmallCopy<159, IsAligned>(pDestination, pSource);
						break;
					case 160:
						SmallCopy<160, IsAligned>(pDestination, pSource);
						break;
					case 161:
						SmallCopy<161, IsAligned>(pDestination, pSource);
						break;
					case 162:
						SmallCopy<162, IsAligned>(pDestination, pSource);
						break;
					case 163:
						SmallCopy<163, IsAligned>(pDestination, pSource);
						break;
					case 164:
						SmallCopy<164, IsAligned>(pDestination, pSource);
						break;
					case 165:
						SmallCopy<165, IsAligned>(pDestination, pSource);
						break;
					case 166:
						SmallCopy<166, IsAligned>(pDestination, pSource);
						break;
					case 167:
						SmallCopy<167, IsAligned>(pDestination, pSource);
						break;
					case 168:
						SmallCopy<168, IsAligned>(pDestination, pSource);
						break;
					case 169:
						SmallCopy<169, IsAligned>(pDestination, pSource);
						break;
					case 170:
						SmallCopy<170, IsAligned>(pDestination, pSource);
						break;
					case 171:
						SmallCopy<171, IsAligned>(pDestination, pSource);
						break;
					case 172:
						SmallCopy<172, IsAligned>(pDestination, pSource);
						break;
					case 173:
						SmallCopy<173, IsAligned>(pDestination, pSource);
						break;
					case 174:
						SmallCopy<174, IsAligned>(pDestination, pSource);
						break;
					case 175:
						SmallCopy<175, IsAligned>(pDestination, pSource);
						break;
					case 176:
						SmallCopy<176, IsAligned>(pDestination, pSource);
						break;
					case 177:
						SmallCopy<177, IsAligned>(pDestination, pSource);
						break;
					case 178:
						SmallCopy<178, IsAligned>(pDestination, pSource);
						break;
					case 179:
						SmallCopy<179, IsAligned>(pDestination, pSource);
						break;
					case 180:
						SmallCopy<180, IsAligned>(pDestination, pSource);
						break;
					case 181:
						SmallCopy<181, IsAligned>(pDestination, pSource);
						break;
					case 182:
						SmallCopy<182, IsAligned>(pDestination, pSource);
						break;
					case 183:
						SmallCopy<183, IsAligned>(pDestination, pSource);
						break;
					case 184:
						SmallCopy<184, IsAligned>(pDestination, pSource);
						break;
					case 185:
						SmallCopy<185, IsAligned>(pDestination, pSource);
						break;
					case 186:
						SmallCopy<186, IsAligned>(pDestination, pSource);
						break;
					case 187:
						SmallCopy<187, IsAligned>(pDestination, pSource);
						break;
					case 188:
						SmallCopy<188, IsAligned>(pDestination, pSource);
						break;
					case 189:
						SmallCopy<189, IsAligned>(pDestination, pSource);
						break;
					case 190:
						SmallCopy<190, IsAligned>(pDestination, pSource);
						break;
					case 191:
						SmallCopy<191, IsAligned>(pDestination, pSource);
						break;
					case 192:
						SmallCopy<192, IsAligned>(pDestination, pSource);
						break;
					case 193:
						SmallCopy<193, IsAligned>(pDestination, pSource);
						break;
					case 194:
						SmallCopy<194, IsAligned>(pDestination, pSource);
						break;
					case 195:
						SmallCopy<195, IsAligned>(pDestination, pSource);
						break;
					case 196:
						SmallCopy<196, IsAligned>(pDestination, pSource);
						break;
					case 197:
						SmallCopy<197, IsAligned>(pDestination, pSource);
						break;
					case 198:
						SmallCopy<198, IsAligned>(pDestination, pSource);
						break;
					case 199:
						SmallCopy<199, IsAligned>(pDestination, pSource);
						break;
					case 200:
						SmallCopy<200, IsAligned>(pDestination, pSource);
						break;
					case 201:
						SmallCopy<201, IsAligned>(pDestination, pSource);
						break;
					case 202:
						SmallCopy<202, IsAligned>(pDestination, pSource);
						break;
					case 203:
						SmallCopy<203, IsAligned>(pDestination, pSource);
						break;
					case 204:
						SmallCopy<204, IsAligned>(pDestination, pSource);
						break;
					case 205:
						SmallCopy<205, IsAligned>(pDestination, pSource);
						break;
					case 206:
						SmallCopy<206, IsAligned>(pDestination, pSource);
						break;
					case 207:
						SmallCopy<207, IsAligned>(pDestination, pSource);
						break;
					case 208:
						SmallCopy<208, IsAligned>(pDestination, pSource);
						break;
					case 209:
						SmallCopy<209, IsAligned>(pDestination, pSource);
						break;
					case 210:
						SmallCopy<210, IsAligned>(pDestination, pSource);
						break;
					case 211:
						SmallCopy<211, IsAligned>(pDestination, pSource);
						break;
					case 212:
						SmallCopy<212, IsAligned>(pDestination, pSource);
						break;
					case 213:
						SmallCopy<213, IsAligned>(pDestination, pSource);
						break;
					case 214:
						SmallCopy<214, IsAligned>(pDestination, pSource);
						break;
					case 215:
						SmallCopy<215, IsAligned>(pDestination, pSource);
						break;
					case 216:
						SmallCopy<216, IsAligned>(pDestination, pSource);
						break;
					case 217:
						SmallCopy<217, IsAligned>(pDestination, pSource);
						break;
					case 218:
						SmallCopy<218, IsAligned>(pDestination, pSource);
						break;
					case 219:
						SmallCopy<219, IsAligned>(pDestination, pSource);
						break;
					case 220:
						SmallCopy<220, IsAligned>(pDestination, pSource);
						break;
					case 221:
						SmallCopy<221, IsAligned>(pDestination, pSource);
						break;
					case 222:
						SmallCopy<222, IsAligned>(pDestination, pSource);
						break;
					case 223:
						SmallCopy<223, IsAligned>(pDestination, pSource);
						break;
					case 224:
						SmallCopy<224, IsAligned>(pDestination, pSource);
						break;
					case 225:
						SmallCopy<225, IsAligned>(pDestination, pSource);
						break;
					case 226:
						SmallCopy<226, IsAligned>(pDestination, pSource);
						break;
					case 227:
						SmallCopy<227, IsAligned>(pDestination, pSource);
						break;
					case 228:
						SmallCopy<228, IsAligned>(pDestination, pSource);
						break;
					case 229:
						SmallCopy<229, IsAligned>(pDestination, pSource);
						break;
					case 230:
						SmallCopy<230, IsAligned>(pDestination, pSource);
						break;
					case 231:
						SmallCopy<231, IsAligned>(pDestination, pSource);
						break;
					case 232:
						SmallCopy<232, IsAligned>(pDestination, pSource);
						break;
					case 233:
						SmallCopy<233, IsAligned>(pDestination, pSource);
						break;
					case 234:
						SmallCopy<234, IsAligned>(pDestination, pSource);
						break;
					case 235:
						SmallCopy<235, IsAligned>(pDestination, pSource);
						break;
					case 236:
						SmallCopy<236, IsAligned>(pDestination, pSource);
						break;
					case 237:
						SmallCopy<237, IsAligned>(pDestination, pSource);
						break;
					case 238:
						SmallCopy<238, IsAligned>(pDestination, pSource);
						break;
					case 239:
						SmallCopy<239, IsAligned>(pDestination, pSource);
						break;
					case 240:
						SmallCopy<240, IsAligned>(pDestination, pSource);
						break;
					case 241:
						SmallCopy<241, IsAligned>(pDestination, pSource);
						break;
					case 242:
						SmallCopy<242, IsAligned>(pDestination, pSource);
						break;
					case 243:
						SmallCopy<243, IsAligned>(pDestination, pSource);
						break;
					case 244:
						SmallCopy<244, IsAligned>(pDestination, pSource);
						break;
					case 245:
						SmallCopy<245, IsAligned>(pDestination, pSource);
						break;
					case 246:
						SmallCopy<246, IsAligned>(pDestination, pSource);
						break;
					case 247:
						SmallCopy<247, IsAligned>(pDestination, pSource);
						break;
					case 248:
						SmallCopy<248, IsAligned>(pDestination, pSource);
						break;
					case 249:
						SmallCopy<249, IsAligned>(pDestination, pSource);
						break;
					case 250:
						SmallCopy<250, IsAligned>(pDestination, pSource);
						break;
					case 251:
						SmallCopy<251, IsAligned>(pDestination, pSource);
						break;
					case 252:
						SmallCopy<252, IsAligned>(pDestination, pSource);
						break;
					case 253:
						SmallCopy<253, IsAligned>(pDestination, pSource);
						break;
					case 254:
						SmallCopy<254, IsAligned>(pDestination, pSource);
						break;
					case 255:
						SmallCopy<255, IsAligned>(pDestination, pSource);
						break;
					default:
						ExpectUnreachable();
				}
#else
				Memory::CopyWithoutOverlap(pDestination, pSource, dataSize);
#endif
			}
			else if constexpr (CopyType_ == CopyType::Medium)
			{
				if constexpr (!IsAligned)
				{
					// Read the first bytes to align the memory
					const size offset = IsAligned ? 0 : (32 - (reinterpret_cast<size>(pDestination) & 31)) & 31;
					const __m256i source = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSource));
					_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDestination), source);
					pSource += offset;
					pDestination += offset;
					dataSize -= offset;
				}

				for (; dataSize >= 128; dataSize -= 64, pDestination += 64, pSource += 64)
				{
					Memory::PrefetchLine<Memory::PrefetchType::Read, Memory::PrefetchLocality::KeepInAllPossibleCacheLevels>(
						reinterpret_cast<const char*>(pSource + 2)
					);

					// TODO: Generalize this using Vectorization::Packed, select maximum packed type possible.
					_mm256_store_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource)));
					_mm256_store_si256(
						reinterpret_cast<__m256i*>(pDestination) + 1,
						_mm256_load_si256(reinterpret_cast<const __m256i*>(pSource) + 1)
					);
				}
				_mm256_store_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource)));
				_mm256_store_si256(reinterpret_cast<__m256i*>(pDestination) + 1, _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource) + 1));
				pDestination += 64;
				pSource += 64;
				dataSize -= 64;

				// TODO: Check other cases where dataSize is guaranteed to be 0 where small copy can be skipped at compile-time
				constexpr bool CanSkipSmallCopy = IsAligned && ElementSize == 64;
				if constexpr (!CanSkipSmallCopy)
				{
					// Copy the remainder
					CopyInternal<CopyType::Small, ElementAlignment, ElementSize>(pDestination, pSource, dataSize);
				}
				else
				{
					Assert(dataSize == 0);
				}
			}
			else // CopyType::Big
			{
				if constexpr (!IsAligned)
				{
					// Read the first bytes to align the memory
					const size offset = IsAligned ? 0 : (32 - (reinterpret_cast<size>(pDestination) & 31)) & 31;
					const __m256i source = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSource));
					_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDestination), source);
					pSource += offset;
					pDestination += offset;
					dataSize -= offset;
				}

				for (; dataSize >= 128; dataSize -= 64, pDestination += 64, pSource += 64)
				{
					Memory::PrefetchLine<Memory::PrefetchType::Read, Memory::PrefetchLocality::KeepInAllPossibleCacheLevels>(
						reinterpret_cast<const char*>(pSource + 2)
					);

					// TODO: Generalize this using Vectorization::Packed, select maximum packed type possible.
					_mm256_stream_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource)));
					_mm256_stream_si256(
						reinterpret_cast<__m256i*>(pDestination) + 1,
						_mm256_load_si256(reinterpret_cast<const __m256i*>(pSource) + 1)
					);
				}
				_mm256_stream_si256(reinterpret_cast<__m256i*>(pDestination), _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource)));
				_mm256_stream_si256(reinterpret_cast<__m256i*>(pDestination) + 1, _mm256_load_si256(reinterpret_cast<const __m256i*>(pSource) + 1));
				_mm_sfence();
				pDestination += 64;
				pSource += 64;
				dataSize -= 64;

				// TODO: Check other cases where dataSize is guaranteed to be 0 where small copy can be skipped at compile-time
				constexpr bool CanSkipSmallCopy = IsAligned && ElementSize == 64;
				if constexpr (!CanSkipSmallCopy)
				{
					// Copy the remainder
					CopyInternal<CopyType::Small, ElementAlignment, ElementSize>(pDestination, pSource, dataSize);
				}
				else
				{
					Assert(dataSize == 0);
				}
			}
		}

		template<CopyType CopyType_, size ElementAlignment, size ElementSize>
		FORCE_INLINE NO_DEBUG void Copy(void* pDestination, const void* pSource, size dataSize)
		{
			CopyInternal<CopyType_, ElementAlignment, ElementSize>(
				static_cast<unsigned char*>(pDestination),
				static_cast<const unsigned char*>(pSource),
				dataSize
			);
		}
	}
#endif

	template<typename Type>
	FORCE_INLINE NO_DEBUG void CopyNonOverlappingElements(Type* pDestination, const Type* pSource, const size count) noexcept
	{
		const size dataSize = sizeof(Type) * count;
#if USE_VECTORIZED_COPY
		if constexpr (sizeof(Type) < 256)
		{
			if (dataSize < 256)
			{
				Internal::Copy<Internal::CopyType::Small, alignof(Type), sizeof(Type)>(pDestination, pSource, dataSize);
				return;
			}
		}
		if constexpr (sizeof(Type) <= CacheSize)
		{
			if (dataSize <= CacheSize)
			{
				Internal::Copy<Internal::CopyType::Medium, alignof(Type), sizeof(Type)>(pDestination, pSource, dataSize);
				return;
			}
		}

		Internal::Copy<Internal::CopyType::Big, alignof(Type), sizeof(Type)>(pDestination, pSource, dataSize);
#else
		Memory::CopyWithoutOverlap(pDestination, pSource, dataSize);
#endif
	}

	template<typename Type, size Count>
	FORCE_INLINE NO_DEBUG void CopyNonOverlappingElements(Type* pDestination, const Type* pSource) noexcept
	{
		constexpr size DataSize = sizeof(Type) * Count;
#if USE_VECTORIZED_COPY
		if constexpr (DataSize < 256)
		{
			Internal::Copy<Internal::CopyType::Small, alignof(Type), sizeof(Type)>(pDestination, pSource, DataSize);
		}
		else if constexpr (DataSize <= CacheSize)
		{
			Internal::Copy<Internal::CopyType::Medium, alignof(Type), sizeof(Type)>(pDestination, pSource, DataSize);
		}
		else
		{
			Internal::Copy<Internal::CopyType::Big, alignof(Type), sizeof(Type)>(pDestination, pSource, DataSize);
		}
#else
		Memory::CopyWithoutOverlap(pDestination, pSource, DataSize);
#endif
	}

	template<typename Type>
	FORCE_INLINE NO_DEBUG void CopyNonOverlappingElement(Type& destination, const Type& source) noexcept
	{
#if USE_VECTORIZED_COPY
		if constexpr (sizeof(Type) < 256)
		{
			Internal::Copy<Internal::CopyType::Small, alignof(Type), sizeof(Type)>(&destination, &source, sizeof(Type));
		}
		else if constexpr (sizeof(Type) <= CacheSize)
		{
			Internal::Copy<Internal::CopyType::Medium, alignof(Type), sizeof(Type)>(&destination, &source, sizeof(Type));
		}
		else
		{
			Internal::Copy<Internal::CopyType::Big, alignof(Type), sizeof(Type)>(&destination, &source, sizeof(Type));
		}
#else
		Memory::CopyWithoutOverlap(&destination, &source, sizeof(Type));
#endif
	}

	template<typename Type>
	FORCE_INLINE NO_DEBUG void CopyOverlappingElements(Type* pDestination, const Type* pSource, const size count) noexcept
	{
		CopyWithOverlap(pDestination, pSource, count * sizeof(Type));
	}

	template<typename Type>
	FORCE_INLINE NO_DEBUG void CopyOverlappingElement(Type& destination, const Type& source) noexcept
	{
		CopyOverlappingElements(destination, source, 1);
	}
}
