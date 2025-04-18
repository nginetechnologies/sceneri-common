#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/String.h>

namespace ngine::Tests
{
	UNIT_TEST(String, DefaultConstruct)
	{
		{
			String string;
			EXPECT_EQ(string.GetSize(), 0u);
			EXPECT_EQ(string.GetCapacity(), 0u);
			EXPECT_TRUE(string.IsEmpty());
			EXPECT_FALSE(string.HasElements());
			EXPECT_EQ(string.GetData(), nullptr);
		}

		{
			String string(Memory::Reserve, 1u);
			EXPECT_EQ(string.GetSize(), 0u);
			EXPECT_EQ(string.GetCapacity(), 1u);
			EXPECT_TRUE(string.IsEmpty());
			EXPECT_FALSE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string.GetView(), "");
		}

		{
			String string(Memory::ConstructWithSize, Memory::Uninitialized, 1u);
			EXPECT_EQ(string.GetSize(), 1u);
			EXPECT_EQ(string.GetCapacity(), 1u);
			EXPECT_FALSE(string.IsEmpty());
			EXPECT_TRUE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string[1], '\0');
		}
	}

	UNIT_TEST(String, ConstructFromView)
	{
		{
			String string("yep");
			EXPECT_EQ(string.GetSize(), 3u);
			EXPECT_EQ(string.GetCapacity(), 3u);
			EXPECT_FALSE(string.IsEmpty());
			EXPECT_TRUE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, "yep");
		}

		{
			String string(ConstStringView("yep"));
			EXPECT_EQ(string.GetSize(), 3u);
			EXPECT_EQ(string.GetCapacity(), 3u);
			EXPECT_FALSE(string.IsEmpty());
			EXPECT_TRUE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, ConstStringView("yep"));
		}
	}

	UNIT_TEST(String, ConstructFromBuffer)
	{
		String string("myStr", 5u);
		EXPECT_EQ(string.GetSize(), 5u);
		EXPECT_EQ(string.GetCapacity(), 5u);
		EXPECT_FALSE(string.IsEmpty());
		EXPECT_TRUE(string.HasElements());
		EXPECT_NE(string.GetData(), nullptr);
		EXPECT_EQ(string, "myStr");
	}

	UNIT_TEST(String, ConstructFromZeroTerminated)
	{
		{
			constexpr ZeroTerminatedStringView str = "";

			String string(str);
			EXPECT_EQ(string.GetSize(), 0u);
			EXPECT_EQ(string.GetCapacity(), 0u);
			EXPECT_TRUE(string.IsEmpty());
			EXPECT_FALSE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, "");
		}

		{
			constexpr ZeroTerminatedStringView str = "test";

			String string(str);
			EXPECT_EQ(string.GetSize(), 4u);
			EXPECT_EQ(string.GetCapacity(), 4u);
			EXPECT_FALSE(string.IsEmpty());
			EXPECT_TRUE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, "test");
		}
	}

	UNIT_TEST(String, AssignFromZeroTerminated)
	{
		{
			constexpr ZeroTerminatedStringView str = "";

			String string = str;
			EXPECT_EQ(string.GetSize(), 0u);
			EXPECT_EQ(string.GetCapacity(), 0u);
			EXPECT_TRUE(string.IsEmpty());
			EXPECT_FALSE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, "");
		}

		{
			constexpr ZeroTerminatedStringView str = "test";

			String string = str;
			EXPECT_EQ(string.GetSize(), 4u);
			EXPECT_EQ(string.GetCapacity(), 4u);
			EXPECT_FALSE(string.IsEmpty());
			EXPECT_TRUE(string.HasElements());
			EXPECT_NE(string.GetData(), nullptr);
			EXPECT_EQ(string, "test");
		}
	}

	UNIT_TEST(String, CopyConstruct)
	{
		const String original = "original";
		const String other(original);

		EXPECT_EQ(other.GetSize(), 8u);
		EXPECT_EQ(other.GetCapacity(), 8u);
		EXPECT_FALSE(other.IsEmpty());
		EXPECT_TRUE(other.HasElements());
		EXPECT_NE(other.GetData(), nullptr);
		EXPECT_EQ(other, "original");
	}

	UNIT_TEST(String, ConstructFromFlatString)
	{
		const FlatString<9> original = "original";
		const String other(original);

		EXPECT_EQ(other.GetSize(), 8u);
		EXPECT_EQ(other.GetCapacity(), 8u);
		EXPECT_FALSE(other.IsEmpty());
		EXPECT_TRUE(other.HasElements());
		EXPECT_NE(other.GetData(), nullptr);
		EXPECT_EQ(other, "original");
	}

