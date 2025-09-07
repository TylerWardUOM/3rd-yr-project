#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

class Shader {
public:
    // Program ID
    unsigned int ID = 0;

    // Build from file paths
    Shader(const char* vertexPath, const char* fragmentPath);

    // Use/activate the shader
    void use() const;

    // -------- Uniform setters (basic types) --------
    void setBool (const std::string& name, bool  value) const;
    void setInt  (const std::string& name, int   value) const;
    void setUInt (const std::string& name, unsigned int value) const;
    void setFloat(const std::string& name, float value) const;

    // -------- Uniform setters (vectors) --------
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;

    // glm overloads
    void setVec2(const std::string& name, const glm::vec2& v) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;
    void setVec4(const std::string& name, const glm::vec4& v) const;

    // -------- Uniform setters (matrices) --------
    void setMat3(const std::string& name, const glm::mat3& m, bool transpose=false) const;
    void setMat4(const std::string& name, const glm::mat4& m, bool transpose=false) const;

    // -------- Arrays --------
    // Note: count = number of elements (not bytes)
    void setIntArray  (const std::string& name, const int*   data, int count) const;
    void setFloatArray(const std::string& name, const float* data, int count) const;
    void setVec3Array (const std::string& name, const glm::vec3* data, int count) const;
    void setMat4Array (const std::string& name, const glm::mat4* data, int count, bool transpose=false) const;

    // -------- Textures --------
    // Binds texture to 'unit' and sets sampler uniform to that unit
    void setTexture(const std::string& samplerName, GLuint texture, GLuint unit, GLenum target = GL_TEXTURE_2D) const;

private:
    // Lookup (and cache) uniform location
    GLint getLocation(const std::string& name) const;

    // Compile/link helpers
    static std::string loadFile(const char* path);
    static GLuint compile(GLenum type, const char* src, const char* debugName);
    static GLuint link(GLuint vs, GLuint fs);
    

    // Mutable to allow caching in const setters
    mutable std::unordered_map<std::string, GLint> uniformCache_;
    mutable std::unordered_map<std::string, bool>  warnedMissing_;
};

#endif // SHADER_H
