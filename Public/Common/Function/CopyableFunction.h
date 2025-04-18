#pragma once

#include "Function.h"
#include "ForwardDeclarations/CopyableFunction.h"

namespace ngine
{
	template<typename ReturnType, typename... ArgumentTypes, size StorageSizeBytes>
	struct CopyableFunction<ReturnType(ArgumentTypes...), StorageSizeBytes> : public Function<ReturnType(ArgumentTypes...), StorageSizeBytes>
	{
		using BaseType = Function<ReturnType(ArgumentTypes...), StorageSizeBytes>;
		using AllocatorType = typename BaseType::AllocatorType;
		using StorageView = typename BaseType::StorageView;
		using ConstStorageView = typename BaseType::ConstStorageView;

		CopyableFunction() = default;

		CopyableFunction(const CopyableFunction& other) noexcept
			: BaseType(static_cast<const BaseType&>(other))
			, m_copyFunction(other.m_copyFunction)
		{
			m_copyFunction(BaseType::m_storage, other.BaseType::m_storage.GetData());
		}
		CopyableFunction(CopyableFunction& other)
			: CopyableFunction(static_cast<const CopyableFunction&>(other))
		{
		}
		CopyableFunction& operator=(const CopyableFunction& other) noexcept
		{
			BaseType::operator=(static_cast<const BaseType&>(other));
			m_copyFunction = other.m_copyFunction;
			m_copyFunction(BaseType::m_storage, other.BaseType::m_storage.GetData());
			return *this;
		}
		CopyableFunction& operator=(CopyableFunction& other)
		{
			return CopyableFunction::operator=(static_cast<const CopyableFunction&>(other));
		}

		CopyableFunction(CopyableFunction&& other) noexcept
			: BaseType(Forward<BaseType>(other))
			, m_copyFunction(other.m_copyFunction)
		{
		}

		CopyableFunction& operator=(CopyableFunction&& other) noexcept
		{
			BaseType::operator=(Forward<BaseType>(other));
			m_copyFunction = other.m_copyFunction;
			return *this;
		}

		template<typename Function>
		CopyableFunction(Function&& function) noexcept
			: BaseType(Forward<Function>(function))
		{
			using FunctionValueType = TypeTraits::WithoutReference<Function>;
			static_assert(
				sizeof(FunctionValueType) <= BaseType::AllocatorType::GetTheoreticalCapacity(),
				"Attempt to assign function which surpassed the allocated storage"
			);

			m_copyFunction = [](AllocatorType& source, const ByteType* pSource)
			{
				if (source.GetCapacity() != sizeof(FunctionValueType))
				{
					source.Allocate(sizeof(FunctionValueType));
				}

				const FunctionValueType* pSourceFunction = reinterpret_cast<const FunctionValueType*>(pSource);
				new (source.GetData()) FunctionValueType(*pSourceFunction);
			};
		}

		template<typename ObjectType>
		inline CopyableFunction(ObjectType& object, ReturnType (ObjectType::*memberFunction)(ArgumentTypes...)) noexcept
		{
			Bind(object, memberFunction);
		}

		template<typename Function>
		inline CopyableFunction& operator=(Function&& function) noexcept
		{
			BaseType::operator=(Forward<Function>(function));
			using FunctionValueType = TypeTraits::WithoutReference<Function>;
			static_assert(
				sizeof(FunctionValueType) <= BaseType::AllocatorType::GetTheoreticalCapacity(),
				"Attempt to assign function which surpassed the allocated storage"
			);

			m_copyFunction = [](AllocatorType& source, const ByteType* pSource)
			{
				if (source.GetCapacity() != sizeof(FunctionValueType))
				{
					source.Allocate(sizeof(FunctionValueType));
				}

				const FunctionValueType* pSourceFunction = reinterpret_cast<const FunctionValueType*>(pSource);
				new (source.GetData()) FunctionValueType(*pSourceFunction);
			};

			return *this;
		}

		template<typename ObjectType>
		inline void Bind(ObjectType& object, ReturnType (ObjectType::*memberFunction)(ArgumentTypes...)) noexcept
		{
			BaseType::Bind(object, memberFunction);

			m_copyFunction = [](AllocatorType&, const ByteType*)
			{
			};
		}
	protected:
		using CopyFunction = void (*)(AllocatorType& target, const ByteType* source);
		CopyFunction m_copyFunction = [](AllocatorType&, const ByteType*)
		{
		};
	};

	template<typename ReturnType, typename... ArgumentTypes>
	struct CopyableFunction<ReturnType(ArgumentTypes...), 0> : public Function<ReturnType(ArgumentTypes...), 0>
	{
		using BaseType = Function<ReturnType(ArgumentTypes...), 0>;
		using BaseType::BaseType;
	};
}
