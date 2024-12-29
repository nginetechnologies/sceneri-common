#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/IO/PathView.h>
#include <Common/IO/Path.h>

namespace ngine::Tests
{
	UNIT_TEST(PathView, Equals)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
		);

		if constexpr (IO::PathView::CaseSensitive)
		{
			EXPECT_NE(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("PAth2"), MAKE_PATH("File.mp3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_NE(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mp3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_NE(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mP3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_NE(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mP3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
		}
		else
		{
			EXPECT_EQ(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("PAth2"), MAKE_PATH("File.mp3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_EQ(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mp3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_EQ(
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mP3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
			EXPECT_EQ(
				IO::Path::Combine(MAKE_PATH("dev"), MAKE_PATH("Path2"), MAKE_PATH("file.mP3")).GetView(),
				IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
			);
		}
	}

	UNIT_TEST(PathView, GetRightMostExtension)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetRightMostExtension(),
			MAKE_PATH(".mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetRightMostExtension(),
			MAKE_PATH(".mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetRightMostExtension(),
			MAKE_PATH(".mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetRightMostExtension(),
			MAKE_PATH(".mp3")
		);
	}

	UNIT_TEST(PathView, GetLeftMostExtension)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetLeftMostExtension(),
			MAKE_PATH(".mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetLeftMostExtension(),
			MAKE_PATH(".sub")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetLeftMostExtension(),
			MAKE_PATH(".sub")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetLeftMostExtension(),
			MAKE_PATH(".sub")
		);
	}

	UNIT_TEST(PathView, GetAllExtensions)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetAllExtensions(),
			MAKE_PATH(".mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetAllExtensions(),
			MAKE_PATH(".sub.mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetAllExtensions(),
			MAKE_PATH(".sub.next.mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetAllExtensions(),
			MAKE_PATH(".sub.next.mp3")
		);
	}

	UNIT_TEST(PathView, GetParentExtension)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).GetView().GetParentExtension(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetParentExtension(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File"))
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File")).GetView().GetParentExtension(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File"))
		);
	}

	UNIT_TEST(PathView, HasExtension)
	{
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).HasExtension());
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File")).HasExtension());
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).HasExtension());
	}

	UNIT_TEST(PathView, HasExactExtensions)
	{
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).HasExactExtensions(MAKE_PATH(".avi")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).HasExactExtensions(MAKE_PATH(".mp")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).HasExactExtensions(MAKE_PATH("mp3")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).HasExactExtensions(MAKE_PATH(".mp3")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).HasExactExtensions(MAKE_PATH(".avi")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).HasExactExtensions(MAKE_PATH(".mp3")));
	}

	UNIT_TEST(PathView, StartsWithExtensions)
	{
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).StartsWithExtensions(MAKE_PATH(".avi")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).StartsWithExtensions(MAKE_PATH(".mp")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).StartsWithExtensions(MAKE_PATH("mp3")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).StartsWithExtensions(MAKE_PATH(".mp3")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).StartsWithExtensions(MAKE_PATH(".avi"))
		);
		EXPECT_TRUE(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).StartsWithExtensions(MAKE_PATH(".mp3.avi"))
		);
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).StartsWithExtensions(MAKE_PATH(".mp3")));
	}

	UNIT_TEST(PathView, EndsWithExtensions)
	{
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).EndsWithExtensions(MAKE_PATH(".avi")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).EndsWithExtensions(MAKE_PATH(".mp")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).EndsWithExtensions(MAKE_PATH("mp3")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).EndsWithExtensions(MAKE_PATH(".mp3")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).EndsWithExtensions(MAKE_PATH(".avi")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3.avi")).EndsWithExtensions(MAKE_PATH(".mp3.avi"))
		);
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).EndsWithExtensions(MAKE_PATH(".mp3")));
	}

	UNIT_TEST(PathView, GetFileName)
	{
		EXPECT_EQ(IO::Path(MAKE_PATH("File.mp3")).GetFileName(), MAKE_PATH("File.mp3"));
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetFileName(),
			MAKE_PATH("File.mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetFileName(),
			MAKE_PATH("File.sub.mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetFileName(),
			MAKE_PATH("File.sub.next.mp3")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetFileName(),
			MAKE_PATH("File.sub.next.mp3")
		);
	}

	UNIT_TEST(PathView, GetFileNameWithoutExtensions)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetFileNameWithoutExtensions(),
			MAKE_PATH("File")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetFileNameWithoutExtensions(),
			MAKE_PATH("File")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetFileNameWithoutExtensions(),
			MAKE_PATH("File")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetFileNameWithoutExtensions(),
			MAKE_PATH("File")
		);
	}

	UNIT_TEST(PathView, GetWithoutExtensions)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetWithoutExtensions(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetWithoutExtensions(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetWithoutExtensions(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3"))
				.GetView()
				.GetWithoutExtensions(),
			IO::Path::Combine(MAKE_PATH("https://www.mySite.com"), MAKE_PATH("Path2.dir"), MAKE_PATH("File")).GetView()
		);
	}

	UNIT_TEST(PathView, GetParentPath)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetParentPath(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetParentPath(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetParentPath(),
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir")).GetView()
		);
	}

	UNIT_TEST(PathView, GetFirstPath)
	{
		EXPECT_EQ(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetFirstPath(), MAKE_PATH("Dev"));
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3")).GetView().GetFirstPath(),
			MAKE_PATH("Dev")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.next.mp3")).GetView().GetFirstPath(),
			MAKE_PATH("Dev")
		);
	}

	UNIT_TEST(PathView, GetSharedParentPath)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetSharedParentPath(MAKE_PATH("Dev")),
			MAKE_PATH("")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
				.GetView()
				.GetSharedParentPath(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).GetView()),
			IO::Path::Combine(MAKE_PATH("Dev")).GetView()
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3"))
				.GetView()
				.GetSharedParentPath(MAKE_PATH("Dev")),
			MAKE_PATH("")
		);
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir"), MAKE_PATH("File.sub.mp3"))
				.GetView()
				.GetSharedParentPath(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2.dir")).GetView()),
			MAKE_PATH("Dev")
		);
	}

	UNIT_TEST(PathView, IsRelativeTo)
	{
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().IsRelativeTo(MAKE_PATH("Dev")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().IsRelativeTo(MAKE_PATH("Other")));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().IsRelativeTo(MAKE_PATH("Dev2")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().IsRelativeTo(MAKE_PATH("Dev")));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
		              .GetView()
		              .IsRelativeTo(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).GetView()));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
		               .GetView()
		               .IsRelativeTo(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path")).GetView()));
		EXPECT_FALSE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
		               .GetView()
		               .IsRelativeTo(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File")).GetView()));
		EXPECT_TRUE(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
		              .GetView()
		              .IsRelativeTo(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()));
	}

	UNIT_TEST(PathView, GetRelativeToParent)
	{
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView().GetRelativeToParent(MAKE_PATH("Dev")),
			IO::Path::Combine(MAKE_PATH("Path2"), MAKE_PATH("File.mp3")).GetView()
		);
		EXPECT_EQ(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).GetView().GetRelativeToParent(MAKE_PATH("Dev")), MAKE_PATH("Path2"));
		EXPECT_EQ(
			IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2"), MAKE_PATH("File.mp3"))
				.GetView()
				.GetRelativeToParent(IO::Path::Combine(MAKE_PATH("Dev"), MAKE_PATH("Path2")).GetView()),
			MAKE_PATH("File.mp3")
		);
	}

	UNIT_TEST(PathView, RepeatedSlashes)
	{
		IO::Path path{MAKE_PATH("/test///test1//test2/test3/////test4/test5.test6")};
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		path.MakeForwardSlashes();
		EXPECT_EQ(path, MAKE_PATH("/test///test1//test2/test3/////test4"));
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		path.MakeForwardSlashes();
		EXPECT_EQ(path, MAKE_PATH("/test///test1//test2/test3"));
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		path.MakeForwardSlashes();
		EXPECT_EQ(path, MAKE_PATH("/test///test1//test2"));
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		path.MakeForwardSlashes();
		EXPECT_EQ(path, MAKE_PATH("/test///test1"));
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		path.MakeForwardSlashes();
		EXPECT_EQ(path, MAKE_PATH("/test"));
		path.MakeNativeSlashes();
		path = IO::Path{path.GetParentPath()};
		EXPECT_TRUE(path.IsEmpty());
	}
}
