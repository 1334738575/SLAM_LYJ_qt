#ifndef QT_LYJ_H
#define QT_LYJ_H

#include <iostream>
#include "QT_LYJ_Defines.h"
#include <IO/DataWin2D.h>
#include <common/BaseTriMesh.h>
#include <base/Pose.h>
#include <base/CameraModule.h>
#include <common/CompressedImage.h>
#include <array>
#include <cfloat>
#include <string>
#include <vector>


NSP_QT_LYJ_BEGIN

enum class ProjectorBackend
{
	CUDA,
	Vulkan
};

enum class ProjectorCameraModel
{
	Pinhole,
	Fisheye
};

struct QT_LYJ_API ProjectorCamera
{
	ProjectorCamera() = default;
	explicit ProjectorCamera(const COMMON_LYJ::PinholeCamera& camera);
	ProjectorCamera(ProjectorCameraModel cameraModel, int width, int height,
		const std::vector<double>& cameraParameters);

	ProjectorCameraModel model = ProjectorCameraModel::Pinhole;
	int width = 0;
	int height = 0;
	std::array<float, 8> parameters{};
};

struct QT_LYJ_API ProjectionOptions
{
	ProjectorBackend backend = ProjectorBackend::CUDA;
	float minDepth = 0.0f;
	float maxDepth = FLT_MAX;
	float normalCosineThreshold = 0.5f;
	float visibilityDepthThreshold = 0.01f;
};

QT_LYJ_API int testQT(int argc, char* argv[]);
QT_LYJ_API int testOpenGLOnly();

QT_LYJ_API bool projectMeshVisibility(
	const COMMON_LYJ::BaseTriMesh& mesh,
	const std::vector<COMMON_LYJ::Pose3D>& Tcws,
	const std::vector<ProjectorCamera>& cameras,
	std::vector<COMMON_LYJ::BitFlagVec>& pointVisibility,
	const ProjectionOptions& options = ProjectionOptions(),
	std::string* errorMessage = nullptr);

QT_LYJ_API void debugWindows(int argc, char* argv[]); 
QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& _btm,
	const std::vector<COMMON_LYJ::Pose3D>& _Tcws, const std::vector<COMMON_LYJ::PinholeCamera>& _cams, const std::vector<COMMON_LYJ::CompressedImage>& _comImgs);
QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& mesh,
	const std::vector<COMMON_LYJ::Pose3D>& Tcws,
	const std::vector<ProjectorCamera>& cameras,
	const std::vector<COMMON_LYJ::CompressedImage>& compressedImages,
	ProjectorBackend backend);

NSP_QT_LYJ_END

#endif // QT_LYJ_H
