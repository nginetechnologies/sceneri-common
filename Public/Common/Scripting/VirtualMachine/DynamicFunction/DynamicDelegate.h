#pragma once

#include <Common/Function/Function.h>
#include <Common/Scripting/VirtualMachine/DynamicFunction/DynamicFunction.h>
#include <Common/Math/Vectorization/PackedInt32.h>
#include <Common/TypeTraits/HasFunctionCallOperator.h>
#include <Common/TypeTraits/IsPrimitive.h>

namespace ngine::Scripting::VM
{
	struct DynamicEvent;

	struct TRIVIAL_ABI DynamicDelegate
	{
		using UserData = Register;

		DynamicDelegate() = default;
		DynamicDelegate(const DynamicDelegate&) = default;
		DynamicDelegate& operator=(const DynamicDelegate&) = default;
		DynamicDelegate(DynamicDelegate&&) = default;
		DynamicDelegate& operator=(DynamicDelegate&&) = default;

		DynamicDelegate(const UserData userData, const DynamicFunction callback)
			: m_userData(userData)
			, m_callback(callback)
		{
		}

		template<auto Callback, typename ObjectType>
		[[nodiscard]] static DynamicDelegate Make(ObjectType& object)
		{
			void* pAddress = Memory::GetAddressOf(object);
			Register userData;
			Memory::Set(&userData, 0, sizeof(Register));
			new (&userData) void*(pAddress);

			// Store the identifier (same as the object in this case) at the end
			ByteType* pIdentifierAddress = reinterpret_cast<ByteType*>(&userData) + (sizeof(UserData) - sizeof(ObjectType*));
			new (pIdentifierAddress) void*(pAddress);

			return DynamicDelegate{userData, DynamicFunction::Make<Callback>()};
		}
		template<typename Identifier, typename CallbackObject, typename = EnableIf<TypeTraits::HasFunctionCallOperator<CallbackObject>>>
		[[nodiscard]] static DynamicDelegate Make(const Identifier identifier, CallbackObject&& callback)
		{

			using StoredTuple = Tuple<CallbackObject, Identifier>;
			static_assert(sizeof(StoredTuple) <= sizeof(UserData), "Callable object was too large!");

			Register userData;
			Memory::Set(&userData, 0, sizeof(Register));
			new (&userData) CallbackObject(Forward<CallbackObject>(callback));

			// Store the identifier at the end
			ByteType* pIdentifierAddress = reinterpret_cast<ByteType*>(&userData) + (sizeof(UserData) - sizeof(Identifier));
			new (pIdentifierAddress) Identifier(identifier);

			return DynamicDelegate{userData, DynamicFunction::Make<&CallbackObject::operator()>()};
		}

		[[nodiscard]] FORCE_INLINE static bool CompareRegisters(const Register a, const Register b)
		{
#if USE_WASM_SIMD128
			return (wasm_i32x4_bitmask(wasm_f32x4_eq(a, b)) & 0xF) == 0xF;
#elif USE_AVX512
#error "TODO"
#elif USE_AVX
			using PackedType = Math::Vectorization::Packed<int32, 8>;
			return ((unsigned int)(PackedType{a} == PackedType{b}).GetMask() & 0xFF) == 0xFF;
#elif USE_SSE
			using PackedType = Math::Vectorization::Packed<int32, 4>;
			return ((PackedType{a} == PackedType{b}).GetMask() & 0xF) == 0xF;
#elif USE_NEON
			const Register equality = vceqq_u32(a, b);
			const uint32 mask = vminvq_u32(equality);
			return mask == 0xFFFFFFFF;
#else
			return a == b;
#endif
		}

		[[nodiscard]] bool operator==(const DynamicDelegate other) const
		{
			return CompareRegisters(m_userData, other.m_userData) && m_callback == other.m_callback;
		}
		[[nodiscard]] bool operator!=(const DynamicDelegate other) const
		{
			return !operator==(other);
		}

		//! Checks if the delegate is bound to the specified object
		[[nodiscard]] bool IsBoundToObject(const UserData object) const
		{
			return CompareRegisters(m_userData, object);
		}
		template<typename IdentifierObjectType>
		[[nodiscard]] bool IsBoundToObject(const IdentifierObjectType& object) const
		{
			if constexpr (TypeTraits::IsPrimitive<IdentifierObjectType>)
			{
				// Offset the user data address as the identifier is always stored last
				const ByteType* pUserData = reinterpret_cast<const ByteType*>(&m_userData);
				pUserData += sizeof(UserData) - sizeof(void*);
				return *reinterpret_cast<const IdentifierObjectType*>(pUserData) == object;
			}
			else
			{
				// Offset the user data address as the identifier is always stored last
				const ByteType* pAddress = reinterpret_cast<const ByteType*>(&m_userData);
				pAddress += sizeof(UserData) - sizeof(void*);
				const IdentifierObjectType* pIdentifierAddress = *reinterpret_cast<IdentifierObjectType* const *>(pAddress);
				return pIdentifierAddress == Memory::GetAddressOf(object);
			}
		}

		template<typename... Args>
		FORCE_INLINE ReturnValue operator()(Args&&... args) const
		{
			Registers registers;
			static constexpr size UserDataArgumentOffset = 1;
			registers.PushArguments<UserDataArgumentOffset, Args...>(Forward<Args>(args)...);
			ReturnValue returnValue = m_callback(m_userData, registers[1], registers[2], registers[3], registers[4], registers[5]);
			registers.PopArguments<UserDataArgumentOffset, Args...>();
			return returnValue;
		}
		template<typename... Args>
		FORCE_INLINE ReturnValue operator()(const Register R1, const Register R2, const Register R3, const Register R4, const Register R5) const
		{
			return m_callback(m_userData, R1, R2, R3, R4, R5);
		}
	protected:
		friend DynamicEvent;

		UserData m_userData;
		DynamicFunction m_callback;
	};
}

namespace ngine::Reflection
{
	template<>
	struct ReflectedType<Scripting::VM::DynamicDelegate>
	{
		static constexpr auto Type =
			Reflection::Reflect<Scripting::VM::DynamicDelegate>("18c91a1c-7739-4a2a-868f-9cc321e430cb"_guid, MAKE_UNICODE_LITERAL("Delegate"));
	};
}
