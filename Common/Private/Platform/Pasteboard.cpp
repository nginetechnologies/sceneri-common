#include <Common/Platform/Pasteboard.h>

#if PLATFORM_APPLE_MACOS
#import <AppKit/NSPasteboard.h>
#import <Foundation/NSString.h>
#elif PLATFORM_APPLE_IOS
#import <UIKit/UIPasteboard.h>
#import <Foundation/NSString.h>
#elif PLATFORM_EMSCRIPTEN
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#elif PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#endif

#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Function/Function.h>

namespace ngine::Platform
{
	/* static */ bool Pasteboard::PasteText(const ConstUnicodeStringView text)
	{
#if PLATFORM_APPLE_MACOS
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		[pasteboard clearContents];
		NSString* nsText = [[NSString alloc] initWithBytes:text.GetData() length:text.GetDataSize() encoding:NSUTF16LittleEndianStringEncoding];
		[pasteboard setString:nsText forType:NSPasteboardTypeString];
		return true;
#elif PLATFORM_APPLE_IOS
		UIPasteboard* pasteboard = [UIPasteboard generalPasteboard];
		NSString* nsText = [[NSString alloc] initWithBytes:text.GetData() length:text.GetDataSize() encoding:NSUTF16LittleEndianStringEncoding];
		pasteboard.string = nsText;
		return true;
#elif PLATFORM_EMSCRIPTEN
		em_proxying_queue* queue = emscripten_proxy_get_system_queue();
		pthread_t target = emscripten_main_runtime_thread_id();
		UnicodeString* pText{new UnicodeString(text)};
		[[maybe_unused]] const bool called = emscripten_proxy_async(
																					 queue,
																					 target,
																					 [](void* pUserData)
																					 {
																						 UnicodeString* pText = reinterpret_cast<UnicodeString*>(pUserData);

																						 PUSH_CLANG_WARNINGS
																						 DISABLE_CLANG_WARNING("-Wdollar-in-identifier-extension")
																						 // clang-format off
				EM_ASM(
					{
						var url = UTF16ToString($0);
						navigator.clipboard.writeText(url).catch(err => {
							console.error('Failed to copy text to clipboard:', err);
						});
					},
					pText->GetZeroTerminated().GetData()
				);
																						 // clang-format on
																						 POP_CLANG_WARNINGS

																						 delete pText;
																					 },
																					 pText
																				 ) == 1;
		Assert(called);
		return true;
#elif PLATFORM_WINDOWS
		if (!::OpenClipboard(nullptr))
		{
			return false;
		}

		::EmptyClipboard();

		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (text.GetSize() + 1) * sizeof(wchar_t));
		if (hGlobal == nullptr)
		{
			::CloseClipboard();
			return false;
		}

		// Copy the text to the global memory
		wchar_t* globalMemory = static_cast<wchar_t*>(::GlobalLock(hGlobal));
		wcscpy_s(globalMemory, text.GetSize() + 1, reinterpret_cast<const wchar_t*>(text.GetData()));
		::GlobalUnlock(hGlobal);

		::SetClipboardData(CF_UNICODETEXT, hGlobal);
		::CloseClipboard();
		return true;
#else
		UNUSED(text);
		Assert(false, "TODO: Implement pasteboard for platform");
		return false;
#endif
	}

	/* static */ bool Pasteboard::GetText(GetTextCallback&& callback)
	{
#if PLATFORM_APPLE_MACOS
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		NSString* pastedText = [pasteboard stringForType:NSPasteboardTypeString];
		UnicodeString result{Memory::ConstructWithSize, Memory::Uninitialized, (UnicodeString::SizeType)[pastedText length]};
		static_assert(sizeof(unichar) == sizeof(UnicodeString::CharType));
		[pastedText getCharacters:reinterpret_cast<unichar*>(result.GetData()) range:NSMakeRange(0, result.GetSize())];
		callback(Move(result));
		return true;
#elif PLATFORM_APPLE_IOS
		UIPasteboard* pasteboard = [UIPasteboard generalPasteboard];
		NSString* pastedText = pasteboard.string;
		UnicodeString result{Memory::ConstructWithSize, Memory::Uninitialized, (UnicodeString::SizeType)[pastedText length]};
		static_assert(sizeof(unichar) == sizeof(UnicodeString::CharType));
		[pastedText getCharacters:reinterpret_cast<unichar*>(result.GetData()) range:NSMakeRange(0, result.GetSize())];
		callback(Move(result));
		return true;
#elif PLATFORM_EMSCRIPTEN
		em_proxying_queue* queue = emscripten_proxy_get_system_queue();
		pthread_t target = emscripten_main_runtime_thread_id();
		[[maybe_unused]] const bool called =
			emscripten_proxy_async(
				queue,
				target,
				[](void* pUserData)
				{
					GetTextCallback* pCallback = reinterpret_cast<GetTextCallback*>(pUserData);

					PUSH_CLANG_WARNINGS
					DISABLE_CLANG_WARNING("-Wdollar-in-identifier-extension")
					// clang-format off
				EM_ASM(
					{
						var callback = $0;
						navigator.clipboard.readText().then(text => {
						  const utf16Array = new Uint16Array(text.length + 1);
						  for (let i = 0; i < text.length; i++) {
							utf16Array[i] = text.charCodeAt(i);
						  }
						  utf16Array[text.length] = 0;
						  const length = text.length;

						  const ptr = Module["_malloc"](utf16Array.length * 2);
						  const data_on_heap = new Uint16Array(GROWABLE_HEAP_U16().buffer, ptr, utf16Array.length * 2);
						  data_on_heap.set(utf16Array);

						  Module["ccall"]("OnPaste", "void", [ "number", "number", "number" ], [ data_on_heap.byteOffset, utf16Array.length, callback ]);
						  
						  Module["_free"](ptr);
						}).catch(err => {
						  console.error("Failed to read text from clipboard:", err);
						  Module["ccall"]("OnPaste", "void", [ "number", "number", "number" ], [ 0, 0, callback ]);
						});
					},
					pCallback
				);
					// clang-format on
					POP_CLANG_WARNINGS
				},
				new GetTextCallback(Forward<GetTextCallback>(callback))
			) == 1;
		Assert(called);
		return true;
#elif PLATFORM_WINDOWS
		if (!OpenClipboard(nullptr))
		{
			callback({});
			return false;
		}

		HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
		if (hData)
		{
			// Lock the handle to get the text pointer
			wchar_t* globalMemory = static_cast<wchar_t*>(::GlobalLock(hData));
			if (globalMemory)
			{
				callback(UnicodeString{reinterpret_cast<char16_t*>(globalMemory), (UnicodeString::SizeType)wcslen(globalMemory)});
				::GlobalUnlock(hData);
				::CloseClipboard();
				return true;
			}
		}

		callback({});
		::CloseClipboard();
		return false;
#else
		Assert(false, "TODO: Implement pasteboard for platform");
		callback({});
		return false;
#endif
	}
}

#if PLATFORM_WEB
extern "C"
{
	EMSCRIPTEN_KEEPALIVE inline void OnPaste(const ngine::UnicodeCharType* pastePtr, const int length, void* pCallbackPtr)
	{
		using namespace ngine;
		Platform::Pasteboard::GetTextCallback* pCallback = reinterpret_cast<Platform::Pasteboard::GetTextCallback*>(pCallbackPtr);

		if (length > 0)
		{
			(*pCallback)(UnicodeString{ConstUnicodeStringView{pastePtr, (ConstUnicodeStringView::SizeType)length}});
		}
		else
		{
			(*pCallback)(UnicodeString{});
		}
		delete pCallback;
	}
}
#endif
