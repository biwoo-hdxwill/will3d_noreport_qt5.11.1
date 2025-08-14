#include "shader_compiler.h"

#include <QString>
#include "../Common/W3Logger.h"
#include "WGLSLprogram.h"
ShaderCompiler::ShaderCompiler() {}

ShaderCompiler::~ShaderCompiler() {
  common::Logger* logger = common::Logger::instance();

  if (map_program_.size()) {
    logger->Print(common::LogType::ERR,
                  "ShaderCompiler::~ShaderCompiler: Please clear programs to "
                  "destroy this.");
    //assert(false);
  }
}

void ShaderCompiler::ShaderCompileAndAttachProgram(
    int program_key, const QString& vertex_path, const QString& fragment_path) {
  common::Logger* logger = common::Logger::instance();

  if (map_program_.find(program_key) != map_program_.end()) {
    logger->Print(common::LogType::ERR,
                  "ShaderCompiler::ShaderCompileAndAttachProgram: The compiler "
                  "already has program.");
    //assert(false);
    return;
  }

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    logger->Print(common::LogType::ERR,
                  "ShaderCompiler::ShaderCompileAndAttachProgram: Please check "
                  "the status of opengl.");
    //assert(false);
    return;
  }

  GLuint program;
  WGLSLprogram::createShaderProgram(vertex_path, fragment_path, program);

  map_program_[program_key] = (unsigned int)program;
}

unsigned int ShaderCompiler::GetProgramID(int program_key) {
  common::Logger* logger = common::Logger::instance();

  auto iter = map_program_.find(program_key);
  if (iter == map_program_.end()) {
    logger->Print(common::LogType::ERR,
                  "ShaderCompiler::GetProgramID: The key is invalid..");
    assert(false);
    return 0;
  }

  return iter->second;
}

void ShaderCompiler::ClearPrograms() {
  common::Logger* logger = common::Logger::instance();

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    logger->Print(
        common::LogType::ERR,
        "ShaderCompiler::ClearPrograms: Please check the status of opengl.");
    //assert(false);
    return;
  }

  for (auto iter = map_program_.begin(); iter == map_program_.end(); iter++) {
    glUseProgram(0);
    glDeleteProgram(iter->second);
  }
  map_program_.clear();
}
