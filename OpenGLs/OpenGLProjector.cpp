#include "OpenGLProjector.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QSurfaceFormat>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>

namespace QT_LYJ
{
namespace
{
	const char* kVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat3 uRotation;
uniform vec3 uTranslation;
uniform vec4 uCamera;
uniform vec4 uDistortion;
uniform vec2 uImageSize;
uniform float uMinDepth;
uniform float uMaxDepth;
uniform int uCameraModel;

flat out vec3 transformedNormal;
out float cameraDepth;

vec2 projectPoint(vec3 point)
{
    float x = point.x / point.z;
    float y = point.y / point.z;
    if (uCameraModel == 1) {
        float r = length(vec2(x, y));
        if (r < 1e-12)
            return uCamera.zw;
        float theta = atan(r);
        float theta2 = theta * theta;
        float theta4 = theta2 * theta2;
        float theta6 = theta4 * theta2;
        float theta8 = theta4 * theta4;
        float thetaD = theta * (1.0 + uDistortion.x * theta2 +
            uDistortion.y * theta4 + uDistortion.z * theta6 + uDistortion.w * theta8);
        float scale = thetaD / r;
        return vec2(uCamera.x * x * scale + uCamera.z,
                    uCamera.y * y * scale + uCamera.w);
    }
    return vec2(x * uCamera.x + uCamera.z, y * uCamera.y + uCamera.w);
}

void main()
{
    vec3 point = uRotation * position + uTranslation;
    cameraDepth = point.z;
    transformedNormal = uRotation * normal;
    if (point.z <= uMinDepth || point.z > uMaxDepth) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return;
    }
    vec2 uv = projectPoint(point);
    gl_Position = vec4(2.0 * uv.x / uImageSize.x - 1.0,
                       2.0 * uv.y / uImageSize.y - 1.0,
                       2.0 * point.z / uMaxDepth - 1.0, 1.0);
}
)GLSL";

	const char* kFragmentShader = R"GLSL(
#version 330 core
flat in vec3 transformedNormal;
in float cameraDepth;

uniform float uNormalCosineThreshold;
uniform float uMaxDepth;

layout(location = 0) out vec4 result;

void main()
{
    if (transformedNormal.z >= uNormalCosineThreshold)
        discard;
    result = vec4(cameraDepth, float(gl_PrimitiveID + 1), 0.0, 1.0);
    gl_FragDepth = clamp(cameraDepth / uMaxDepth, 0.0, 1.0);
}
)GLSL";

	bool setError(std::string* errorMessage, const std::string& message)
	{
		if (errorMessage)
			*errorMessage = message;
		return false;
	}

	bool projectCameraPoint(const float* Tcw, const OpenGLProjectorCamera& camera,
		const float* point, float& u, float& v, float& depth)
	{
		const float x = Tcw[0] * point[0] + Tcw[3] * point[1] + Tcw[6] * point[2] + Tcw[9];
		const float y = Tcw[1] * point[0] + Tcw[4] * point[1] + Tcw[7] * point[2] + Tcw[10];
		depth = Tcw[2] * point[0] + Tcw[5] * point[1] + Tcw[8] * point[2] + Tcw[11];
		if (!(depth > 0.0f) || !std::isfinite(depth))
			return false;

		const float xn = x / depth;
		const float yn = y / depth;
		float scale = 1.0f;
		if (camera.cameraModel == 1)
		{
			const float r = std::sqrt(xn * xn + yn * yn);
			if (r > 1e-12f)
			{
				const float theta = std::atan(r);
				const float theta2 = theta * theta;
				const float theta4 = theta2 * theta2;
				const float theta6 = theta4 * theta2;
				const float theta8 = theta4 * theta4;
				const float thetaD = theta * (1.0f + camera.parameters[4] * theta2 +
					camera.parameters[5] * theta4 + camera.parameters[6] * theta6 +
					camera.parameters[7] * theta8);
				scale = thetaD / r;
			}
		}
		u = camera.parameters[0] * xn * scale + camera.parameters[2];
		v = camera.parameters[1] * yn * scale + camera.parameters[3];
		return std::isfinite(u) && std::isfinite(v);
	}
}

class OpenGLProjector::Impl
{
public:
	std::unique_ptr<QGuiApplication> ownedApplication;
	QOpenGLContext context;
	QOffscreenSurface surface;
	QOpenGLFunctions_3_3_Core* gl = nullptr;
	GLuint program = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;
	GLuint colorTexture = 0;
	GLuint depthBuffer = 0;
	GLuint framebuffer = 0;
	unsigned int vertexCount = 0;
	unsigned int faceCount = 0;
	std::vector<float> vertices;
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	bool initialized = false;

