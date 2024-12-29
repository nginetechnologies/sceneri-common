#include <Common/Math/Random.h>
#include <Common/Math/Hash.h>

#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#include <thread>

namespace ngine::Math::Internal
{
	std::mt19937 SeedRandomEngine()
	{
		constexpr size N = std::mt19937::state_size * sizeof(typename std::mt19937::result_type);

		std::random_device source;
		std::random_device::result_type random_data[(N - 1) / sizeof(source()) + 1];
		std::generate(std::begin(random_data), std::end(random_data), std::ref(source));

		// Mix the random data by thread ID, to ensure that our threads start with different random generators (see thread_local below)
		const size threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
		random_data[0] ^= threadId;

		std::seed_seq seeds(std::begin(random_data), std::end(random_data));
		return std::mt19937(seeds);
	}
	static std::mt19937& GetRandomGenerator()
	{
		static thread_local std::mt19937 randomGenerator = SeedRandomEngine();
		return randomGenerator;
	}

	uint64 RandomUint64(const uint64 min, const uint64 max) noexcept
	{
		return std::uniform_int_distribution<uint64>(min, max)(GetRandomGenerator());
	}
	uint64 RandomUint64() noexcept
	{
		return std::uniform_int_distribution<uint64>()(GetRandomGenerator());
	}

	int64 RandomInt64(const int64 min, const int64 max) noexcept
	{
		return std::uniform_int_distribution<int64>(min, max)(GetRandomGenerator());
	}
	int64 RandomInt64() noexcept
	{
		return std::uniform_int_distribution<int64>()(GetRandomGenerator());
	}

	uint32 RandomUint32(const uint32 min, const uint32 max) noexcept
	{
		return std::uniform_int_distribution<uint32>(min, max)(GetRandomGenerator());
	}
	uint32 RandomUint32() noexcept
	{
		return std::uniform_int_distribution<uint32>()(GetRandomGenerator());
	}
	int32 RandomInt32(const int32 min, const int32 max) noexcept
	{
		return std::uniform_int_distribution<int32>(min, max)(GetRandomGenerator());
	}
	int32 RandomInt32() noexcept
	{
		return std::uniform_int_distribution<int32>()(GetRandomGenerator());
	}

	uint16 RandomUint16(const uint16 min, const uint16 max) noexcept
	{
		return std::uniform_int_distribution<uint16>(min, max)(GetRandomGenerator());
	}
	uint16 RandomUint16() noexcept
	{
		return std::uniform_int_distribution<uint16>()(GetRandomGenerator());
	}
	int16 RandomInt16(const int16 min, const int16 max) noexcept
	{
		return std::uniform_int_distribution<int16>(min, max)(GetRandomGenerator());
	}
	int16 RandomInt16() noexcept
	{
		return std::uniform_int_distribution<int16>()(GetRandomGenerator());
	}

	uint8 RandomUint8(const uint8 min, const uint8 max) noexcept
	{
		return (uint8)std::uniform_int_distribution<uint16>(min, max)(GetRandomGenerator());
	}
	uint8 RandomUint8() noexcept
	{
		return (uint8)std::uniform_int_distribution<uint16>(0u, Math::NumericLimits<uint8>::Max)(GetRandomGenerator());
	}
	int8 RandomInt8(const int8 min, const int8 max) noexcept
	{
		return (int8)std::uniform_int_distribution<int16>(min, max)(GetRandomGenerator());
	}
	int8 RandomInt8() noexcept
	{
		return (uint8)std::uniform_int_distribution<int16>(Math::NumericLimits<int8>::Min, Math::NumericLimits<int8>::Max)(GetRandomGenerator()
		);
	}

	float RandomFloat(const float min, const float max) noexcept
	{
		return std::uniform_real_distribution<float>(min, max)(GetRandomGenerator());
	}
	float RandomFloat() noexcept
	{
		return std::uniform_real_distribution<float>()(GetRandomGenerator());
	}

	double RandomDouble(const double min, const double max) noexcept
	{
		return std::uniform_real_distribution<double>(min, max)(GetRandomGenerator());
	}
	double RandomDouble() noexcept
	{
		return std::uniform_real_distribution<double>()(GetRandomGenerator());
	}
}
