#pragma once
#include "AssetManager.h"

namespace Assets {

	template< class tAsset, class tKey /*= std::string*/, typename... ExtraArguments>
	bool Load(const tKey& i_key, cHandle<tAsset> & o_asset, ExtraArguments... i_arguments) {
		return true;
	}

	template< class tAsset, class tKey /*= std::string*/>
	tAsset* Assets::cAssetManager<tAsset, tKey>::Get(const cHandle<tAsset>& i_handle)
	{
		return nullptr;
	}

	template< class tAsset, class tKey /*= std::string*/>
	bool Assets::cAssetManager<tAsset, tKey>::Release(cHandle<tAsset> & io_asset)
	{
		return true;
	}



	template< class tAsset, class tKey /*= std::string*/>
	void cAssetManager<tAsset, tKey>::CleanUp()
	{

	}


}