	~Impl()
	{
		if (!context.isValid() || !context.makeCurrent(&surface))
			return;
		if (gl)
		{
			if (framebuffer) gl->glDeleteFramebuffers(1, &framebuffer);
			if (depthBuffer) gl->glDeleteRenderbuffers(1, &depthBuffer);
			if (colorTexture) gl->glDeleteTextures(1, &colorTexture);
			if (ebo) gl->glDeleteBuffers(1, &ebo);
			if (vbo) gl->glDeleteBuffers(1, &vbo);
			if (vao) gl->glDeleteVertexArrays(1, &vao);
			if (program) gl->glDeleteProgram(program);
		}
		context.doneCurrent();
	}

	bool makeContext(std::string* errorMessage)
	{
		if (initialized)
			return true;
		if (!QCoreApplication::instance())
		{
			static int applicationArgc = 1;
			static char applicationName[] = "QT_LYJ_OpenGLProjector";
			static char* applicationArgv[] = { applicationName, nullptr };
			ownedApplication = std::make_unique<QGuiApplication>(applicationArgc, applicationArgv);
		}
		QSurfaceFormat format;
		format.setRenderableType(QSurfaceFormat::OpenGL);
		format.setVersion(3, 3);
		format.setProfile(QSurfaceFormat::CoreProfile);
		surface.setFormat(format);
		surface.create();
		if (!surface.isValid())
			return setError(errorMessage, "failed to create OpenGL offscreen surface");
		context.setFormat(format);
		if (!context.create())
			return setError(errorMessage, "failed to create OpenGL context");
		if (!context.makeCurrent(&surface))
			return setError(errorMessage, "failed to make OpenGL context current");
		gl = context.versionFunctions<QOpenGLFunctions_3_3_Core>();
		if (!gl || !gl->initializeOpenGLFunctions())
		{
			context.doneCurrent();
			return setError(errorMessage, "OpenGL 3.3 core functions are unavailable");
		}
		initialized = true;
		return true;
	}

	GLuint compile(GLenum type, const char* source, std::string* errorMessage)
	{
		GLuint shader = gl->glCreateShader(type);
		gl->glShaderSource(shader, 1, &source, nullptr);
		gl->glCompileShader(shader);
		GLint status = GL_FALSE;
		gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_TRUE)
			return shader;
		GLint length = 0;
		gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		std::string log(static_cast<size_t>(std::max(1, length)), '\0');
		gl->glGetShaderInfoLog(shader, length, nullptr, log.data());
		gl->glDeleteShader(shader);
		setError(errorMessage, "OpenGL projector shader compilation failed: " + log);
		return 0;
	}

	bool createProgram(std::string* errorMessage)
	{
		const GLuint vertex = compile(GL_VERTEX_SHADER, kVertexShader, errorMessage);
		if (!vertex)
			return false;
		const GLuint fragment = compile(GL_FRAGMENT_SHADER, kFragmentShader, errorMessage);
		if (!fragment)
		{
			gl->glDeleteShader(vertex);
			return false;
		}
		program = gl->glCreateProgram();
		gl->glAttachShader(program, vertex);
		gl->glAttachShader(program, fragment);
		gl->glLinkProgram(program);
		gl->glDeleteShader(vertex);
		gl->glDeleteShader(fragment);
		GLint status = GL_FALSE;
		gl->glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == GL_TRUE)
			return true;
		GLint length = 0;
		gl->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		std::string log(static_cast<size_t>(std::max(1, length)), '\0');
		gl->glGetProgramInfoLog(program, length, nullptr, log.data());
		return setError(errorMessage, "OpenGL projector shader linking failed: " + log);
	}

	bool createFramebuffer(int width, int height, std::string* errorMessage)
	{
		if (width == framebufferWidth && height == framebufferHeight && framebuffer)
			return true;
		if (framebuffer)
		{
			gl->glDeleteFramebuffers(1, &framebuffer);
			gl->glDeleteRenderbuffers(1, &depthBuffer);
			gl->glDeleteTextures(1, &colorTexture);
			framebuffer = depthBuffer = colorTexture = 0;
		}
		gl->glGenFramebuffers(1, &framebuffer);
		gl->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		gl->glGenTextures(1, &colorTexture);
		gl->glBindTexture(GL_TEXTURE_2D, colorTexture);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
		gl->glGenRenderbuffers(1, &depthBuffer);
		gl->glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
		gl->glDrawBuffers(1, &drawBuffer);
		const bool complete = gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (!complete)
			return setError(errorMessage, "OpenGL projector framebuffer is incomplete");
		framebufferWidth = width;
		framebufferHeight = height;
		return true;
	}
};

