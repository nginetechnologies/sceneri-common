#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/HashTable.h>
#include <Common/Memory/Containers/UnorderedMap.h>
#include <Common/Memory/Containers/UnorderedSet.h>
#include <Common/Memory/Allocators/DynamicAllocator.h>

PUSH_MSVC_WARNINGS_TO_LEVEL(2)
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-builtins")
DISABLE_CLANG_WARNING("-Wdeprecated")

#include <Common/3rdparty/absl/hash/hash.h>

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

namespace ngine::Tests
{
	namespace Internal
	{
		template<typename Key>
		struct HashTableDetail
		{
			[[nodiscard]] static constexpr const Key& GetKey(const Key& key)
			{
				return key;
			}
		};
	}

	template<typename T, typename HashType = absl::Hash<T>>
	using HashTable = THashTable<
		T,
		T,
		Internal::HashTableDetail<T>,
		HashType,
		Memory::Internal::DefaultEqualityCheck<T>,
		Memory::DynamicAllocator<ByteType, uint32, uint32>>;

	UNIT_TEST(HashTable, DefaultConstruct)
	{
		HashTable<int> hashTable;
		EXPECT_TRUE(hashTable.IsEmpty());
		EXPECT_FALSE(hashTable.HasElements());
		EXPECT_EQ(hashTable.GetSize(), 0u);
		EXPECT_EQ(hashTable.begin(), hashTable.end());
		EXPECT_EQ(hashTable.Find(0), hashTable.end());
		EXPECT_FALSE(hashTable.Contains(0));
	}

	UNIT_TEST(HashTable, Basic)
	{
		HashTable<int> hashTable;
		EXPECT_TRUE(hashTable.GetBucketCount() == 0);
		hashTable.Reserve(10);
		EXPECT_TRUE(hashTable.GetBucketCount() == 16);

		// Check system limits
		EXPECT_TRUE(hashTable.GetMaximumBucketCount() == 0x80000000);
		EXPECT_TRUE(hashTable.GetTheoreticalCapacity() == uint64(0x80000000) * 7 / 8);

		// Insert some entries
		EXPECT_TRUE(*hashTable.Insert(1) == 1);
		EXPECT_TRUE(hashTable.Insert(3) != hashTable.end());
		EXPECT_TRUE(hashTable.Insert(3) == hashTable.end());
		EXPECT_TRUE(hashTable.GetSize() == 2);
		EXPECT_TRUE(*hashTable.Find(1) == 1);
		EXPECT_TRUE(*hashTable.Find(3) == 3);
		EXPECT_TRUE(hashTable.Find(5) == hashTable.end());

		// Validate all elements are visited by a visitor
		int count = 0;
		Array<bool, 10, uint32, uint32> visited{Memory::InitializeAll, false};
		for (HashTable<int>::const_iterator i = hashTable.begin(); i != hashTable.end(); ++i)
		{
			EXPECT_FALSE(visited[*i]);
			visited[*i] = true;
			++count;
		}
		EXPECT_TRUE(count == 2);
		EXPECT_TRUE(visited[1]);
		EXPECT_TRUE(visited[3]);
		for (HashTable<int>::iterator i = hashTable.begin(); i != hashTable.end(); ++i)
		{
			visited[*i] = false;
			--count;
		}
		EXPECT_TRUE(count == 0);
		EXPECT_TRUE(!visited[1]);
		EXPECT_TRUE(!visited[3]);

		// Copy the hashTable
		HashTable<int> hashTable2;
		hashTable2 = hashTable;
		EXPECT_TRUE(*hashTable2.Find(1) == 1);
		EXPECT_TRUE(*hashTable2.Find(3) == 3);
		EXPECT_TRUE(hashTable2.Find(5) == hashTable2.end());

		// Swap
		HashTable<int> hashTable3;
		hashTable3.Swap(hashTable);
		EXPECT_TRUE(*hashTable3.Find(1) == 1);
		EXPECT_TRUE(*hashTable3.Find(3) == 3);
		EXPECT_TRUE(hashTable3.Find(5) == hashTable3.end());
		EXPECT_TRUE(hashTable.IsEmpty());

		// Move construct
		HashTable<int> hashTable4(Move(hashTable3));
		EXPECT_TRUE(*hashTable4.Find(1) == 1);
		EXPECT_TRUE(*hashTable4.Find(3) == 3);
		EXPECT_TRUE(hashTable4.Find(5) == hashTable4.end());
		EXPECT_TRUE(hashTable3.IsEmpty());

		// Move assign
		HashTable<int> hashTable5;
		hashTable5.Insert(999);
		EXPECT_TRUE(*hashTable5.Find(999) == 999);
		hashTable5 = Move(hashTable4);
		EXPECT_TRUE(hashTable5.Find(999) == hashTable5.end());
		EXPECT_TRUE(*hashTable5.Find(1) == 1);
		EXPECT_TRUE(*hashTable5.Find(3) == 3);
		EXPECT_TRUE(hashTable4.IsEmpty());
	}

