#pragma once

namespace CatEngine
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<CatEngine::UUID>
	{
		std::size_t operator()(const CatEngine::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};
}
