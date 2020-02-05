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
	template<class tAsset>
	class cHandle
	{
	public:
		cHandle();
		~cHandle();

	private:
		// The index in the m_assetList in cAssetManager<tAsset>
		uint16_t m_index;
	};

	// 
	// Inline function definition 
	//
	template<class tAsset>
	cHandle<tAsset>::cHandle()
	{
	}
	template<class tAsset>
	cHandle<tAsset>::~cHandle()
	{
	}
}
