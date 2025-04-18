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
		static auto checkHasGlobalDecompress(int)
			-> decltype(Compressor<T>::Decompress(TypeTraits::DeclareValue<T&>(), TypeTraits::DeclareValue<ConstBitView&>()), uint8());
		template<typename T>
		static uint16 checkHasGlobalDecompress(...);
		template<typename Type>
		inline static constexpr bool HasGlobalDecompress = sizeof(checkHasGlobalDecompress<Type>(0)) == sizeof(uint8);

		template<typename T>
		static auto checkMemberDecompress(int)
			-> decltype(TypeTraits::DeclareValue<T&>().Decompress(TypeTraits::DeclareValue<ConstBitView&>()), uint8());
		template<typename T>
		static uint16 checkMemberDecompress(...);

		template<typename Type>
		inline static constexpr bool HasMemberDecompress = sizeof(checkMemberDecompress<Type>(0)) == sizeof(uint8);

		template<typename Type>
		inline static constexpr bool CanDecompress = HasGlobalDecompress<Type> || HasMemberDecompress<Type>;
	}

	template<typename Type>
	[[nodiscard]] constexpr bool Decompress(Type& target, ConstBitView& source, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		if constexpr (Internal::HasGlobalDecompress<Type>)
		{
			return Compressor<Type>::Decompress(target, source);
		}
		else if constexpr (Internal::HasMemberDecompress<Type>)
		{
			return target.Decompress(source);
		}
		else if constexpr (Reflection::IsReflected<Type>)
		{
			const auto& reflectedType = Reflection::GetType<Type>();

			if constexpr (reflectedType.HasProperties)
			{
				bool wasDecompressed{true};
				reflectedType.m_properties.ForEach(
					[&source, &owner = target, requiredFlags, &wasDecompressed](auto& value) mutable
					{
						if (requiredFlags.AreAnySet() && !value.m_flags.AreAllSet(requiredFlags))
						{
							return;
						}

						using ReflectedPropertyType = TypeTraits::WithoutReference<decltype(value)>;
						using ReturnType = typename ReflectedPropertyType::ReturnType;
						using PropertyType = TypeTraits::WithoutConst<TypeTraits::WithoutReference<ReturnType>>;

						if constexpr (TypeTraits::IsReference<ReturnType> && !TypeTraits::IsConst<ReturnType>)
						{
							// In-place decompress
							PropertyType& propertyValue = value.GetValue(owner, Invalid);
							wasDecompressed &= Decompress(propertyValue, source, requiredFlags);
						}
						else if constexpr (TypeTraits::IsDefaultConstructible<PropertyType>)
						{
							PropertyType propertyValue;
							wasDecompressed &= Decompress(propertyValue, source, requiredFlags);
							value.SetValueBatched(owner, Invalid, Move(propertyValue));
						}
						else
						{
							wasDecompressed = false;
						}
					}
				);
				return wasDecompressed;
			}
			else if (TypeTraits::IsEmpty<Type>)
			{
				return true;
			}
			else
			{
				// Read the whole uncompressed type
				return source.UnpackAndSkip(BitView::Make(target));
			}
		}
		else if (TypeTraits::IsEmpty<Type>)
		{
			return true;
		}
		else
		{
			// Type doesn't support compression
			return source.UnpackAndSkip(BitView::Make(target));
		}
	}

	template<typename Type>
	[[nodiscard]] constexpr Optional<Type> Decompress(ConstBitView& source, const EnumFlags<Reflection::PropertyFlags> requiredFlags = {})
	{
		Type value;
		const bool wasDecompressed = Decompress<Type>(value, source, requiredFlags);
		return Optional<Type>{Move(value), wasDecompressed};
	}
}
