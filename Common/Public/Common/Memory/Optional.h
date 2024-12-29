#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/LifetimeBound.h>
#include <Common/Platform/Pure.h>
#include <Common/TypeTraits/WithoutConst.h>
#include <Common/TypeTraits/IsConst.h>
#include <Common/TypeTraits/IsCopyConstructible.h>
#include <Common/TypeTraits/IsCopyAssignable.h>
#include <Common/TypeTraits/IsMoveConstructible.h>
#include <Common/TypeTraits/IsMoveAssignable.h>
#include <Common/TypeTraits/IsTriviallyDestructible.h>
#include <Common/TypeTraits/IsConvertibleTo.h>
#include <Common/TypeTraits/HasConstructor.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsBaseOf.h>
#include <Common/TypeTraits/HasMemberFunction.h>
#include <Common/Assert/Assert.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/AddressOf.h>
#include <Common/Memory/Compare.h>
#include <Common/Memory/ForwardDeclarations/Optional.h>
#include <Common/Memory/Invalid.h>
#include <Common/Memory/New.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/CompilerWarnings.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

PUSH_MSVC_WARNINGS
DISABLE_MSVC_WARNINGS(5267)

namespace ngine
{
	namespace Internal
	{
		template<typename Type>
		struct TIsOptional
		{
			inline static constexpr bool Value = false;
		};

		template<typename Type>
		struct TIsOptional<Optional<Type>>
		{
			inline static constexpr bool Value = true;
		};

		template<typename Type>
		inline static constexpr bool IsOptional = TIsOptional<Type>::Value;

		template<bool IsTriviallyDestructible, typename T>
		struct OptionalBase
		{
			using MutableType = TypeTraits::WithoutConst<T>;

			NO_DEBUG constexpr OptionalBase()
			{
			}
			NO_DEBUG constexpr OptionalBase(MutableType&& element, const bool isValid = true)
				: m_element(Forward<MutableType>(element))
				, m_isValid(isValid)
			{
			}
			NO_DEBUG ~OptionalBase()
			{
				DestroyElement();
			}

			NO_DEBUG void DestroyElement()
			{
				if (m_isValid)
				{
					m_element.~MutableType();
					m_isValid = false;
				}
			}
		protected:
			union
			{
				MutableType m_element;
				alignas(MutableType) ByteType m_elementStorage[sizeof(MutableType)];
			};
			bool m_isValid = false;
		};

		template<typename T>
		struct OptionalBase<true, T>
		{
			using MutableType = TypeTraits::WithoutConst<T>;

			NO_DEBUG constexpr OptionalBase()
			{
			}
			NO_DEBUG constexpr OptionalBase(MutableType&& element, const bool isValid = true)
				: m_element(Forward<MutableType>(element))
				, m_isValid(isValid)
			{
			}
			NO_DEBUG void DestroyElement()
			{
				m_isValid = false;
			}
		protected:
			union
			{
				MutableType m_element;
				alignas(MutableType) ByteType m_elementStorage[sizeof(MutableType)];
			};
			bool m_isValid = false;
		};
	}

	template<typename T, typename Enable>
	struct TRIVIAL_ABI Optional : public Internal::OptionalBase<TypeTraits::IsTriviallyDestructible<T>, T>
	{
		using BaseType = Internal::OptionalBase<TypeTraits::IsTriviallyDestructible<T>, T>;
		using MutableType = TypeTraits::WithoutConst<T>;

		using BaseType::BaseType;

