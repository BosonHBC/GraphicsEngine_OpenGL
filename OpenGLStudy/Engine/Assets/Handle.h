/*
	AssetManager manages all kinds of resources like models, effects and some assets that only need to load once.
	Manager is used with Handle to reference counting this asset.
	Each of asset has a static AssetManager<key, asset>.
	Key: key represents how this asset is loaded. Usually it is a string which has the path to the asset in the driver.
	Asset: the type of the asset, such as Model.

	CREDITS TO: JOHN PAUL, professor of EAE 6320, University of Utah,
	Implementations are inspired by JOHN PAUL's engine. I have made some changes to fulfill my requirement.
*/

#pragma once

namespace Assets {
	/** Forward declaration*/
	template< class tAsset, class tKey>
	class cAssetManager;

	template<class tAsset>
	class cHandle
	{
	public:
		// All default handles are invalid handle.
		cHandle() { m_index = s_InvalidIndex; }
		// rule of three
		~cHandle() { m_index = s_InvalidIndex; };
		cHandle(const cHandle& i_other) { m_index = i_other.m_index; }
		cHandle& operator = (const cHandle& i_other) { m_index = i_other.m_index; return *this; }

		bool operator ==(const cHandle& i_other) const { return m_index == i_other.m_index; }
		/** Setters */
		void InvalidateIndex() { m_index = s_InvalidIndex; }
		/** Getters */
		uint16_t GetIndex() const { return m_index; }

	private:
		// The index in the m_assetList in cAssetManager<tAsset>
		uint16_t m_index = s_InvalidIndex;
		static const uint16_t s_InvalidIndex = 0xffff;

		// constructor with index
		cHandle(const uint16_t i_index) : m_index(i_index){}

		// friend class that can asses private parameters
		template<class tAsset, class tKey>
		friend class cAssetManager;
	};

}
