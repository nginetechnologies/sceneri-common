#pragma once

#include <Common/Compression/ForwardDeclarations/Compressor.h>
#include <Common/Memory/Containers/BitView.h>

#include <Common/Reflection/Type.h>
#include <Common/TypeTraits/IsEmpty.h>

namespace ngine::Compression
{
	namespace Internal
	{
		template<typename T>
		static auto checkHasGlobalCalculateFixedDataSize(int) -> decltype(uint32{Compressor<T>::CalculateCompressedDataSize()}, uint8());
		template<typename T>
		static uint16 checkHasGlobalCalculateFixedDataSize(...);
		template<typename Type>
		inline static constexpr bool HasGlobalCalculateFixedDataSize = sizeof(checkHasGlobalCalculateFixedDataSize<Type>(0)) == sizeof(uint8);

		template<typename T>
		static auto checkHasGlobalCalculateDynamicDataSize(int)
			-> decltype(uint32{Compressor<T>::CalculateCompressedDataSize(TypeTraits::DeclareValue<const T&>())}, uint8());
		template<typename T>
		static uint16 checkHasGlobalCalculateDynamicDataSize(...);
		template<typename Type>
		inline static constexpr bool HasGlobalCalculateDynamicDataSize = sizeof(checkHasGlobalCalculateDynamicDataSize<Type>(0)) ==
		                                                                 sizeof(uint8);

		template<typename T>
		static auto checkMemberCalculateFixedDataSize(int) -> decltype(uint32{T::CalculateCompressedDataSize()}, uint8());
		template<typename T>
		static uint16 checkMemberCalculateFixedDataSize(...);

		template<typename Type>
		inline static constexpr bool HasMemberCalculateFixedDataSize = sizeof(checkMemberCalculateFixedDataSize<Type>(0)) == sizeof(uint8);

		template<typename T>
		static auto checkMemberCalculateDynamicDataSize(int)
			-> decltype(uint32{TypeTraits::DeclareValue<const T&>().CalculateCompressedDataSize()}, uint8());
		template<typename T>
		static uint16 checkMemberCalculateDynamicDataSize(...);

		template<typename Type>
		inline static constexpr bool HasMemberCalculateDynamicDataSize = sizeof(checkMemberCalculateDynamicDataSize<Type>(0)) == sizeof(uint8);

		template<typename T>
		static auto checkHasGlobalCompress(int)
			-> decltype(Compressor<T>::Compress(TypeTraits::DeclareValue<const T&>(), TypeTraits::DeclareValue<BitView&>()), uint8());
		template<typename T>
		static uint16 checkHasGlobalCompress(...);
		template<typename Type>
		inline static constexpr bool HasGlobalCompress = sizeof(checkHasGlobalCompress<Type>(0)) == sizeof(uint8);

		template<typename T>
		static auto checkMemberCompress(int)
			-> decltype(TypeTraits::DeclareValue<const T&>().Compress(TypeTraits::DeclareValue<BitView&>()), uint8());
		template<typename T>
		static uint16 checkMemberCompress(...);

		template<typename Type>
		inline static constexpr bool HasMemberCompress = sizeof(checkMemberCompress<Type>(0)) == sizeof(uint8);

		template<typename Type>
		inline static constexpr bool HasGlobalCompressor =
			(HasGlobalCalculateDynamicDataSize<Type> || HasGlobalCalculateFixedDataSize<Type>)&&HasGlobalCompress<Type>;

		template<typename Type>
		inline static constexpr bool HasMemberCompressor =
			(HasMemberCalculateDynamicDataSize<Type> || HasMemberCalculateFixedDataSize<Type>)&&HasMemberCompress<Type>;

		template<typename Type>
		inline static constexpr bool CanCompress = HasGlobalCompressor<Type> || HasMemberCompressor<Type>;
	}