		NO_DEBUG constexpr Optional()
		{
		}
		NO_DEBUG constexpr Optional(InvalidType)
		{
		}
		// template<typename ContainedType = MutableType, typename = EnableIf<TypeTraits::IsCopyConstructible<ContainedType>>>
		NO_DEBUG constexpr Optional(const Optional& other)
		{
			if (other.m_isValid)
			{
				BaseType::m_isValid = other.m_isValid;
				new (Memory::GetAddressOf(BaseType::m_element)) MutableType(other.m_element);
			}
		}
		/*template<
		  typename ContainedType = T,
		  typename ContainedMutableType = MutableType,
		  typename = EnableIf<TypeTraits::IsCopyAssignable<ContainedMutableType> && !TypeTraits::IsConst<ContainedType>>>*/
		NO_DEBUG constexpr Optional& operator=(const Optional& other)
		{
			if (BaseType::m_isValid)
			{
				BaseType::m_element.~MutableType();
			}

			BaseType::m_isValid = other.m_isValid;
			if (other.m_isValid)
			{
				new (Memory::GetAddressOf(BaseType::m_element)) MutableType(other.m_element);
			}
			return *this;
		}
		// template<typename ContainedType = MutableType, typename = EnableIf<TypeTraits::IsMoveConstructible<ContainedType>>>
		NO_DEBUG constexpr Optional(Optional&& other)
		{
			if (other.m_isValid)
			{
				BaseType::m_isValid = true;
				new (Memory::GetAddressOf(BaseType::m_element)) MutableType(Move(other.m_element));
				other.m_isValid = false;
			}
		}
		/*template<
		  typename ContainedType = T,
		  typename ContainedMutableType = MutableType,
		  typename = EnableIf<TypeTraits::IsMoveAssignable<ContainedMutableType> && !TypeTraits::IsConst<ContainedType>>>*/
		NO_DEBUG constexpr Optional& operator=(Optional&& other)
		{
			if (BaseType::m_isValid)
			{
				BaseType::m_element.~T();
			}

			BaseType::m_isValid = other.m_isValid;
			if (other.m_isValid)
			{
				new (Memory::GetAddressOf(BaseType::m_element)) MutableType(Move(other.m_element));
				other.m_isValid = false;
			}
			return *this;
		}
		// template<typename ContainedType = MutableType, typename = EnableIf<TypeTraits::IsCopyConstructible<ContainedType>>>
		NO_DEBUG constexpr Optional(const MutableType& value, const bool isValid = true)
			: BaseType(MutableType(value), isValid)
		{
		}
		/*template<
		  typename ContainedType = T,
		  typename ContainedMutableType = MutableType,
		  typename = EnableIf<TypeTraits::IsCopyConstructible<ContainedMutableType> && !TypeTraits::IsConst<ContainedType>>>*/
		NO_DEBUG constexpr Optional& operator=(const MutableType& value)
		{
			if (BaseType::m_isValid)
			{
				BaseType::m_element.~MutableType();
			}

			BaseType::m_isValid = true;
			new (Memory::GetAddressOf(BaseType::m_element)) MutableType(value);
			return *this;
		}
		// template<typename ContainedType = MutableType, typename = EnableIf<TypeTraits::IsMoveConstructible<ContainedType>>>
		NO_DEBUG constexpr Optional(MutableType&& value, const bool isValid = true)
			: BaseType(Forward<MutableType>(value), isValid)
		{
		}
		/*template<
		  typename ContainedType = T,
		  typename ContainedMutableType = MutableType,
		  typename = EnableIf<TypeTraits::IsMoveConstructible<ContainedMutableType> && !TypeTraits::IsConst<ContainedType>>>*/
		NO_DEBUG constexpr Optional& operator=(MutableType&& value)
		{
			if (BaseType::m_isValid)
			{
				BaseType::m_element.~MutableType();
			}

			BaseType::m_isValid = true;
			new (Memory::GetAddressOf(BaseType::m_element)) MutableType(Forward<T>(value));
			return *this;
		}

		template<typename OtherType, typename = EnableIf<TypeTraits::IsConvertibleTo<OtherType, MutableType>>>
		NO_DEBUG constexpr Optional(OtherType&& value)
			: Optional(MutableType(Forward<OtherType>(value)))
		{
		}

		template<
			typename ContainedType = T,
			typename OtherType,
			typename = EnableIf<TypeTraits::IsConvertibleTo<OtherType, MutableType> && !TypeTraits::IsConst<ContainedType>>>
		NO_DEBUG constexpr Optional& operator=(OtherType&& value)
		{
			Optional::operator=(MutableType(Forward<OtherType>(value)));
			return *this;
		}

