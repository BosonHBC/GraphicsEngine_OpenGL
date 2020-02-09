#include "MatBlinn.h"
#include "Engine/Constants/Constants.h"
#include "assimp/scene.h"
#include "Externals/Lua/Includes.h"

namespace Graphics {


	bool cMatBlinn::Initialize(const std::string& i_path, aiMaterial* const i_aiMat)
	{
		bool result = true;
		std::string _diffusePath, _specularPath;
		// TODO: load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, m_matType,_diffusePath, _specularPath, m_diffuseIntensity, m_specularIntensity, m_shininess))) {
			printf("Fail to load material[%s] from LUA\n.");
			return result;
		}


		// Load diffuse texture
		std::string _texPath = "Contents/textures/";
		std::string _fileName = "";
		if (i_aiMat->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString _path;
			if (i_aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &_path) == AI_SUCCESS) {
				auto _idx = std::string(_path.data).rfind("\\");
				_fileName = std::string(_path.data).substr(_idx + 1);
			}
		}
		SetDiffuse(_texPath + _fileName);
		// Load specular texture
		if (i_aiMat->GetTextureCount(aiTextureType_SPECULAR)) {
			aiString _path;
			if (i_aiMat->GetTexture(aiTextureType_SPECULAR, 0, &_path) == AI_SUCCESS) {
				auto _idx = std::string(_path.data).rfind("\\");
				_fileName = std::string(_path.data).substr(_idx + 1);
			}
		}
		SetSpecular(_texPath + _fileName);

		//TODO: Temp set intensity and Shininess
		SetShininess(32.f);
		SetDiffuseIntensity(Color::White());
		SetSpecularIntensity(Color::White());
		return result;
	}

	bool cMatBlinn::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_diffuseTexID = glGetUniformLocation(i_programID, "diffuseTex");
		m_specularTexID = glGetUniformLocation(i_programID, "specularTex");

		m_shininessID = glGetUniformLocation(i_programID, "material.shininess");
		m_diffuseIntensityID = glGetUniformLocation(i_programID, "material.kd");
		m_specularIntensityID = glGetUniformLocation(i_programID, "material.ks");
		return result;
	}

	void cMatBlinn::UseMaterial()
	{
		glUniform1i(m_diffuseTexID, 0);
		glUniform1i(m_specularTexID, 1);

		cTexture* _diffuseTex = cTexture::s_manager.Get(m_diffuseTextureHandle);
		if (_diffuseTex) {
			_diffuseTex->UseTexture(GL_TEXTURE0);
		}
		cTexture* _specularTex = cTexture::s_manager.Get(m_specularTextureHandle);
		if (_specularTex) {
			_specularTex->UseTexture(GL_TEXTURE1);
		}

		glUniform1f(m_shininessID, m_shininess);
		glUniform3f(m_diffuseIntensityID, m_diffuseIntensity.r, m_diffuseIntensity.g, m_diffuseIntensity.b);
		glUniform3f(m_specularIntensityID, m_specularIntensity.r, m_specularIntensity.g, m_specularIntensity.b);

	}

	void cMatBlinn::CleanUpMaterialBind()
	{
		cTexture* _diffuseTex = cTexture::s_manager.Get(m_diffuseTextureHandle);
		if (_diffuseTex) {
			_diffuseTex->CleanUpTextureBind(GL_TEXTURE0);
		}
		cTexture* _specularTex = cTexture::s_manager.Get(m_specularTextureHandle);
		if (_specularTex) {
			_specularTex->CleanUpTextureBind(GL_TEXTURE1);
		}
	}

	void cMatBlinn::CleanUp()
	{
		cTexture::s_manager.Release(m_diffuseTextureHandle);
		cTexture::s_manager.Release(m_specularTextureHandle);

	}
	void cMatBlinn::SetDiffuse(const std::string& i_diffusePath)
	{
		auto result = true;
		if (!(result = cTexture::s_manager.Load(i_diffusePath, m_diffuseTextureHandle, false))) {
			if (result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_TEXTURE, m_diffuseTextureHandle, false))
			{
				//TODO: Use default texture, which is the white board

			}
			else {
				//TODO: print Fail to load default texture

			}
		}
	}

	void cMatBlinn::SetSpecular(const std::string& i_specularPath)
	{
		auto result = true;
		if (!(result = cTexture::s_manager.Load(i_specularPath, m_specularTextureHandle, false))) {
			if (result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_TEXTURE, m_specularTextureHandle, false))
			{
				//TODO: Use default texture, which is the white board

			}
			else {
				//TODO: print Fail to load default texture

			}
		}
	}

	void cMatBlinn::SetShininess(GLfloat i_shine)
	{
		m_shininess = i_shine;
	}

	void cMatBlinn::SetDiffuseIntensity(Color i_diffuseIntensity)
	{
		m_diffuseIntensity = i_diffuseIntensity;
	}

	void cMatBlinn::SetSpecularIntensity(Color i_specularIntensity)
	{
		m_specularIntensity = i_specularIntensity;
	}

	bool cMatBlinn::LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType, std::string& o_diffusePath, std::string& o_specularPath, Color& o_IntensityColor, Color& o_SpecularColor, float& o_Shineness)
	{
		bool result;
		lua_State* luaState = nullptr;
		// Initialize LUA
		if (!(result = InitializeLUA(i_path, luaState))) {
			ReleaseLUA(luaState);
			return result;
		}

		result = ReleaseLUA(luaState);
		return result;
	}

}