	//! Whether the type compressed size depends on the object itself
	template<typename Type>
	[[nodiscard]] constexpr bool IsDynamicallyCompressed(const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		if constexpr (Internal::HasGlobalCalculateFixedDataSize<Type> || Internal::HasMemberCalculateFixedDataSize<Type>)
		{
			return false;
		}
		else if constexpr (Internal::HasMemberCalculateDynamicDataSize<Type> || Internal::HasGlobalCalculateDynamicDataSize<Type>)
		{
			return true;
		}
		else if constexpr (Reflection::IsReflected<Type>)
		{
			const auto& reflectedType = Reflection::GetType<Type>();

			if constexpr (reflectedType.HasProperties)
			{
				bool hasDynamicProperties{false};
				reflectedType.m_properties.ForEach(
					[requiredFlags, &hasDynamicProperties](auto& value) mutable
					{
						if (requiredFlags.AreAnySet() && !value.m_flags.AreAllSet(requiredFlags))
						{
							return;
						}

						using PropertyType = typename TypeTraits::WithoutReference<decltype(value)>::Type;
						hasDynamicProperties |= IsDynamicallyCompressed<PropertyType>(requiredFlags);
					}
				);
				return hasDynamicProperties;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	//! Calculates the fixed compressed data size in bits that can be calculated per type
	//! Note: if the type contains dynamically compressed data (see IsDynamicallyCompressed) then CalculateDynamicDataSize must be called for
	//! each instance to calculate additional bits required
	template<typename Type>
	[[nodiscard]] constexpr uint32 CalculateFixedDataSize(const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		if constexpr (Internal::HasGlobalCalculateFixedDataSize<Type>)
		{
			return Compressor<Type>::CalculateCompressedDataSize();
		}
		else if constexpr (Internal::HasMemberCalculateFixedDataSize<Type>)
		{
			return Type::CalculateCompressedDataSize();
		}
		else if constexpr (Internal::HasMemberCalculateDynamicDataSize<Type> || Internal::HasGlobalCalculateDynamicDataSize<Type>)
		{
			// Will be calculated by calls to CalculateDynamicDataSize (requires an object)
			return 0;
		}
		else if constexpr (Reflection::IsReflected<Type>)
		{
			const auto& reflectedType = Reflection::GetType<Type>();

			if constexpr (reflectedType.HasProperties)
			{
				uint32 dataSize{0u};
				reflectedType.m_properties.ForEach(
					[requiredFlags, &dataSize](auto& value) mutable
					{
						if (requiredFlags.AreAnySet() && !value.m_flags.AreAllSet(requiredFlags))
						{
							return;
						}

						using PropertyType = typename TypeTraits::WithoutReference<decltype(value)>::Type;
						dataSize += CalculateFixedDataSize<PropertyType>(requiredFlags);
					}
				);
				return dataSize;
			}
			else if (TypeTraits::IsEmpty<Type>)
			{
				return 0;
			}
			else
			{
				// No compression supported, return the full type
				return sizeof(Type) * CharBitCount;
			}
		}
		else if (TypeTraits::IsEmpty<Type>)
		{
			return 0;
		}
		else
		{
			// Type doesn't support compression
			return sizeof(Type) * CharBitCount;
		}
	}

	//! Calculates the number of bits required for dynamic data in the object
	//! Note: Does not include fixed / non-dynamic data, call CalculateFixedDataSize first
	template<typename Type>
	[[nodiscard]] constexpr uint32 CalculateDynamicDataSize(const Type& source, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		if constexpr (Internal::HasGlobalCalculateFixedDataSize<Type> || Internal::HasMemberCalculateFixedDataSize<Type>)
		{
			// Calculated by calls to CalculateFixedDataSize
			return 0;
		}
		else if constexpr (Internal::HasMemberCalculateDynamicDataSize<Type>)
		{
			return source.CalculateCompressedDataSize();
		}
		else if constexpr (Internal::HasGlobalCalculateDynamicDataSize<Type>)
		{
			return Compressor<Type>::CalculateCompressedDataSize(source);
		}
		else if constexpr (Reflection::IsReflected<Type>)
		{
			const auto& reflectedType = Reflection::GetType<Type>();

			if constexpr (reflectedType.HasProperties)
			{
				uint32 dataSize{0u};
				reflectedType.m_properties.ForEach(
					[&owner = source, requiredFlags, &dataSize](auto& value) mutable
					{
						if (requiredFlags.AreAnySet() && !value.m_flags.AreAllSet(requiredFlags))
						{
							return;
						}

						using PropertyType = typename TypeTraits::WithoutReference<decltype(value)>::Type;
						if constexpr (IsDynamicallyCompressed<PropertyType>())
						{
							const PropertyType& propertyValue = value.GetValue(owner, Invalid);
							dataSize += CalculateDynamicDataSize(propertyValue, requiredFlags);
						}
					}
				);
				return dataSize;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	template<typename Type>
	[[nodiscard]] constexpr bool Compress(const Type& source, BitView& target, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		if constexpr (Internal::HasGlobalCompress<Type>)
		{
			return Compressor<Type>::Compress(source, target);
		}
		else if constexpr (Internal::HasMemberCompress<Type>)
		{
			return source.Compress(target);
		}
		else if constexpr (Reflection::IsReflected<Type>)
		{
			const auto& reflectedType = Reflection::GetType<Type>();

			if constexpr (reflectedType.HasProperties)
			{
				bool wasCompressed{true};
				reflectedType.m_properties.ForEach(
					[&target, &owner = source, requiredFlags, &wasCompressed](auto& value) mutable
					{
						if (requiredFlags.AreAnySet() && !value.m_flags.AreAllSet(requiredFlags))
						{
							return;
						}

						using PropertyType = typename TypeTraits::WithoutReference<decltype(value)>::Type;
						const PropertyType& propertyValue = value.GetValue(owner, Invalid);
						wasCompressed &= Compress(propertyValue, target, requiredFlags);
					}
				);
				return wasCompressed;
			}
			else if (TypeTraits::IsEmpty<Type>)
			{
				return true;
			}
			else
			{
				// Write the whole uncompressed type
				return target.PackAndSkip(ConstBitView::Make(source));
			}
		}
		else if (TypeTraits::IsEmpty<Type>)
		{
			return true;
		}
		else
		{
			// Type doesn't support compression
			return target.PackAndSkip(ConstBitView::Make(source));
		}
	}
}
