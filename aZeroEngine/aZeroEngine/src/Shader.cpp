#include "Shader.h"

Shader::Shader(std::string_view srcPath)
{
	m_compiledShader = Helper::loadBlobFromFile({ srcPath.begin(), srcPath.end() });
	if (!m_compiledShader)
		throw;
}
