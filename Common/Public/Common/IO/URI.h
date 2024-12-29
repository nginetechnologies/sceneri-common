#pragma once

#include <Common/IO/TPath.h>
#include <Common/IO/URIView.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::IO
{
	extern template struct TPath<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;

	struct TRIVIAL_ABI URI : public TPath<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>
	{
		using BaseType = TPath<URICharType, uint8(PathFlags::SupportQueries), URISeparator, MaximumURILength>;
		using TPath::TPath;

		URI(const TPath& path)
			: TPath(path)
		{
		}
		URI& operator=(const TPath& path)
		{
			TPath::operator=(path);
			return *this;
		}
		URI(TPath&& path)
			: TPath(Forward<TPath>(path))
		{
		}
		URI& operator=(TPath&& path)
		{
			TPath::operator=(Forward<TPath>(path));
			return *this;
		}

		[[nodiscard]] operator ConstURIView() const LIFETIME_BOUND
		{
			return {m_path.GetView().begin(), m_path.GetSize()};
		}
		[[nodiscard]] ConstURIView GetView() const LIFETIME_BOUND
		{
			return {m_path.GetView().begin(), m_path.GetSize()};
		}

		template<typename... Args>
		[[nodiscard]] static URI Combine(const Args&... args)
		{
			return TPath::Combine(args...);
		}

		template<typename... Args>
		[[nodiscard]] static URI Merge(const Args&... args)
		{
			return TPath::Merge(args...);
		}

		[[nodiscard]] static URI Escape(const ConstURIView view) noexcept
		{
			return URI(BaseType::StringType::Escape(view.GetStringView()));
		}

		[[nodiscard]] static Optional<URI> Unescape(const ConstURIView view) noexcept
		{
			if (Optional<BaseType::StringType> decodedString = BaseType::StringType::Unescape(view.GetStringView()))
			{
				return URI(Move(*decodedString));
			}
			else
			{
				return Invalid;
			}
		}

		void EscapeCharacters()
		{
			*this = URI(Escape(GetView()));
		}

		void EscapeSpaces()
		{
			m_path.ReplaceAllOccurrences(MAKE_URI_LITERAL(' '), MAKE_URI_LITERAL("%20"));
		}
	};
}
