#include "viz/Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// ---------- File IO ----------
std::string Shader::loadFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "[Shader] Failed to open file: " << path << "\n";
        return {};
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ---------- Compile/Link ----------
GLuint Shader::compile(GLenum type, const char* src, const char* debugName) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint maxLen = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLen);

        std::vector<GLchar> log(static_cast<size_t>(maxLen));
        GLsizei written = 0;
        glGetShaderInfoLog(s, maxLen, &written, log.data());

        std::cerr << "[Shader] Compile error in " << debugName
            << ":\n" << std::string(log.data(), static_cast<size_t>(written)) << "\n";

        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint Shader::link(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint maxLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxLen);

        std::vector<GLchar> log(static_cast<size_t>(maxLen));
        GLsizei written = 0;
        glGetProgramInfoLog(prog, maxLen, &written, log.data());

        std::cerr << "[Shader] Link error:\n"
            << std::string(log.data(), static_cast<size_t>(written)) << "\n";

        glDeleteProgram(prog);
        return 0;
    }
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ---------- Ctor ----------
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    const std::string vsrc = loadFile(vertexPath);
    const std::string fsrc = loadFile(fragmentPath);
    if (vsrc.empty() || fsrc.empty()) { ID = 0; return; }

    GLuint vs = compile(GL_VERTEX_SHADER,   vsrc.c_str(), vertexPath);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc.c_str(), fragmentPath);
    if (!vs || !fs) { ID = 0; return; }

    ID = link(vs, fs);
    if (!ID) {
        std::cerr << "[Shader] Failed to link program.\n";
    }
}

// ---------- Use ----------
void Shader::use() const {
    glUseProgram(ID);
}

// ---------- Uniform location cache ----------
GLint Shader::getLocation(const std::string& name) const {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) return it->second;

    GLint loc = glGetUniformLocation(ID, name.c_str());
    uniformCache_.emplace(name, loc);

    if (loc == -1) {
        // Warn once
        if (!warnedMissing_[name]) {
            std::cerr << "[Shader] Warning: uniform '" << name << "' not found (or optimized out).\n";
            warnedMissing_[name] = true;
        }
    }
    return loc;
}

// ---------- Basic types ----------
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getLocation(name), value ? 1 : 0);
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getLocation(name), value);
}
void Shader::setUInt(const std::string& name, unsigned int value) const {
    glUniform1ui(getLocation(name), value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getLocation(name), value);
}

// ---------- Vectors ----------
void Shader::setVec2(const std::string& name, float x, float y) const {
    glUniform2f(getLocation(name), x, y);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(getLocation(name), x, y, z);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(getLocation(name), x, y, z, w);
}

void Shader::setVec2(const std::string& name, const glm::vec2& v) const {
    glUniform2fv(getLocation(name), 1, &v[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& v) const {
    glUniform3fv(getLocation(name), 1, &v[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& v) const {
    glUniform4fv(getLocation(name), 1, &v[0]);
}

// ---------- Matrices ----------
void Shader::setMat3(const std::string& name, const glm::mat3& m, bool transpose) const {
    glUniformMatrix3fv(getLocation(name), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}
void Shader::setMat4(const std::string& name, const glm::mat4& m, bool transpose) const {
    glUniformMatrix4fv(getLocation(name), 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(m));
}

// ---------- Arrays ----------
void Shader::setIntArray(const std::string& name, const int* data, int count) const {
    glUniform1iv(getLocation(name), count, data);
}
void Shader::setFloatArray(const std::string& name, const float* data, int count) const {
    glUniform1fv(getLocation(name), count, data);
}
void Shader::setVec3Array(const std::string& name, const glm::vec3* data, int count) const {
    glUniform3fv(getLocation(name), count, &data[0][0]);
}
void Shader::setMat4Array(const std::string& name, const glm::mat4* data, int count, bool transpose) const {
    glUniformMatrix4fv(getLocation(name), count, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(data[0]));
}

// ---------- Textures ----------
void Shader::setTexture(const std::string& samplerName, GLuint texture, GLuint unit, GLenum target) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, texture);
    // Set sampler to use this unit
    glUniform1i(getLocation(samplerName), static_cast<GLint>(unit));
}
