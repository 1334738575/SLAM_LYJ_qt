#include "QT_LYJ.h"

#include <Eigen/Core>

#include <iostream>
#include <string>
#include <vector>

namespace
{
COMMON_LYJ::BaseTriMesh makeMesh()
{
	COMMON_LYJ::BaseTriMesh mesh;
	mesh.setVertexs({
		Eigen::Vector3f(-0.8f, -0.6f, 2.0f),
		Eigen::Vector3f(0.8f, -0.6f, 2.0f),
		Eigen::Vector3f(0.0f, 0.8f, 2.0f),
		Eigen::Vector3f(0.0f, 0.0f, 3.0f),
	});
	std::vector<COMMON_LYJ::BaseTriFace> faces(4);
	const unsigned int indices[4][3] = {
		{ 0, 2, 1 }, { 0, 1, 3 }, { 1, 2, 3 }, { 2, 0, 3 }
	};
	for (size_t i = 0; i < faces.size(); ++i)
		for (int j = 0; j < 3; ++j)
			faces[i].vId_[j] = indices[i][j];
	mesh.setFaces(faces);
	return mesh;
}

bool runModel(QT_LYJ::ProjectorBackend backend, QT_LYJ::ProjectorCameraModel model)
{
	const std::vector<double> parameters = model == QT_LYJ::ProjectorCameraModel::Fisheye
		? std::vector<double>{ 160.0, 160.0, 128.0, 128.0, 0.01, -0.005, 0.001, 0.0 }
		: std::vector<double>{ 160.0, 160.0, 128.0, 128.0 };
	QT_LYJ::ProjectorCamera camera(model, 256, 256, parameters);
	COMMON_LYJ::Pose3D pose;
	std::vector<COMMON_LYJ::BitFlagVec> visibility;
	QT_LYJ::ProjectionOptions options;
	options.backend = backend;
	options.minDepth = 0.1f;
	options.maxDepth = 10.0f;
	options.normalCosineThreshold = 1.1f;
	std::string error;
	if (!QT_LYJ::projectMeshVisibility(makeMesh(), { pose }, { camera }, visibility, options, &error))
	{
		std::cerr << error << std::endl;
		return false;
	}
	int visibleCount = 0;
	for (int i = 0; i < visibility[0].size(); ++i)
		visibleCount += visibility[0][i] ? 1 : 0;
	std::cout << (model == QT_LYJ::ProjectorCameraModel::Fisheye ? "fisheye" : "pinhole")
		<< " visible points=" << visibleCount << std::endl;
	return visibleCount > 0;
}
}

int main(int argc, char** argv)
{
	if (argc != 2)
		return 2;
	const std::string backendName = argv[1];
	QT_LYJ::ProjectorBackend backend;
	if (backendName == "cuda")
		backend = QT_LYJ::ProjectorBackend::CUDA;
	else if (backendName == "vulkan")
		backend = QT_LYJ::ProjectorBackend::Vulkan;
	else
		return 2;
	return runModel(backend, QT_LYJ::ProjectorCameraModel::Pinhole) &&
		runModel(backend, QT_LYJ::ProjectorCameraModel::Fisheye) ? 0 : 1;
}
