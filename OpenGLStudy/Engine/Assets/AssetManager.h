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

#include "Handle.h"
namespace Assets {
	template< class tAsset , class tKey = std::string>
	class cAssetManager
	{
	public:
		cAssetManager<tAsset, tKey>() = default;
		~cAssetManager<tAsset, tKey>() {CleanUp()};
		
		/** Usage functions*/
		// Load function by the key and extra arguments
		<typename ... ExtraArguments> // ExtraArgument is needed because some asset need to specify some variables when it is loaded
		bool Load(const tKey& i_key, cHandle<tAsset> & o_asset, ExtraArguments... i_arguments);
		// Release the Asset, free the memory
		bool Release(cHandle<tAsset> & io_asset);
		
		/** Getters */
		tAsset* Get(const cHandle<tAsset>& i_handle);

		/** Clean up*/
		void CleanUp();

	private:
		// This struct define the reference of an asset
		// helping for counting reference

		struct sAssetRecord
		{
			tAsset* ptr;
			uint16_t count;
		};

		//
		std::vector<sAssetRecord> m_assetList;

		// This map store the pair of key to the handle
		// every time the user wants to load a asset that has already been loaded,
		// It will return the existing one from this map.
		std::map<tKey, cHandle<tAsset>> m_keyToAssetHandle_Map;

	};

}

// include the inline implementation
#include "cAssetManager.inl"