#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Assert/Assert.h>
#include <Common/TypeTraits/WithoutReference.h>
#include <Common/TypeTraits/ReturnType.h>
#include <Common/TypeTraits/IsFunction.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/Memory/Optional.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/StaticUnreachable.h>
#include <Common/Memory/Move.h>
#include <Common/Memory/Forward.h>

#include "FunctionPointer.h"
#include "ForwardDeclarations/FunctionBase.h"

namespace ngine
{
	namespace TypeTraits
	{
		namespace Internal
		{
			template<typename Type>
			struct IsOptional
			{
				inline static constexpr bool Value = false;
			};

			template<typename Type>
			struct IsOptional<Optional<Type>>
			{
				inline static constexpr bool Value = true;
			};
		}

		template<typename Type>
		inline static constexpr bool IsOptional = Internal::IsOptional<Type>::Value;
	}

	template<typename ReturnType, typename... ArgumentTypes, typename AllocatorType_>
	struct TFunction<ReturnType(ArgumentTypes...), AllocatorType_>
	{
		using FunctionSignatureType = ReturnType (*)(ArgumentTypes...);
		using AllocatorType = AllocatorType_;
		using StorageView = typename AllocatorType::View;
		using ConstStorageView = typename AllocatorType::ConstView;

		constexpr TFunction() = default;

		template<typename Callback, typename = EnableIf<TypeTraits::IsFunction<TypeTraits::WithoutReference<Callback>>>>
		FORCE_INLINE TFunction(Callback&& function) noexcept
		{
			using FunctionValueType = TypeTraits::WithoutReference<Callback>;

			constexpr size functionSize = sizeof(FunctionValueType);
			static_assert(
				functionSize <= AllocatorType::GetTheoreticalCapacity(),
				"Attempt to assign function which surpassed the allocated storage"
			);

			m_storage.Allocate(sizeof(FunctionValueType));
			new (m_storage.GetData()) FunctionValueType(Forward<Callback>(function));

			m_invoke = [](ByteType* pData, ArgumentTypes... args) -> ReturnType
			{
				FunctionValueType* pFunction = reinterpret_cast<FunctionValueType*>(pData);
				Expect(pFunction != nullptr);

				using FunctionReturnType = TypeTraits::ReturnType<FunctionValueType>;
				if constexpr (TypeTraits::IsSame<FunctionReturnType, ReturnType>)
				{
					return (*pFunction)(Forward<ArgumentTypes>(args)...);
				}
				else if constexpr (TypeTraits::IsOptional<ReturnType> && TypeTraits::IsSame<FunctionReturnType, void>)
				{
					(*pFunction)(Forward<ArgumentTypes>(args)...);
					return Invalid;
				}
				else
				{
					static_unreachable("Attempted to assign TFunction with wrong return type!");
				}
			};

			m_destructor = [](ByteType* pData)
			{
				FunctionValueType* pFunction = reinterpret_cast<FunctionValueType*>(pData);
				Expect(pFunction != nullptr);
				pFunction->~FunctionValueType();
			};
		}
		template<typename Callback, typename = EnableIf<TypeTraits::IsFunction<TypeTraits::WithoutReference<Callback>>>>
		FORCE_INLINE TFunction& operator=(Callback&& function) noexcept
		{
			m_destructor(m_storage.GetData());

			using FunctionValueType = TypeTraits::WithoutReference<Callback>;
			static_assert(TypeTraits::IsFunction<FunctionValueType>, "Type was not a function!");

			constexpr size functionSize = sizeof(FunctionValueType);
			static_assert(
				functionSize <= AllocatorType::GetTheoreticalCapacity(),
				"Attempt to assign function which surpassed the allocated storage"
			);

			if (sizeof(FunctionValueType) > m_storage.GetCapacity())
			{
				m_storage.Allocate(sizeof(FunctionValueType));
			}
			new (m_storage.GetData()) FunctionValueType(Forward<Callback>(function));

			m_invoke = [](ByteType* pData, ArgumentTypes... args) -> ReturnType
			{
				FunctionValueType* pFunction = reinterpret_cast<FunctionValueType*>(pData);
				Expect(pFunction != nullptr);

				using FunctionReturnType = TypeTraits::ReturnType<FunctionValueType>;
				if constexpr (TypeTraits::IsSame<FunctionReturnType, ReturnType>)
				{
					return (*pFunction)(Forward<ArgumentTypes>(args)...);
				}
				else if constexpr (TypeTraits::IsOptional<ReturnType> && TypeTraits::IsSame<FunctionReturnType, void>)
				{
					(*pFunction)(Forward<ArgumentTypes>(args)...);
					return Invalid;
				}
				else
				{
					static_unreachable("Attempted to assign TFunction with wrong return type!");
				}
			};

			m_destructor = [](ByteType* pData)
			{
				FunctionValueType* pFunction = reinterpret_cast<FunctionValueType*>(pData);
				Expect(pFunction != nullptr);
				pFunction->~FunctionValueType();
			};

			return *this;
		}

