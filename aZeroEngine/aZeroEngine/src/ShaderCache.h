#pragma once
#include "Shader.h"
#include <map>

class ShaderCache
{
private:
	std::map<std::string, std::shared_ptr<Shader>> m_shaders;
	std::string m_srcDirectory = "../x64/Release/";

public:
	ShaderCache() = default;
	ShaderCache(const ShaderCache&) = delete;
	ShaderCache(ShaderCache&&) = delete;
	ShaderCache operator=(ShaderCache&&) = delete;
	ShaderCache operator=(const ShaderCache&) = delete;

	std::shared_ptr<Shader> loadShader(const std::string& name)
	{
		std::map<std::string, std::shared_ptr<Shader>>::const_iterator it = m_shaders.find(name);
		if (it != m_shaders.end())
			return it->second;

#ifdef _DEBUG
		m_srcDirectory = "../x64/Debug/";
#endif // _DEBUG

		m_shaders.emplace(name, std::make_shared<Shader>(m_srcDirectory + name + ".cso"));

		return m_shaders.at(name);
	}

	void removeShader(const std::string& name)
	{
		std::map<std::string, std::shared_ptr<Shader>>::const_iterator it = m_shaders.find(name);
		if (it != m_shaders.end())
			m_shaders.erase(name);
	}

	std::shared_ptr<Shader> getShader(const std::string& name)
	{
		std::map<std::string, std::shared_ptr<Shader>>::const_iterator it = m_shaders.find(name);
		if (it != m_shaders.end())
			return it->second;
		return nullptr;
	}
};