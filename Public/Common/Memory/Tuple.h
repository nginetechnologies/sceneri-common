#pragma once

#include <Common/TypeTraits/WithoutReference.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Platform/NoUniqueAddress.h>
#include <Common/Memory/Forward.h>
#include <Common/Memory/AnyView.h>
#include <Common/Memory/Containers/ContainerCommon.h>
#include <Common/Memory/ForwardDeclarations/Tuple.h>
#include <Common/Memory/CallbackResult.h>
#include <Common/TypeTraits/Any.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/IntegerSequence.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename... Elements>
	struct TupleElements;

	template<>
	struct TRIVIAL_ABI TupleElements<>
	{
		using ElementType = void;
		inline static constexpr bool HasValue = false;
		inline static constexpr bool HasNext = false;
	};

	template<typename ElementType_>
	struct TRIVIAL_ABI TupleElements<ElementType_>
	{
		using ElementType = ElementType_;

		inline static constexpr bool HasValue = true;

		template<size Index>
		struct GetElementTypeAt
		{
			using Type = ElementType;
		};

		template<typename Type, size IndexOffset = 0>
		struct GetFirstIndexOf
		{
			static_assert(TypeTraits::IsSame<Type, ElementType>, "Invalid type");
			inline static constexpr size Value = IndexOffset;
		};

		template<typename Type>
		struct ContainsType
		{
			inline static constexpr bool Value = TypeTraits::IsSame<Type, ElementType>;
		};

		template<size Index>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr auto& Get() LIFETIME_BOUND
		{
			if constexpr (Index == 0)
			{
				return m_value;
			}
			else
			{
				static_unreachable("Invalid index!");
			}
		}

		template<size Index>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const auto& Get() const LIFETIME_BOUND
		{
			if constexpr (Index == 0)
			{
				return m_value;
			}
			else
			{
				static_unreachable("Invalid index!");
			}
		}

		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&& function) const
		{
			if (auto result = function(m_value))
			{
				return result;
			}
			return {};
		}
		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&& function)
		{
			if (auto result = function(m_value))
			{
				return result;
			}
			return {};
		}

		ElementType m_value;
	};

	template<typename ElementType_, typename... ArgumentTypes>
	struct TRIVIAL_ABI TupleElements<ElementType_, ArgumentTypes...>
	{
		using ElementType = ElementType_;
		using NextElements = TupleElements<ArgumentTypes...>;

		template<size Index>
		struct GetElementTypeAt
		{
			using Type = TypeTraits::Select<Index == 0, ElementType, typename NextElements::template GetElementTypeAt<Index - 1>::Type>;
		};

		template<typename Type, size IndexOffset = 0>
		struct GetFirstIndexOf
		{
			inline static constexpr size Value = NextElements::template GetFirstIndexOf<Type, IndexOffset + 1>::Value;
		};

		template<size IndexOffset>
		struct GetFirstIndexOf<ElementType, IndexOffset>
		{
			inline static constexpr size Value = IndexOffset;
		};

		template<typename Type>
		struct ContainsType
		{
			inline static constexpr bool Value = TypeTraits::IsSame<Type, ElementType> || NextElements::template ContainsType<Type>::Value;
		};

		inline static constexpr bool HasValue = true;
		inline static constexpr bool HasNext = true;

		constexpr TupleElements() = default;

		constexpr TupleElements(TupleElements&& other) = default;
		constexpr TupleElements& operator=(TupleElements&& other) = default;
		constexpr TupleElements(const TupleElements& other) = default;
		constexpr TupleElements& operator=(const TupleElements& other) = default;

		FORCE_INLINE constexpr TupleElements(ElementType element, ArgumentTypes... others)
			: m_value{Forward<ElementType>(element)}
			, m_nextElements{Forward<ArgumentTypes>(others)...}
		{
		}

		template<size Index>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr auto& Get() LIFETIME_BOUND
		{
			if constexpr (Index == 0)
			{
				return m_value;
			}
			else if constexpr (HasNext)
			{
				return m_nextElements.template Get<Index - 1>();
			}
			else
			{
				static_unreachable("Invalid index!");
			}
		}

		template<size Index>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const auto& Get() const LIFETIME_BOUND
		{
			if constexpr (Index == 0)
			{
				return m_value;
			}
			else if constexpr (HasNext)
			{
				return m_nextElements.template Get<Index - 1>();
			}
			else
			{
				static_unreachable("Invalid index!");
			}
		}

		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&& function) const
		{
			if (auto result = function(m_value))
			{
				return result;
			}
			else if constexpr (HasNext)
			{
				return m_nextElements.template FindIf<ReturnType>(Forward<FunctionType>(function));
			}
			else
			{
				return {};
			}
		}
		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&& function)
		{
			if (auto result = function(m_value))
			{
				return result;
			}
			else if constexpr (HasNext)
			{
				return m_nextElements.template FindIf<ReturnType>(Forward<FunctionType>(function));
			}
			else
			{
				return {};
			}
		}

		ElementType m_value;
		NO_UNIQUE_ADDRESS NextElements m_nextElements;
	};

	template<typename... ArgumentTypes>
	struct TRIVIAL_ABI Tuple
	{
		using Elements = TupleElements<ArgumentTypes...>;
		using SizeType = Memory::UnsignedNumericSize<sizeof...(ArgumentTypes)>;
		template<SizeType Index>
		using ElementType = typename Elements::template GetElementTypeAt<Index>::Type;
		inline static constexpr SizeType ElementCount = sizeof...(ArgumentTypes);
		static_assert(ElementCount > 0);
		inline static constexpr bool HasElements = true;
		inline static constexpr bool IsEmpty = false;

		using IndexSequence = TypeTraits::MakeIntegerSequence<SizeType, ElementCount>;

		template<typename... Types>
		using Amend = Tuple<ArgumentTypes..., Types...>;

		template<SizeType i>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr auto& Get() noexcept LIFETIME_BOUND
		{
			static_assert(i < ElementCount);
			return m_elements.template Get<i>();
		}

		template<SizeType i>
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const auto& Get() const noexcept LIFETIME_BOUND
		{
			static_assert(i < ElementCount);
			return m_elements.template Get<i>();
		}

		template<typename Type>
		inline static constexpr SizeType FirstIndexOf = (SizeType)Elements::template GetFirstIndexOf<Type>::Value;
		template<typename Type>
		inline static constexpr bool ContainsType = Elements::template ContainsType<Type>::Value;

		template<typename Type>
		[[nodiscard]] constexpr PURE_STATICS Type& GetFirstOf() noexcept LIFETIME_BOUND
		{
			return Get<FirstIndexOf<Type>>();
		}

		template<typename Type>
		[[nodiscard]] constexpr PURE_STATICS const Type& GetFirstOf() const noexcept LIFETIME_BOUND
		{
			return Get<FirstIndexOf<Type>>();
		}

		constexpr Tuple() noexcept = default;

		FORCE_INLINE Tuple(const Memory::UninitializedType) noexcept
		{
		}

		template<typename... ConstructorArgs>
		FORCE_INLINE constexpr Tuple(ConstructorArgs&&... arguments) noexcept
			: m_elements{Forward<ConstructorArgs>(arguments)...}
		{
		}

		FORCE_INLINE constexpr Tuple(Tuple&& other) noexcept = default;
		FORCE_INLINE constexpr Tuple& operator=(Tuple&& other) noexcept = default;
		FORCE_INLINE constexpr Tuple(const Tuple& other) noexcept = default;
		FORCE_INLINE constexpr Tuple& operator=(const Tuple& other) noexcept = default;

		template<typename FunctionType, SizeType... Indices>
		constexpr void ForEach(FunctionType&& function, TypeTraits::IntegerSequence<SizeType, Indices...>) const
		{
			(..., function(Get<Indices>()));
		}
		template<typename FunctionType>
		constexpr void ForEach(FunctionType&& function) const
		{
			ForEach(Forward<FunctionType>(function), IndexSequence{});
		}

		template<typename FunctionType, SizeType... Indices>
		constexpr void ForEach(FunctionType&& function, TypeTraits::IntegerSequence<SizeType, Indices...>)
		{
			(..., function(Get<Indices>()));
		}
		template<typename FunctionType>
		constexpr void ForEach(FunctionType&& function)
		{
			ForEach(Forward<FunctionType>(function), IndexSequence{});
		}

		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf([[maybe_unused]] FunctionType&& function) const
		{
			return m_elements.template FindIf<ReturnType>(Forward<FunctionType>(function));
		}
		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf([[maybe_unused]] FunctionType&& function)
		{
			return m_elements.template FindIf<ReturnType>(Forward<FunctionType>(function));
		}

		template<SizeType Index>
		using Type = decltype(TypeTraits::DeclareValue<Elements&>().template Get<Index>());
	private:
		Elements m_elements;
	};

	template<>
	struct TRIVIAL_ABI Tuple<>
	{
		using Elements = TupleElements<>;
		using SizeType = uint8;
		inline static constexpr SizeType ElementCount = 0;
		inline static constexpr bool HasElements = false;
		inline static constexpr bool IsEmpty = true;

		using IndexSequence = TypeTraits::MakeIntegerSequence<SizeType, ElementCount>;

		template<typename... Types>
		using Amend = Tuple<Types...>;

		template<typename Type>
		inline static constexpr bool ContainsType = false;

		constexpr Tuple() noexcept = default;

		FORCE_INLINE Tuple(const Memory::UninitializedType) noexcept
		{
		}

		FORCE_INLINE constexpr Tuple(Tuple&& other) noexcept = default;
		FORCE_INLINE constexpr Tuple& operator=(Tuple&& other) noexcept = default;
		FORCE_INLINE constexpr Tuple(const Tuple& other) noexcept = default;
		FORCE_INLINE constexpr Tuple& operator=(const Tuple& other) noexcept = default;

		template<SizeType i>
		[[nodiscard]] constexpr auto& Get() noexcept
		{
			static_unreachable();
		}
		template<SizeType i>
		[[nodiscard]] constexpr const auto& Get() const noexcept
		{
			static_unreachable();
		}

		template<typename FunctionType>
		constexpr void ForEach(FunctionType&&) const
		{
		}
		template<typename FunctionType>
		constexpr void ForEach(FunctionType&&)
		{
		}

		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&&) const
		{
			return {};
		}
		template<typename ReturnType, typename FunctionType>
		constexpr ReturnType FindIf(FunctionType&&)
		{
			return {};
		}
	};

	template<typename... Ts>
	Tuple(const Ts&...) -> Tuple<Ts...>;
	template<typename... Ts>
	Tuple(Ts&&...) -> Tuple<Ts...>;
}
