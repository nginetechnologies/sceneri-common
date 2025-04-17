#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Storage/SaltedIdentifierStorage.h>
#include <Common/Storage/IdentifierArray.h>
#include <Common/Memory/Containers/Vector.h>

namespace ngine::Entity::Tests
{
	UNIT_TEST(IdentifierStorage, Initialization)
	{
		using TestIdentifier = TIdentifier<uint32, 16>;
		TSaltedIdentifierStorage<TestIdentifier> storage;

		EXPECT_TRUE(storage.IsEmpty());
		EXPECT_TRUE(!storage.IsFull());
		EXPECT_EQ(storage.GetView().GetSize(), 0u);
	}

	UNIT_TEST(IdentifierStorage, Create)
	{
		using TestIdentifier = TIdentifier<uint32, 16>;
		TSaltedIdentifierStorage<TestIdentifier> storage;

		EXPECT_TRUE(!storage.IsFull());
		EXPECT_EQ(storage.GetView().GetSize(), 0u);
		const TestIdentifier identifier = storage.AcquireIdentifier();
		EXPECT_TRUE(storage.IsIdentifierActive(identifier));
		EXPECT_TRUE(storage.IsIdentifierPotentiallyValid(identifier));
		EXPECT_EQ(identifier.GetFirstValidIndex(), 0u);
		EXPECT_EQ(identifier.GetIndex(), 1u);
		EXPECT_EQ(identifier.GetIndexUseCount(), 0u);
		EXPECT_EQ(identifier, storage.GetActiveIdentifier(identifier));
		EXPECT_EQ(storage.GetView().GetSize(), 1u);

		EXPECT_TRUE(!storage.IsEmpty());
		EXPECT_TRUE(!storage.IsFull());

		storage.ReturnIdentifier(identifier);

		EXPECT_TRUE(storage.IsIdentifierActive(identifier));
		EXPECT_FALSE(storage.IsIdentifierPotentiallyValid(identifier));
		EXPECT_EQ(storage.GetView().GetSize(), 0u);

		EXPECT_TRUE(storage.IsEmpty());
		EXPECT_TRUE(!storage.IsFull());
	}

	UNIT_TEST(IdentifierStorage, UseFullStorage)
	{
		using TestIdentifier = TIdentifier<uint32, 10>;
		TSaltedIdentifierStorage<TestIdentifier> storage;

		Vector<TestIdentifier, uint16> identifiers(Memory::Reserve, TestIdentifier::MaximumCount);
		for (uint16 i = 0; i < TestIdentifier::MaximumCount; ++i)
		{
			const TestIdentifier identifier = storage.AcquireIdentifier();
			EXPECT_TRUE(identifier.IsValid());
			EXPECT_TRUE(storage.IsIdentifierActive(identifier));
			EXPECT_TRUE(storage.IsIdentifierPotentiallyValid(identifier));
			EXPECT_GT(identifier.GetIndex(), 0);
			EXPECT_LE(identifier.GetIndex(), TestIdentifier::MaximumCount);
			EXPECT_LT(identifier.GetFirstValidIndex(), TestIdentifier::MaximumCount);
			identifiers.EmplaceBack(identifier);
		}

		const TestIdentifier lastIdentifier = storage.AcquireIdentifier();
		EXPECT_FALSE(lastIdentifier.IsValid());
		EXPECT_EQ(lastIdentifier.GetIndex(), 0);
		EXPECT_FALSE(storage.IsIdentifierActive(lastIdentifier));

		while (identifiers.HasElements())
		{
			const TestIdentifier identifier = identifiers.PopAndGetBack();
			EXPECT_TRUE(storage.IsIdentifierActive(identifier));
			EXPECT_TRUE(storage.IsIdentifierPotentiallyValid(identifier));

			storage.ReturnIdentifier(identifier);

			EXPECT_TRUE(storage.IsIdentifierActive(identifier));
			EXPECT_FALSE(storage.IsIdentifierPotentiallyValid(identifier));
		}

		EXPECT_TRUE(storage.IsEmpty());
		EXPECT_TRUE(!storage.IsFull());
	}

	UNIT_TEST(IdentifierStorage, IdentifierArray)
	{
		using TestIdentifier = TIdentifier<uint32, 8>;
		TSaltedIdentifierStorage<TestIdentifier> storage;

		struct Element
		{
			TestIdentifier identifier;
		};
		TIdentifierArray<Element, TestIdentifier> identifierArray;
		EXPECT_EQ(identifierArray.GetView().GetSize(), TestIdentifier::MaximumCount);
		EXPECT_EQ(storage.GetValidElementView(identifierArray.GetView()).GetSize(), 0u);

		Vector<TestIdentifier, uint16> identifiers(Memory::Reserve, TestIdentifier::MaximumCount);
		for (uint16 i = 0; i < TestIdentifier::MaximumCount; ++i)
		{
			const TestIdentifier identifier = storage.AcquireIdentifier();
			identifierArray[identifier].identifier = identifier;
			EXPECT_EQ(storage.GetValidElementView(identifierArray.GetView()).GetSize(), i + 1);
			identifiers.EmplaceBack(identifier);
		}

		while (identifiers.HasElements())
		{
			EXPECT_EQ(storage.GetValidElementView(identifierArray.GetView()).GetSize(), identifiers.GetSize());
			const TestIdentifier identifier = identifiers.PopAndGetBack();
			EXPECT_EQ(identifierArray[identifier].identifier, identifier);
			storage.ReturnIdentifier(identifier);
			EXPECT_EQ(storage.GetValidElementView(identifierArray.GetView()).GetSize(), identifiers.GetSize());
		}
	}
}