OpenGLProjector::OpenGLProjector() : impl_(new Impl) {}
OpenGLProjector::~OpenGLProjector() { delete impl_; }

bool OpenGLProjector::initialize(const float* vertices, unsigned int vertexCount,
	const float* faceNormals, const unsigned int* faces, unsigned int faceCount,
	std::string* errorMessage)
{
	if (!vertices || !faceNormals || !faces || vertexCount == 0 || faceCount == 0)
		return setError(errorMessage, "OpenGL projector received an empty mesh");
	if (!impl_->makeContext(errorMessage))
		return false;
	if (!impl_->createProgram(errorMessage))
		return false;

	struct Vertex { float position[3]; float normal[3]; };
	std::vector<Vertex> data(vertexCount);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		std::copy_n(vertices + static_cast<size_t>(i) * 3, 3, data[i].position);
		// Face normals are supplied per face; the vertex shader uses the first
		// face normal for each triangle through a duplicated indexed stream below.
		data[i].normal[0] = data[i].normal[1] = data[i].normal[2] = 0.0f;
	}

	// Use a non-indexed stream so each triangle can carry its own flat normal.
	data.clear();
	data.reserve(static_cast<size_t>(faceCount) * 3);
	std::vector<unsigned int> drawIndices(static_cast<size_t>(faceCount) * 3);
	for (unsigned int face = 0; face < faceCount; ++face)
	{
		for (unsigned int corner = 0; corner < 3; ++corner)
		{
			const unsigned int vertex = faces[static_cast<size_t>(face) * 3 + corner];
			if (vertex >= vertexCount)
				return setError(errorMessage, "OpenGL projector mesh contains an invalid face index");
			Vertex item{};
			std::copy_n(vertices + static_cast<size_t>(vertex) * 3, 3, item.position);
			std::copy_n(faceNormals + static_cast<size_t>(face) * 3, 3, item.normal);
			data.push_back(item);
			drawIndices[static_cast<size_t>(face) * 3 + corner] =
				static_cast<unsigned int>(data.size() - 1);
		}
	}

	impl_->gl->glGenVertexArrays(1, &impl_->vao);
	impl_->gl->glGenBuffers(1, &impl_->vbo);
	impl_->gl->glGenBuffers(1, &impl_->ebo);
	impl_->gl->glBindVertexArray(impl_->vao);
	impl_->gl->glBindBuffer(GL_ARRAY_BUFFER, impl_->vbo);
	impl_->gl->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(Vertex)), data.data(), GL_STATIC_DRAW);
	impl_->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, impl_->ebo);
	impl_->gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawIndices.size() * sizeof(unsigned int)), drawIndices.data(), GL_STATIC_DRAW);
	impl_->gl->glEnableVertexAttribArray(0);
	impl_->gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	impl_->gl->glEnableVertexAttribArray(1);
	impl_->gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	impl_->gl->glBindVertexArray(0);
	impl_->vertexCount = vertexCount;
	impl_->faceCount = faceCount;
	impl_->vertices.assign(vertices, vertices + static_cast<size_t>(vertexCount) * 3);
	impl_->context.doneCurrent();
	return true;
}

