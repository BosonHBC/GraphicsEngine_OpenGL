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
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "Handle.h"
namespace Assets {
	template< class tAsset , class tKey = std::string>
	class cAssetManager
	{
	public:
		cAssetManager() = default;
		~cAssetManager() { CleanUp(); }
		
		/** Usage functions*/
		// Load function by the key and extra arguments
		// ExtraArgument is needed because some asset need to specify some variables when it is loaded
		template <typename ... ExtraArguments> 
		bool Load(const tKey& i_key, cHandle<tAsset> & o_asset, ExtraArguments&&... i_arguments);
		// Copy from i_handle to o_handle, increase reference count;
		bool Copy(const cHandle<tAsset> & i_handle, cHandle<tAsset> & o_handle);
		// Duplicate asset from i_handle to o_handle
		bool Duplicate(const cHandle<tAsset> & i_handle, cHandle<tAsset> & o_handle);
		// Release the Asset, free the memory
		bool Release(cHandle<tAsset> & io_handle);
		
		/** Getters */
		tAsset* Get(const cHandle<tAsset>& i_handle);

		/** Clean up*/
		bool CleanUp();

	private:
		// This struct define the reference of an asset
		// helping for counting reference

		struct sAssetRecord
		{
			tAsset* pAsset;
			uint16_t ReferenceCount;

			sAssetRecord(tAsset* const i_pAsset, const uint16_t i_referenceCount) : pAsset(i_pAsset), ReferenceCount(i_referenceCount) 
			{}
			~sAssetRecord() { pAsset = nullptr; ReferenceCount = 0; }
		};

		// List to store the asset
		std::vector<sAssetRecord> m_assetList;

		// This map store the pair of key to the handle
		// every time the user wants to load a asset that has already been loaded,
		// It will return the existing one from this map.
		std::map<tKey, cHandle<tAsset>> m_keyToAssetHandle_Map;
		std::mutex m_mutex;
	};



}

// include the inline implementation
#include "cAssetManager.inl"