#if PLATFORM_WINDOWS
	UNIT_TEST(String, StringToWideString)
	{
		const String standard = "standard";
		const WideString wide(standard);

		EXPECT_EQ(wide.GetSize(), 8u);
		EXPECT_EQ(wide.GetCapacity(), 8u);
		EXPECT_FALSE(wide.IsEmpty());
		EXPECT_TRUE(wide.HasElements());
		EXPECT_NE(wide.GetData(), nullptr);
		EXPECT_EQ(wide, L"standard");
	}

	UNIT_TEST(String, WideStringToString)
	{
		const WideString wide = L"standard";
		const String string(wide);

		EXPECT_EQ(string.GetSize(), 8u);
		EXPECT_EQ(string.GetCapacity(), 8u);
		EXPECT_FALSE(string.IsEmpty());
		EXPECT_TRUE(string.HasElements());
		EXPECT_NE(string.GetData(), nullptr);
		EXPECT_EQ(string, "standard");
	}
#endif

	UNIT_TEST(String, UTF8StringToUTF16String)
	{
		const UTF8String utf8 = u8"standard";
		const UTF16String utf16(utf8);

		EXPECT_EQ(utf16.GetSize(), 8u);
		EXPECT_EQ(utf16.GetCapacity(), 8u);
		EXPECT_FALSE(utf16.IsEmpty());
		EXPECT_TRUE(utf16.HasElements());
		EXPECT_NE(utf16.GetData(), nullptr);
		EXPECT_EQ(utf16, u"standard");
	}

	UNIT_TEST(String, UTF8StringToUTF32String)
	{
		const UTF8String utf8 = u8"standard";
		const UTF32String utf32(utf8);

		EXPECT_EQ(utf32.GetSize(), 8u);
		EXPECT_EQ(utf32.GetCapacity(), 8u);
		EXPECT_FALSE(utf32.IsEmpty());
		EXPECT_TRUE(utf32.HasElements());
		EXPECT_NE(utf32.GetData(), nullptr);
		EXPECT_EQ(utf32, U"standard");
	}

	UNIT_TEST(String, UTF16StringToUTF8String)
	{
		const UTF16String utf16 = u"standard";
		const UTF8String utf8(utf16);

		EXPECT_EQ(utf8.GetSize(), 8u);
		EXPECT_EQ(utf8.GetCapacity(), 8u);
		EXPECT_FALSE(utf8.IsEmpty());
		EXPECT_TRUE(utf8.HasElements());
		EXPECT_NE(utf8.GetData(), nullptr);
		EXPECT_EQ(utf8, u8"standard");
	}

	UNIT_TEST(String, UTF16StringToUTF32String)
	{
		const UTF16String utf16 = u"standard";
		const UTF32String utf32(utf16);

		EXPECT_EQ(utf32.GetSize(), 8u);
		EXPECT_EQ(utf32.GetCapacity(), 8u);
		EXPECT_FALSE(utf32.IsEmpty());
		EXPECT_TRUE(utf32.HasElements());
		EXPECT_NE(utf32.GetData(), nullptr);
		EXPECT_EQ(utf32, U"standard");
	}

	UNIT_TEST(String, UTF32StringToUTF8String)
	{
		const UTF32String utf32 = U"standard";
		const UTF8String utf8(utf32);

		EXPECT_EQ(utf8.GetSize(), 8u);
		EXPECT_EQ(utf8.GetCapacity(), 8u);
		EXPECT_FALSE(utf8.IsEmpty());
		EXPECT_TRUE(utf8.HasElements());
		EXPECT_NE(utf8.GetData(), nullptr);
		EXPECT_EQ(utf8, u8"standard");
	}

	UNIT_TEST(String, UTF32StringToUTF16String)
	{
		const UTF32String utf32 = U"standard";
		const UTF16String utf16(utf32);

		EXPECT_EQ(utf16.GetSize(), 8u);
		EXPECT_EQ(utf16.GetCapacity(), 8u);
		EXPECT_FALSE(utf16.IsEmpty());
		EXPECT_TRUE(utf16.HasElements());
		EXPECT_NE(utf16.GetData(), nullptr);
		EXPECT_EQ(utf16, u"standard");
	}

	UNIT_TEST(String, MoveConstruct)
	{
		String string = "test";
		const String moved = Move(string);

		EXPECT_FALSE(string.HasElements());
		EXPECT_EQ(string.GetCapacity(), 0u);
		EXPECT_EQ(string.GetData(), nullptr);

		EXPECT_EQ(moved.GetSize(), 4u);
		EXPECT_EQ(moved.GetCapacity(), 4u);
		EXPECT_FALSE(moved.IsEmpty());
		EXPECT_TRUE(moved.HasElements());
		EXPECT_NE(moved.GetData(), nullptr);
		EXPECT_EQ(moved, "test");
	}

	UNIT_TEST(String, GetView)
	{
		const String string = "test";
		const ConstStringView view = string.GetView();
		EXPECT_TRUE(view.HasElements());
		EXPECT_FALSE(view.IsEmpty());
		EXPECT_EQ(view, "test");
		EXPECT_EQ(*string.end(), '\0');
	}

	UNIT_TEST(String, GetZeroTerminated)
	{
		const String string = "test";
		EXPECT_TRUE(!strcmp(string.GetZeroTerminated().GetData(), "test"));
		EXPECT_EQ(strlen(string.GetZeroTerminated().GetData()), 4u);
		EXPECT_EQ(string.GetZeroTerminated().GetSize(), 4u);
	}

	UNIT_TEST(String, Resize)
	{
		String string = "test";
		string.Resize(2u);
		EXPECT_EQ(string.GetSize(), 2u);
		EXPECT_EQ(string, "te");
	}

	UNIT_TEST(String, Operators)
	{
		{
			String test = "test";
			test += "25";
			EXPECT_EQ(test, "test25");
		}

		{
			String test = String("test") + "115";
			EXPECT_EQ(test, "test115");
		}

		{
			String test = String("test") + 'b';
			EXPECT_EQ(test, "testb");
		}

		{
			String test = "test";
			test += 'a';
			EXPECT_EQ(test, "testa");
		}
	}

	UNIT_TEST(String, OperatorsWithFlatString)
	{
		{
			FlatString<5> flat = "test";
			String test = flat;
			test += flat;
			EXPECT_EQ(test, "testtest");
		}

		{
			FlatString<5> flat = "test";
			String test = String(flat) + flat;
			EXPECT_EQ(test, "testtest");
		}

		{
			FlatString<5> flat = "test";
			String test = flat;
			bool re = test == flat;
			EXPECT_EQ(re, true);
		}
	}

	UNIT_TEST(String, ReplaceOccurrences)
	{
		String test = "that is not true";
		test.ReplaceCharacterOccurrences('t', ' ');
		EXPECT_EQ(test, " ha  is no   rue");
	}

	UNIT_TEST(String, TrimEnd)
	{
		String test = "that";
		test.TrimNumberOfTrailingCharacters(2u);
		EXPECT_EQ(test, "th");
	}

	UNIT_TEST(String, Subscript)
	{
		String test = "tester";
		EXPECT_EQ(test[0], 't');
		EXPECT_EQ(test[1], 'e');
		EXPECT_EQ(test[2], 's');
		EXPECT_EQ(test[3], 't');
		EXPECT_EQ(test[4], 'e');
		EXPECT_EQ(test[5], 'r');
	}

	UNIT_TEST(String, Merge)
	{
		const String string = String::Merge("this ", "is ", "merged", "!");
		EXPECT_EQ(string, "this is merged!");
	}

	UNIT_TEST(String, AssignFrom)
	{
		String test = "gone";
		test = "new";
		EXPECT_EQ(test, "new");
	}

	UNIT_TEST(StringView, ToInteger)
	{
		EXPECT_EQ(ConstStringView{"0"}.ToIntegral<int>(), 0);
		EXPECT_EQ(ConstStringView{"1337"}.ToIntegral<int>(), 1337);
		{
			EXPECT_FALSE(ConstStringView{""}.TryToIntegral<int>().success);
		}
		{
			auto result = ConstStringView{"1337"}.TryToIntegral<int>();
			EXPECT_TRUE(result.success && result.value == 1337);
		}
		{
			auto result = ConstStringView{"-9001"}.TryToIntegral<int>();
			EXPECT_TRUE(result.success && result.value == -9001);
		}
		{
			auto result = ConstStringView{"6"}.TryToIntegral<int>();
			EXPECT_TRUE(result.success && result.value == 6);
		}
		{
			EXPECT_FALSE(ConstStringView{"0701e83f-9bf9-4ef3-89e7-d2f9bab4f023"}.TryToIntegral<int>().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"c0f01726-2934-4402-9818-caecd5096ab3"}.TryToIntegral<int>().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"g"}.TryToIntegral<int>().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0g"}.TryToIntegral<int>().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"g0"}.TryToIntegral<int>().success);
		}
	}

	UNIT_TEST(StringView, ToFloat)
	{
		EXPECT_EQ(ConstStringView{"0"}.ToFloat(), 0);
		EXPECT_EQ(ConstStringView{"1337"}.ToFloat(), 1337);
		EXPECT_EQ(ConstStringView{"1337.95"}.ToFloat(), 1337.95f);
		{
			EXPECT_FALSE(ConstStringView{""}.TryToFloat().success);
		}
		{
			auto result = ConstStringView{"1337"}.TryToFloat();
			EXPECT_TRUE(result.success && result.value == 1337);
		}
		{
			auto result = ConstStringView{"-9001"}.TryToFloat();
			EXPECT_TRUE(result.success && result.value == -9001);
		}
		{
			auto result = ConstStringView{"6"}.TryToFloat();
			EXPECT_TRUE(result.success && result.value == 6);
		}
		{
			auto result = ConstStringView{"1e6"}.TryToFloat();
			EXPECT_TRUE(result.success && result.value == 1e6f);
		}
		{
			auto result = ConstStringView{"1.1e6"}.TryToFloat();
			EXPECT_TRUE(result.success && result.value == 1.1e6f);
		}
		{
			EXPECT_FALSE(ConstStringView{"0701e83f-9bf9-4ef3-89e7-d2f9bab4f023"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"c0f01726-2934-4402-9818-caecd5096ab3"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"g"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0g"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"g0"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0."}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"."}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0.0e"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0e"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0.e"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"0.x"}.TryToFloat().success);
		}
		{
			EXPECT_FALSE(ConstStringView{"1e6.1"}.TryToFloat().success);
		}
	}

	UNIT_TEST(StringView, FindRanges)
	{
		constexpr ConstStringView test = "1133711";

		EXPECT_EQ(test.FindFirstOf('1'), 0u);
		EXPECT_EQ(test.FindFirstOf('1', 1), 1u);
		EXPECT_EQ(test.FindFirstOf('1', 2), 5u);

		EXPECT_EQ(test.FindLastOf('1'), 6u);
		EXPECT_EQ(test.FindLastOf('1', 1), 5u);
		EXPECT_EQ(test.FindLastOf('1', 2), 1u);

		{
			const ConstStringView substring = test.FindFirstRange("11");
			EXPECT_EQ(substring.GetSize(), 2u);
			EXPECT_EQ(&substring[0], &test[0]);
		}

		{
			const ConstStringView substring = test.FindLastRange("11");
			EXPECT_EQ(substring.GetSize(), 2u);
			EXPECT_EQ(&substring[0], &test[5]);
		}

		{
			const ConstStringView substring = test.GetSubstring(0u, 1u);
			EXPECT_EQ(substring.GetSize(), 1u);
			EXPECT_EQ(&substring[0], &test[0]);
		}

		{
			const ConstStringView substring = test.GetSubstring(1u, 1u);
			EXPECT_EQ(substring.GetSize(), 1u);
			EXPECT_EQ(&substring[0], &test[1]);
		}

		{
			const ConstStringView substring = test.GetSubstringFrom(1u);
			EXPECT_EQ(substring.GetSize(), test.GetSize() - 1u);
			EXPECT_EQ(&substring[0], &test[1]);
		}

		{
			const ConstStringView substring = test.GetSubstringUpTo(1u);
			EXPECT_EQ(substring.GetSize(), 1u);
			EXPECT_EQ(&substring[0], &test[0]);
		}
	}
}
