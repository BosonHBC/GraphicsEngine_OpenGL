#pragma once
#include "AssetManager.h"
#include "Engine/Cores/Core.h"
#include <mutex>
#include "assert.h"
namespace Assets {


	template< class tAsset, class tKey /*= std::string*/>
	template < typename... ExtraArguments>
	bool cAssetManager<tAsset, tKey>::Load(const tKey& i_key, cHandle<tAsset> & o_asset, ExtraArguments&&... i_arguments) {

		// Scoped lock happens inside this scope
		{
			std::lock_guard<std::mutex> autoLock(m_mutex);
			// If the key exist in the map, get the existing one
			if (m_keyToAssetHandle_Map.find(i_key) != m_keyToAssetHandle_Map.end()) {
				auto _handle = m_keyToAssetHandle_Map[i_key];
				const auto _index = _handle.GetIndex();
				if (_index < m_assetList.size()) {
					// increase the reference count
					m_assetList[_index].ReferenceCount++;
					o_asset = _handle;
					return true;
				}
				else {
					// Wrong Index
					assert(false);
					printf("A handle has an index (%d) that's too big for the number of assets (%u)", _index, m_assetList.size());
					return false;
				}
			}
		}

		auto result = true;
		// Load new assets
		{
			tAsset* _pAsset = nullptr;
			if (tAsset::Load(i_key, _pAsset, i_arguments...)) {

				// Scoped lock happens inside this scope
				{
					std::lock_guard<std::mutex> autoLock(m_mutex);
					const auto recordCount = m_assetList.size();
					const uint16_t referenceCount = 1;
					// Asset load successfully

					// add to the list
					sAssetRecord _record(_pAsset, referenceCount);
					m_assetList.push_back(_record);

					// create handle
					auto _asset = cHandle<tAsset>(recordCount);
					// add to the map
					m_keyToAssetHandle_Map.insert({ i_key, _asset });
					// output the handle
					o_asset = _asset;
				}

			}
			else {
				_pAsset = nullptr;
				result = false;
			}
		}
		return result;
	}

	template< class tAsset, class tKey /*= std::string*/>
	bool Assets::cAssetManager<tAsset, tKey>::Copy(const cHandle<tAsset> & i_handle, cHandle<tAsset> & o_handle)
	{
		auto result = true;

		tAsset* const _asset = Get(i_handle);
		if (!(result = (_asset != nullptr))) {
			// Invalid input handle id
			return result;
		}
		// Scoped lock happens inside this scope
		{
			std::lock_guard<std::mutex> autoLock(m_mutex);
			const auto _index = i_handle.GetIndex();
			if (_index < m_assetList.size()) {
				// increase the reference count
				m_assetList[_index].ReferenceCount++;
				o_handle = i_handle;
			}
			else {
				// Index wrong
				result = false;
			}
		}
		return result;
	}

	template< class tAsset, class tKey /*= std::string*/>
	tAsset* cAssetManager<tAsset, tKey>::Get(const cHandle<tAsset>& i_handle)
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		{
			const auto index = i_handle.GetIndex();
			if (index < m_assetList.size()) {
				sAssetRecord& _record = m_assetList[index];
				if (_record.pAsset)
					return _record.pAsset;
			}
			else {
				// Index wrong
				assert(false);
				printf("A handle has an index (%d) that's too big for the number of assets (%u)", index, m_assetList.size());
			}
		}

		return nullptr;
	}

	template< class tAsset, class tKey /*= std::string*/>
	bool cAssetManager<tAsset, tKey>::Release(cHandle<tAsset> & io_handle)
	{
		auto result = true;
		// Scoped lock happens inside this scope
		{
			std::lock_guard<std::mutex> autoLock(m_mutex);
			const auto index = io_handle.GetIndex();
			if (index < m_assetList.size()) {
				sAssetRecord& _record = m_assetList[index];
				uint16_t _newReferenceCount = --_record.ReferenceCount;
				if (_newReferenceCount == 0) {
					// need to destroy the asset since no one is referencing the asset
					// In this case, the application should be closed?
					safe_delete(_record.pAsset);
					_record.ReferenceCount = 0;
				}
				else {
					// Still some other objects are referencing this asset, should not destroy
					_record.ReferenceCount = _newReferenceCount;
				}
			}
			else {
				// index wrong
				result = false;
				printf("A handle has an index (%d) that's too big for the number of assets (%u)", index, m_assetList.size());
			}
		}
		// Clean up o_handle
		io_handle.InvalidateIndex();
		return result;
	}



	template< class tAsset, class tKey /*= std::string*/>
	bool cAssetManager<tAsset, tKey>::CleanUp()
	{
		bool result = true;
		bool stillAssetsNotFreeYet = false;
		// Scoped lock happens inside this scope
		{
			std::lock_guard<std::mutex> autoLock(m_mutex);
			// When the manager(static variable) is destroyed,  main has finished.
			// So when the program run through here, there should be no more asset that is not released. If that's true, some obejct did not release the handle.
			for (auto record : m_assetList)
			{
				if (record.pAsset) {
					// TODO: Log error
					stillAssetsNotFreeYet = true;
					// There is no way to track which object is referencing this object.
					record.pAsset = nullptr;
					record.ReferenceCount = 0;
					result = false;
					assert(result);
				}
			}
			m_assetList.clear();
			m_keyToAssetHandle_Map.clear();
		}

		if (stillAssetsNotFreeYet)
			printf("There are still assets that are not freed yet!\n");

		return result;
	}


}