		constexpr TFunction(const FunctionSignatureType function) noexcept
		{
			m_storage.Allocate(sizeof(FunctionSignatureType));
			new (m_storage.GetData()) FunctionSignatureType(function);

			m_invoke = [](ByteType* pData, ArgumentTypes... args) -> ReturnType
			{
				FunctionSignatureType* pFunction = reinterpret_cast<FunctionSignatureType*>(pData);
				Expect(pFunction != nullptr);
				return (*pFunction)(Forward<ArgumentTypes>(args)...);
			};
		}

		template<typename ObjectType>
		FORCE_INLINE TFunction(ObjectType& object, ReturnType (ObjectType::*memberFunction)(ArgumentTypes...)) noexcept
		{
			Bind(object, memberFunction);
		}
		TFunction(TFunction&& other) noexcept
			: m_invoke(other.m_invoke)
			, m_destructor(other.m_destructor)
			, m_storage(Move(other.m_storage))
		{
			other.m_invoke = nullptr;
			other.m_destructor = [](ByteType*)
			{
			};
		}
		TFunction& operator=(TFunction&& other) noexcept
		{
			m_destructor(m_storage.GetData());

			m_invoke = other.m_invoke;
			m_destructor = other.m_destructor;
			m_storage = Move(other.m_storage);

			other.m_invoke = nullptr;
			other.m_destructor = [](ByteType*)
			{
			};

			return *this;
		}
		~TFunction() noexcept
		{
			Expect(m_destructor != nullptr);
			m_destructor(m_storage.GetData());
		}

		template<typename ObjectType>
		FORCE_INLINE void Bind(ObjectType& object, ReturnType (ObjectType::*memberFunction)(ArgumentTypes...)) noexcept
		{
			using MemberFunction = ReturnType (ObjectType::*)(ArgumentTypes...);

			struct FunctionData
			{
				ObjectType& m_object;
				MemberFunction m_function;
			};

			static_assert(
				sizeof(FunctionData) <= AllocatorType::GetTheoreticalCapacity(),
				"Attempt to assign function which surpassed the allocated storage"
			);

			m_invoke = [](ByteType* pData, ArgumentTypes... args) -> ReturnType
			{
				FunctionData& __restrict functionData = *reinterpret_cast<FunctionData*>(pData);
				return (functionData.m_object.*functionData.m_function)(Forward<ArgumentTypes>(args)...);
			};

			m_destructor = [](ByteType*)
			{
			};

			if (sizeof(FunctionData) > m_storage.GetCapacity())
			{
				m_storage.Allocate(sizeof(FunctionData));
			}
			new (m_storage.GetData()) FunctionData{object, memberFunction};
		}

		inline void Unbind() noexcept
		{
			m_destructor(m_storage.GetData());

			m_invoke = nullptr;

			m_destructor = [](ByteType*)
			{
			};
		}

		ReturnType operator()(ArgumentTypes... argumentTypes) const noexcept
		{
			Expect(m_invoke != nullptr);
			return m_invoke(m_storage.GetData(), Forward<ArgumentTypes>(argumentTypes)...);
		}

		[[nodiscard]] bool IsValid() const noexcept
		{
			return m_invoke != nullptr;
		}
	protected:
		TFunction(const TFunction& other) noexcept
			: m_invoke(other.m_invoke)
			, m_destructor(other.m_destructor)
		{
			m_storage.Allocate(other.m_storage.GetCapacity());
		}
		TFunction& operator=(const TFunction& other) noexcept
		{
			m_destructor(m_storage.GetData());

			m_invoke = other.m_invoke;
			m_destructor = other.m_destructor;

			if (other.m_storage.GetCapacity() > m_storage.GetCapacity())
			{
				m_storage.Allocate(other.m_storage.GetCapacity());
			}
			return *this;
		}

		TFunction(TFunction& other) noexcept
			: TFunction(const_cast<const TFunction&>(other))
		{
		}
		TFunction& operator=(TFunction& other) noexcept
		{
			TFunction::operator=(const_cast<const TFunction&>(other));
			return *this;
		}
	protected:
		using FunctionWrapper = ReturnType (*)(ByteType* pData, ArgumentTypes...);
		FunctionWrapper m_invoke = nullptr;
		using CleanupFunction = void (*)(ByteType* pData);
		CleanupFunction m_destructor = [](ByteType*)
		{
		};

		mutable AllocatorType m_storage;
	};
}