bool OpenGLProjector::project(const float* Tcw, const OpenGLProjectorCamera& camera,
	float minDepth, float maxDepth, float normalCosineThreshold,
	float visibilityDepthThreshold, std::vector<float>& depths,
	std::vector<unsigned int>& faceIds, std::vector<char>& visiblePoints,
	std::vector<char>& visibleFaces, std::string* errorMessage)
{
	if (!Tcw || !impl_->initialized || !impl_->vao)
		return setError(errorMessage, "OpenGL projector is not initialized");
	if (camera.width <= 0 || camera.height <= 0 || !std::isfinite(maxDepth) || maxDepth <= minDepth)
		return setError(errorMessage, "invalid OpenGL projector camera or depth range");
	if (!impl_->context.makeCurrent(&impl_->surface))
		return setError(errorMessage, "failed to make OpenGL projector context current");
	if (!impl_->createFramebuffer(camera.width, camera.height, errorMessage))
	{
		impl_->context.doneCurrent();
		return false;
	}

	QOpenGLFunctions_3_3_Core* gl = impl_->gl;
	gl->glBindFramebuffer(GL_FRAMEBUFFER, impl_->framebuffer);
	gl->glViewport(0, 0, camera.width, camera.height);
	const GLfloat clearColor[4] = { 0, 0, 0, 0 };
	gl->glClearBufferfv(GL_COLOR, 0, clearColor);
	gl->glClear(GL_DEPTH_BUFFER_BIT);
	gl->glEnable(GL_DEPTH_TEST);
	gl->glDepthFunc(GL_LESS);
	gl->glDisable(GL_CULL_FACE);
	gl->glUseProgram(impl_->program);
	const GLint rotationLocation = gl->glGetUniformLocation(impl_->program, "uRotation");
	const GLint translationLocation = gl->glGetUniformLocation(impl_->program, "uTranslation");
	const GLint cameraLocation = gl->glGetUniformLocation(impl_->program, "uCamera");
	const GLint distortionLocation = gl->glGetUniformLocation(impl_->program, "uDistortion");
	const GLint imageSizeLocation = gl->glGetUniformLocation(impl_->program, "uImageSize");
	const GLint minDepthLocation = gl->glGetUniformLocation(impl_->program, "uMinDepth");
	const GLint maxDepthLocation = gl->glGetUniformLocation(impl_->program, "uMaxDepth");
	const GLint modelLocation = gl->glGetUniformLocation(impl_->program, "uCameraModel");
	const GLint normalLocation = gl->glGetUniformLocation(impl_->program, "uNormalCosineThreshold");
	gl->glUniformMatrix3fv(rotationLocation, 1, GL_FALSE, Tcw);
	gl->glUniform3fv(translationLocation, 1, Tcw + 9);
	gl->glUniform4f(cameraLocation, camera.parameters[0], camera.parameters[1], camera.parameters[2], camera.parameters[3]);
	gl->glUniform4f(distortionLocation, camera.parameters[4], camera.parameters[5], camera.parameters[6], camera.parameters[7]);
	gl->glUniform2f(imageSizeLocation, static_cast<float>(camera.width), static_cast<float>(camera.height));
	gl->glUniform1f(minDepthLocation, minDepth);
	gl->glUniform1f(maxDepthLocation, maxDepth);
	gl->glUniform1i(modelLocation, camera.cameraModel);
	gl->glUniform1f(normalLocation, normalCosineThreshold);
	gl->glBindVertexArray(impl_->vao);
	gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(impl_->faceCount * 3), GL_UNSIGNED_INT, nullptr);
	gl->glFinish();

	std::vector<float> pixels(static_cast<size_t>(camera.width) * camera.height * 4);
	gl->glReadPixels(0, 0, camera.width, camera.height, GL_RGBA, GL_FLOAT, pixels.data());
	depths.assign(static_cast<size_t>(camera.width) * camera.height, std::numeric_limits<float>::max());
	faceIds.assign(depths.size(), std::numeric_limits<unsigned int>::max());
	visibleFaces.assign(impl_->faceCount, 0);
	for (size_t pixel = 0; pixel < depths.size(); ++pixel)
	{
		const float depth = pixels[pixel * 4];
		const float id = pixels[pixel * 4 + 1];
		if (depth > 0.0f && std::isfinite(depth) && id >= 1.0f)
		{
			depths[pixel] = depth;
			const unsigned int face = static_cast<unsigned int>(id + 0.5f) - 1;
			if (face < visibleFaces.size())
			{
				faceIds[pixel] = face;
				visibleFaces[face] = 1;
			}
		}
	}
	gl->glBindVertexArray(0);
	gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	impl_->context.doneCurrent();

	// The point test mirrors the CUDA/Vulkan projector's depth tolerance and
	// 3x3 neighborhood fallback, while the visibility buffer itself is produced
	// by OpenGL rasterization above.
	visiblePoints.assign(impl_->vertexCount, 0);
	for (unsigned int point = 0; point < impl_->vertexCount; ++point)
	{
		float u = 0.0f;
		float v = 0.0f;
		float pointDepth = 0.0f;
		if (!projectCameraPoint(Tcw, camera, &impl_->vertices[static_cast<size_t>(point) * 3], u, v, pointDepth) ||
			pointDepth < minDepth || pointDepth > maxDepth)
			continue;
		const int ui = static_cast<int>(u);
		const int vi = static_cast<int>(v);
		if (ui < 0 || ui >= camera.width || vi < 0 || vi >= camera.height)
			continue;
		float nearestDepth = std::numeric_limits<float>::max();
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				const int x = ui + dx;
				const int y = vi + dy;
				if (x >= 0 && x < camera.width && y >= 0 && y < camera.height)
					nearestDepth = std::min(nearestDepth, depths[static_cast<size_t>(y) * camera.width + x]);
			}
		}
		if (nearestDepth != std::numeric_limits<float>::max() &&
			std::abs(pointDepth - nearestDepth) <= visibilityDepthThreshold)
			visiblePoints[point] = 1;
	}
	return true;
}

}