	// TODO: Remove when we fix the test
#if ENABLE_HASH_TABLE_UNORDERED_SET || ENABLE_HASH_TABLE_UNORDERED_MAP
	UNIT_TEST(HashTable, Grow)
	{
		HashTable<int> set;
		for (int i = 0; i < 10000; ++i)
			EXPECT_TRUE(set.Insert(i) != set.end());

		EXPECT_TRUE(set.GetSize() == 10000);

		for (int i = 0; i < 10000; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(10001) == set.end());

		for (int i = 0; i < 5000; ++i)
		{
			set.Remove(set.Find(i));
			EXPECT_FALSE(set.Contains(i));
		}

		EXPECT_TRUE(set.GetSize() == 5000);

		for (int i = 0; i < 5000; ++i)
			EXPECT_TRUE(set.Find(i) == set.end());

		for (int i = 5000; i < 10000; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(10001) == set.end());

		for (int i = 0; i < 5000; ++i)
			EXPECT_TRUE(set.Insert(i) != set.end());

		EXPECT_TRUE(set.Insert(0) == set.end());

		EXPECT_TRUE(set.GetSize() == 10000);

		for (int i = 0; i < 10000; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(10001) == set.end());
	}
#endif

	UNIT_TEST(HashTable, HashCollision)
	{
		// A hash function that's guaranteed to collide
		class MyBadHash
		{
		public:
			size_t operator()(int) const
			{
				return 0;
			}
		};

		HashTable<int, MyBadHash> set;
		for (int i = 0; i < 10; ++i)
			EXPECT_TRUE(set.Insert(i) != set.end());

		EXPECT_TRUE(set.GetSize() == 10);

		for (int i = 0; i < 10; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(11) == set.end());

		for (int i = 0; i < 5; ++i)
		{
			set.Remove(set.Find(i));
			EXPECT_FALSE(set.Contains(i));
		}

		EXPECT_TRUE(set.GetSize() == 5);

		for (int i = 0; i < 5; ++i)
			EXPECT_TRUE(set.Find(i) == set.end());

		for (int i = 5; i < 10; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(11) == set.end());

		for (int i = 0; i < 5; ++i)
			EXPECT_TRUE(set.Insert(i) != set.end());

		EXPECT_TRUE(set.Insert(0) == set.end());

		EXPECT_TRUE(set.GetSize() == 10);

		for (int i = 0; i < 10; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		EXPECT_TRUE(set.Find(11) == set.end());
	}

	UNIT_TEST(HashTable, AddRemove)
	{
		HashTable<int> set;
		constexpr int cBucketCount = 64;
		set.Reserve(int(set.GetMaximumLoadFactor() * cBucketCount));
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);

		// Repeatedly add and remove elements to see if the set cleans up tombstones
		constexpr int cNumElements = 64 * 6 /
		                             8; // We make sure that the map is max 6/8 full to ensure that we never grow the map but rehash it instead
		int add_counter = 0;
		int remove_counter = 0;
		for (int i = 0; i < 100; ++i)
		{
			for (int j = 0; j < cNumElements; ++j)
			{
				EXPECT_TRUE(set.Find(add_counter) == set.end());
				EXPECT_TRUE(set.Insert(add_counter) != set.end());
				EXPECT_TRUE(set.Find(add_counter) != set.end());
				++add_counter;
			}

			EXPECT_TRUE(set.GetSize() == cNumElements);

			for (int j = 0; j < cNumElements; ++j)
			{
				EXPECT_TRUE(set.Find(remove_counter) != set.end());
				set.Remove(set.Find(remove_counter));
				EXPECT_TRUE(set.Find(remove_counter) == set.end());
				EXPECT_TRUE(set.Find(remove_counter) == set.end());
				++remove_counter;
			}

			EXPECT_TRUE(set.GetSize() == 0);
			EXPECT_TRUE(set.IsEmpty());
		}

		// Test that adding and removing didn't resize the set
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);
	}

	UNIT_TEST(HashTable, ManyTombStones)
	{
		// A hash function that makes sure that consecutive ints end up in consecutive buckets starting at bucket 63
		class MyBadHash
		{
		public:
			size_t operator()(int inValue) const
			{
				return (inValue + 63) << 7;
			}
		};

		HashTable<int, MyBadHash> set;
		constexpr int cBucketCount = 64;
		set.Reserve(int(set.GetMaximumLoadFactor() * cBucketCount));
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);

		// Fill 32 buckets
		int add_counter = 0;
		for (int i = 0; i < 32; ++i)
			EXPECT_TRUE(set.Insert(add_counter++) != set.end());

		// Since we control the hash, we know in which order we'll visit the elements
		// The first element was inserted in bucket 63, so we start at 1
		int expected = 1;
		for (int i : set)
		{
			EXPECT_TRUE(i == expected);
			expected = (expected + 1) & 31;
		}
		expected = 1;
		for (int i : set)
		{
			EXPECT_TRUE(i == expected);
			expected = (expected + 1) & 31;
		}

		// Remove a bucket in the middle with so that the number of occupied slots
		// surrounding the bucket exceed 16 to force creating a tombstone,
		// then add one at the end
		int remove_counter = 16;
		for (int i = 0; i < 100; ++i)
		{
			EXPECT_TRUE(set.Find(remove_counter) != set.end());
			set.Remove(set.Find(remove_counter));
			EXPECT_FALSE(set.Contains(remove_counter));
			EXPECT_TRUE(set.Find(remove_counter) == set.end());

			EXPECT_TRUE(set.Find(add_counter) == set.end());
			EXPECT_TRUE(set.Insert(add_counter) != set.end());
			EXPECT_TRUE(set.Find(add_counter) != set.end());

			++add_counter;
			++remove_counter;
		}

		// Check that the elements we inserted are still there
		EXPECT_TRUE(set.GetSize() == 32);
		for (int i = 0; i < 16; ++i)
			EXPECT_TRUE(*set.Find(i) == i);
		for (int i = 0; i < 16; ++i)
			EXPECT_TRUE(*set.Find(add_counter - 1 - i) == add_counter - 1 - i);

		// Test that adding and removing didn't resize the set
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);
	}

	static bool sReversedHash = false;

	UNIT_TEST(HashTable, Rehash)
	{
		// A hash function for which we can switch the hashing algorithm
		class MyBadHash
		{
		public:
			size_t operator()(int inValue) const
			{
				return (sReversedHash ? 127 - inValue : inValue) << 7;
			}
		};

		using Set = HashTable<int, MyBadHash>;
		Set set;
		constexpr int cBucketCount = 128;
		set.Reserve(int(set.GetMaximumLoadFactor() * cBucketCount));
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);

		// Fill buckets
		sReversedHash = false;
		constexpr int cNumElements = 96;
		for (int i = 0; i < cNumElements; ++i)
		{
			EXPECT_TRUE(set.Insert(i) != set.end());
		}

		// Check that we get the elements in the expected order
		int expected = 0;
		for (int i : set)
			EXPECT_TRUE(i == expected++);

		// Change the hashing algorithm so that a rehash is forced to move elements.
		// The test is designed in such a way that it will both need to move elements to IsEmpty slots
		// and to move elements to slots that currently already have another element.
		sReversedHash = true;
		set.Rehash();

		// Check that all elements are still there
		for (int i = 0; i < cNumElements; ++i)
			EXPECT_TRUE(*set.Find(i) == i);

		// The hash went from filling buckets 0 .. 95 with values 0 .. 95 to bucket 127 .. 31 with values 0 .. 95
		// However, we don't move elements if they still fall within the same batch, this means that the first 8
		// elements didn't move
		decltype(set)::const_iterator it = set.begin();
		for (int i = 0; i < 8; ++i, ++it)
			EXPECT_TRUE(*it == i);

		// The rest will have been reversed
		for (int i = 95; i > 7; --i, ++it)
			EXPECT_TRUE(*it == i);

		// Test that adding and removing didn't resize the set
		EXPECT_TRUE(set.GetBucketCount() == cBucketCount);
	}

	UNIT_TEST(HashTable, BasicEmplace)
	{
		HashTable<int> hashTable;
		hashTable.Emplace(1337);
		EXPECT_EQ(hashTable.GetSize(), 1);
		EXPECT_TRUE(hashTable.HasElements());
		EXPECT_NE(hashTable.begin(), hashTable.end());
		EXPECT_TRUE(hashTable.Contains(1337));
		EXPECT_FALSE(hashTable.Contains(0));
		EXPECT_FALSE(hashTable.Contains(1));
		EXPECT_FALSE(hashTable.Contains(1336));
		EXPECT_FALSE(hashTable.Contains(1338));
		EXPECT_FALSE(hashTable.Contains(9001));
		EXPECT_EQ(hashTable.Find(1337), hashTable.begin());

		hashTable.Emplace(9001);
		EXPECT_EQ(hashTable.GetSize(), 2);
		EXPECT_TRUE(hashTable.HasElements());
		EXPECT_NE(hashTable.begin(), hashTable.end());
		EXPECT_TRUE(hashTable.Contains(1337));
		EXPECT_TRUE(hashTable.Contains(9001));
		EXPECT_FALSE(hashTable.Contains(0));
		EXPECT_FALSE(hashTable.Contains(1));
		EXPECT_FALSE(hashTable.Contains(1336));
		EXPECT_FALSE(hashTable.Contains(1338));
		EXPECT_NE(hashTable.Find(1337), hashTable.end());
		EXPECT_NE(hashTable.Find(9001), hashTable.end());

		hashTable.Remove(hashTable.Find(1337));
		EXPECT_EQ(hashTable.GetSize(), 1);
		EXPECT_TRUE(hashTable.HasElements());
		EXPECT_NE(hashTable.begin(), hashTable.end());
		EXPECT_FALSE(hashTable.Contains(1337));
		EXPECT_TRUE(hashTable.Contains(9001));
		EXPECT_EQ(hashTable.Find(1337), hashTable.end());
		EXPECT_NE(hashTable.Find(9001), hashTable.end());

		hashTable.Remove(hashTable.Find(9001));
		EXPECT_EQ(hashTable.GetSize(), 0);
		EXPECT_TRUE(hashTable.IsEmpty());
		EXPECT_EQ(hashTable.begin(), hashTable.end());
		EXPECT_FALSE(hashTable.Contains(1337));
		EXPECT_FALSE(hashTable.Contains(9001));
		EXPECT_EQ(hashTable.Find(1337), hashTable.end());
		EXPECT_EQ(hashTable.Find(9001), hashTable.end());
	}
}
