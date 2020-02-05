#pragma once
#include "AssetManager.h"

namespace Assets {


	template< class tAsset, class tKey /*= std::string*/, typename... ExtraArguments>
	bool cAssetManager<tAsset, tKey>::Load(const tKey& i_key, cHandle<tAsset> & o_asset, ExtraArguments... i_arguments) {
		auto result = true;

		// If the key exist in the map, get the existing one
		if (m_keyToAssetHandle_Map.find(i_key) != m_keyToAssetHandle_Map.end()) {
			o_asset = m_keyToAssetHandle_Map[i_key];
		}
		else {
			tAsset* _pAsset = nullptr;
			if (tAsset::Load(i_key, _pAsset, i_arguments)) {

				const auto recordCount = m_assetList.size();
				const uint16_t referenceCount = 1;
				// Asset load successfully

				sAssetRecord _record(_pAsset, referenceCount);
				m_assetList.push_back(_record);

				o_asset = cHandle<tAsset>(recordCount);
			}
			else {
				_pAsset = nullptr;
				result = false;
			}
		}
		return result;
	}

	template< class tAsset, class tKey /*= std::string*/>
	tAsset* cAssetManager<tAsset, tKey>::Get(const cHandle<tAsset>& i_handle)
	{
		const auto index = i_handle.GetIndex();
		if (index < m_assetList.size()) {
			auto _record = m_assetList[index];
			++_record.ReferenceCount;
			return _record.pAsset;
		}
		else {
			// Index wrong

		}
		return nullptr;
	}

	template< class tAsset, class tKey /*= std::string*/>
	bool cAssetManager<tAsset, tKey>::Release(cHandle<tAsset> & io_asset)
	{
		return true;
	}



	template< class tAsset, class tKey /*= std::string*/>
	void cAssetManager<tAsset, tKey>::CleanUp()
	{

	}


}
