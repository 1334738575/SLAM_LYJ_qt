#ifndef QT_LYJ_OPENGL_PROJECTOR_H
#define QT_LYJ_OPENGL_PROJECTOR_H

#include <array>
#include <string>
#include <vector>

namespace QT_LYJ
{

struct OpenGLProjectorCamera
{
	int width = 0;
	int height = 0;
	int cameraModel = 0; // 0: pinhole, 1: fisheye
	std::array<float, 8> parameters{};
};

class OpenGLProjector
{
public:
	OpenGLProjector();
	~OpenGLProjector();

	OpenGLProjector(const OpenGLProjector&) = delete;
	OpenGLProjector& operator=(const OpenGLProjector&) = delete;

	bool initialize(const float* vertices, unsigned int vertexCount,
		const float* faceNormals, const unsigned int* faces, unsigned int faceCount,
		std::string* errorMessage = nullptr);

	bool project(const float* Tcw, const OpenGLProjectorCamera& camera,
		float minDepth, float maxDepth, float normalCosineThreshold,
		float visibilityDepthThreshold, std::vector<float>& depths,
		std::vector<unsigned int>& faceIds, std::vector<char>& visiblePoints,
		std::vector<char>& visibleFaces, std::string* errorMessage = nullptr);

private:
		class Impl;
		Impl* impl_ = nullptr;
};

}

#endif