		template<typename Type = MutableType, typename... Args, typename = EnableIf<TypeTraits::HasConstructor<Type, Args...>>>
		NO_DEBUG void CreateInPlace(Args&&... args)
		{
			if constexpr (!TypeTraits::IsTriviallyDestructible<MutableType>)
			{
				if (BaseType::m_isValid)
				{
					BaseType::m_element.~MutableType();
				}
			}

			BaseType::m_isValid = true;
			new (Memory::GetAddressOf(BaseType::m_element)) MutableType(Forward<Args>(args)...);
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsValid() const noexcept
		{
			return BaseType::m_isValid;
		}
		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsInvalid() const noexcept
		{
			return !BaseType::m_isValid;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr explicit operator bool() const noexcept
		{
			return BaseType::m_isValid;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const T* operator->() const noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return Memory::GetAddressOf(BaseType::m_element);
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const T& operator*() const noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return BaseType::m_element;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr T* operator->() noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return Memory::GetAddressOf(BaseType::m_element);
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr T& operator*() noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return BaseType::m_element;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr T& Get() noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return BaseType::m_element;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const T& Get() const noexcept LIFETIME_BOUND
		{
			Assert(BaseType::m_isValid);
			return BaseType::m_element;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr T& GetUnsafe() noexcept LIFETIME_BOUND
		{
			return BaseType::m_element;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const T& GetUnsafe() const noexcept LIFETIME_BOUND
		{
			return BaseType::m_element;
		}

		template<typename... Args>
		EnableIf<Serialization::Internal::CanRead<T, Args...>, bool> Serialize(const Serialization::Reader reader, Args&... args);
		template<typename... Args>
		EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool> Serialize(Serialization::Writer writer, Args&... args) const;
	};

	namespace Internal
	{
		HasMemberFunction(IsValid, bool);
	}

	template<typename T>
	struct TRIVIAL_ABI Optional<T, EnableIf<Internal::HasIsValid<T> && TypeTraits::HasConstructor<T>>>
	{
		NO_DEBUG constexpr Optional()
		{
		}
		NO_DEBUG constexpr Optional(InvalidType)
		{
		}

		using MutableType = TypeTraits::WithoutConst<T>;

		Optional(const Optional&) = default;
		Optional& operator=(const Optional&) = default;
		Optional(Optional&&) = default;
		Optional& operator=(Optional&&) = default;
		template<typename Type = T, typename = EnableIf<TypeTraits::IsCopyConstructible<Type>>>
		NO_DEBUG Optional(const T& value)
			: m_value(value)
		{
		}
		template<typename Type = T, typename = EnableIf<TypeTraits::IsCopyAssignable<Type>>>
		NO_DEBUG Optional& operator=(const T& value)
		{
			m_value = value;
			return *this;
		}
		template<typename Type = T, typename = EnableIf<TypeTraits::IsMoveConstructible<Type>>>
		NO_DEBUG Optional(T&& value)
			: m_value(Forward<T>(value))
		{
		}
		template<typename Type = T, typename = EnableIf<TypeTraits::IsMoveAssignable<Type>>>
		NO_DEBUG Optional& operator=(T&& value)
		{
			m_value = Forward<T>(value);
			return *this;
		}
		template<typename Type = T, typename = EnableIf<TypeTraits::IsCopyConstructible<Type>>>
		NO_DEBUG Optional(const T& value, const bool isValid)
			: m_value(isValid ? value : T{})
		{
		}
		template<typename Type = T, typename = EnableIf<TypeTraits::IsMoveConstructible<Type>>>
		NO_DEBUG Optional(T&& value, const bool isValid)
			: m_value(isValid ? Forward<T>(value) : T{})
		{
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsValid() const noexcept
		{
			return m_value.IsValid();
		}
		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsInvalid() const noexcept
		{
			return !m_value.IsValid();
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const T* operator->() const noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return &m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const T& operator*() const noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS T* operator->() noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return &m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS T& operator*() noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS T& Get() noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const T& Get() const noexcept LIFETIME_BOUND
		{
			Assert(IsValid());
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS T& GetUnsafe() noexcept LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const T& GetUnsafe() const noexcept LIFETIME_BOUND
		{
			return m_value;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS explicit operator bool() const noexcept
		{
			return m_value.IsValid();
		}

		template<typename... Args>
		EnableIf<Serialization::Internal::CanRead<T, Args...>, bool> Serialize(const Serialization::Reader reader, Args&... args);
		template<typename... Args>
		EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool> Serialize(Serialization::Writer writer, Args&... args) const;
	private:
		T m_value;
	};

	template<typename T>
	struct TRIVIAL_ABI Optional<T*>
	{
		constexpr Optional() = default;
		NO_DEBUG constexpr Optional(InvalidType)
		{
		}

		constexpr Optional(const Optional&) = default;
		constexpr Optional& operator=(const Optional&) = default;
		constexpr Optional(Optional&&) = default;
		constexpr Optional& operator=(Optional&&) = default;

		template<typename OtherType, typename ThisType = T, typename = EnableIf<TypeTraits::IsBaseOf<ThisType, OtherType>>>
		NO_DEBUG FORCE_INLINE constexpr Optional(const Optional<OtherType*>& pValue)
			: m_pValue(pValue)
		{
		}

		NO_DEBUG FORCE_INLINE constexpr Optional(T& value)
			: m_pValue(Memory::GetAddressOf(value))
		{
		}
		NO_DEBUG FORCE_INLINE constexpr Optional(T* pValue)
			: m_pValue(pValue)
		{
		}

		NO_DEBUG FORCE_INLINE constexpr Optional(T& value, const bool isValid)
			: m_pValue(isValid ? Memory::GetAddressOf(value) : nullptr)
		{
		}
		NO_DEBUG FORCE_INLINE constexpr Optional(T* pValue, const bool isValid)
			: m_pValue(isValid ? pValue : nullptr)
		{
		}

		NO_DEBUG FORCE_INLINE constexpr Optional& operator=(T& value)
		{
			m_pValue = Memory::GetAddressOf(value);
			return *this;
		}
		NO_DEBUG FORCE_INLINE constexpr Optional& operator=(T* value)
		{
			m_pValue = value;
			return *this;
		}
		template<typename OtherType, typename ThisType = T, typename = EnableIf<TypeTraits::IsBaseOf<ThisType, OtherType>>>
		NO_DEBUG FORCE_INLINE constexpr Optional& operator=(const Optional<OtherType*>& pValue)
		{
			m_pValue = pValue;
			return *this;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG operator Optional<const T*>() const
		{
			return Optional<const T*>(m_pValue);
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr bool IsValid() const noexcept
		{
			return m_pValue != nullptr;
		}
		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr bool IsInvalid() const noexcept
		{
			return m_pValue == nullptr;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr explicit operator bool() const noexcept
		{
			return m_pValue != nullptr;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr T* operator->() const noexcept
		{
			Assert(m_pValue != nullptr);
			return m_pValue;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr operator T*() const
		{
			return m_pValue;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr T* Get() const
		{
			return m_pValue;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG T& operator*() const noexcept
		{
			Assert(m_pValue != nullptr);
			return *m_pValue;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr bool operator==(decltype(nullptr)) const
		{
			return m_pValue == nullptr;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS NO_DEBUG constexpr bool operator!=(decltype(nullptr)) const
		{
			return m_pValue != nullptr;
		}

		template<typename... Args>
		EnableIf<Serialization::Internal::CanRead<T, Args...>, bool> Serialize(const Serialization::Reader reader, Args&... args);
		template<typename... Args>
		EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool> Serialize(Serialization::Writer writer, Args&... args) const;
	protected:
		T* m_pValue = nullptr;
	};

	template<typename T>
	struct TRIVIAL_ABI Optional<T&> : public Optional<T*>
	{
		using BaseType = Optional<T*>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};

	namespace Internal
	{
		template<typename Type, Type InvalidValue>
		struct TRIVIAL_ABI OptionalWithSentinel
		{
			OptionalWithSentinel() = default;
			NO_DEBUG OptionalWithSentinel(InvalidType)
			{
			}
			NO_DEBUG constexpr OptionalWithSentinel(const Type value)
				: m_value(value)
			{
			}
			NO_DEBUG OptionalWithSentinel(const Type value, const bool isValid)
				: m_value(isValid ? value : InvalidValue)
			{
			}
			NO_DEBUG constexpr OptionalWithSentinel& operator=(const Type value)
			{
				m_value = value;
				return *this;
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS bool IsInvalid() const noexcept
			{
				return m_value == InvalidValue;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS bool IsValid() const noexcept
			{
				return m_value != InvalidValue;
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS explicit operator bool() const noexcept
			{
				return IsValid();
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const Type& Get() const
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr Type& Get()
			{
				Assert(IsValid());
				return m_value;
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const Type& operator*() const
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr Type& operator*()
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS Type& GetUnsafe() noexcept LIFETIME_BOUND
			{
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const Type& GetUnsafe() const noexcept LIFETIME_BOUND
			{
				return m_value;
			}
		protected:
			NO_DEBUG PURE_STATICS static Type GetInvalidValue() noexcept;
		protected:
			Type m_value = InvalidValue;
		};

		// TODO: Replace with OptionalWithSentinel once we're C++20
		template<typename Type>
		struct TRIVIAL_ABI OptionalWithInvalidValue
		{
			OptionalWithInvalidValue() = default;
			OptionalWithInvalidValue(const OptionalWithInvalidValue&) = default;
			OptionalWithInvalidValue(OptionalWithInvalidValue&&) = default;
			OptionalWithInvalidValue& operator=(const OptionalWithInvalidValue&) = default;
			OptionalWithInvalidValue& operator=(OptionalWithInvalidValue&&) = default;
			NO_DEBUG OptionalWithInvalidValue(InvalidType)
			{
			}
			NO_DEBUG constexpr OptionalWithInvalidValue(const Type value)
				: m_value(value)
			{
			}
			NO_DEBUG OptionalWithInvalidValue(const Type value, const bool isValid)
				: m_value(isValid ? value : GetInvalidValue())
			{
			}
			NO_DEBUG constexpr OptionalWithInvalidValue& operator=(const Type value)
			{
				m_value = value;
				return *this;
			}
			NO_DEBUG constexpr OptionalWithInvalidValue& operator=(const InvalidType)
			{
				m_value = GetInvalidValue();
				return *this;
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS bool IsInvalid() const noexcept
			{
				static const Type invalid = GetInvalidValue();
				return Memory::Compare(this, &invalid, sizeof(Type)) == 0;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS bool IsValid() const noexcept
			{
				return !IsInvalid();
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS explicit operator bool() const noexcept
			{
				return IsValid();
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const Type& Get() const
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr Type& Get()
			{
				Assert(IsValid());
				return m_value;
			}

			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr const Type& operator*() const
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr Type& operator*()
			{
				Assert(IsValid());
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS Type& GetUnsafe() noexcept LIFETIME_BOUND
			{
				return m_value;
			}
			[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS const Type& GetUnsafe() const noexcept LIFETIME_BOUND
			{
				return m_value;
			}
		protected:
			NO_DEBUG PURE_STATICS static Type GetInvalidValue() noexcept;
		protected:
			Type m_value = GetInvalidValue();
		};
	}

#if !PLATFORM_WEB
	template<>
	struct TRIVIAL_ABI Optional<double> : public Internal::OptionalWithInvalidValue<double>
	{
		using BaseType = Internal::OptionalWithInvalidValue<double>;
		using BaseType::BaseType;
		using BaseType::operator=;

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;
	};

	template<>
	struct TRIVIAL_ABI Optional<float> : public Internal::OptionalWithInvalidValue<float>
	{
		using BaseType = Internal::OptionalWithInvalidValue<float>;
		using BaseType::BaseType;
		using BaseType::operator=;

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;
	};
#endif

	template<>
	struct TRIVIAL_ABI Optional<bool>
	{
		constexpr Optional() = default;
		NO_DEBUG constexpr Optional(InvalidType)
		{
		}
		NO_DEBUG constexpr Optional(const bool value)
			: m_value((static_cast<uint8>(Data::Value) * value) | static_cast<uint8>(Data::IsValid))
		{
		}
		NO_DEBUG constexpr Optional(const bool value, const bool isValid)
			: m_value((static_cast<uint8>(Data::Value) * value) | static_cast<uint8>(Data::IsValid) * isValid)
		{
		}
		NO_DEBUG Optional& operator=(const bool value)
		{
			m_value = (static_cast<uint8>(Data::Value) * value) | static_cast<uint8>(Data::IsValid);
			return *this;
		}
		enum class Data : uint8
		{
			Value = 1 << 0,
			IsValid = 1 << 1
		};

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsValid() const noexcept
		{
			return (m_value & static_cast<uint8>(Data::IsValid)) != 0;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool IsInvalid() const noexcept
		{
			return (m_value & static_cast<uint8>(Data::IsValid)) == 0;
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool operator*() const
		{
			return GetValue();
		}

		[[nodiscard]] NO_DEBUG FORCE_INLINE PURE_STATICS constexpr bool GetValue() const
		{
			Assert(IsValid());
			return (m_value & static_cast<uint8>(Data::Value)) != 0;
		}

		bool Serialize(const Serialization::Reader reader);
		bool Serialize(Serialization::Writer writer) const;
	protected:
		uint8 m_value = 0;
	};

	extern template struct Optional<bool>;
	extern template struct Optional<float>;
	extern template struct Optional<double>;
	extern template struct Optional<uint8>;
	extern template struct Optional<uint16>;
	extern template struct Optional<uint32>;
	extern template struct Optional<uint64>;
	extern template struct Optional<int8>;
	extern template struct Optional<int16>;
	extern template struct Optional<int32>;
	extern template struct Optional<int64>;
}
POP_MSVC_WARNINGS
