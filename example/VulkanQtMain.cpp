#include <QT_LYJ.h>

#include <IO/MeshIO.h>
#include <IO/SimpleIO.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
namespace fs = std::filesystem;

constexpr const char* kDefaultDataRoot = "C:/Users/DELL/Desktop/code2/codex7/tex";
constexpr const char* kDefaultPoseRoot = "C:/Users/DELL/Desktop/code2/codex7/tex";
constexpr const char* kDefaultMeshRoot =
    "C:/Users/DELL/Desktop/code2/codex7/tex/texture_fusion_result_optimized_poses";
constexpr float kSceneScale = 1000.0f;

bool readCamera(const fs::path& path, std::vector<double>& parameters)
{
    std::ifstream input(path);
    parameters.resize(4);
    for (double& value : parameters)
    {
        std::string name;
        std::string equals;
        if (!(input >> name >> equals >> value) || equals != "=")
            return false;
    }
    return true;
}

size_t findFrameCount(const fs::path& dataRoot, const fs::path& poseRoot)
{
    size_t count = 0;
    while (fs::is_regular_file(dataRoot / (std::to_string(count) + ".jpg")) &&
           fs::is_regular_file(dataRoot / ("cam_" + std::to_string(count) + ".txt")) &&
           fs::is_regular_file(poseRoot / ("rt_" + std::to_string(count) + ".txt")))
    {
        ++count;
    }
    return count;
}

int testViewTextures(int argc, char* argv[])
{
    const bool directObjArgument = argc > 1 &&
        (fs::path(argv[1]).extension() == ".obj" || fs::path(argv[1]).extension() == ".OBJ");
    const fs::path dataRoot = !directObjArgument && argc > 1
        ? fs::path(argv[1]) : fs::path(kDefaultDataRoot);
    const fs::path poseRoot = !directObjArgument && argc > 3
        ? fs::path(argv[3]) : fs::path(kDefaultPoseRoot);
    fs::path meshPath = directObjArgument ? fs::path(argv[1])
        : (argc > 4 ? fs::path(argv[4]) : fs::path(kDefaultMeshRoot));
    if (fs::is_directory(meshPath))
    {
        const fs::path preferred = meshPath / "ret.obj";
        if (fs::is_regular_file(preferred))
            meshPath = preferred;
        else
        {
            for (const fs::directory_entry& entry : fs::directory_iterator(meshPath))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".obj")
                {
                    meshPath = entry.path();
                    break;
                }
            }
        }
    }
    if (!fs::is_regular_file(meshPath))
    {
        std::cerr << "Mesh not found: " << meshPath << std::endl;
        return 1;
    }

    const std::string meshExtension = meshPath.extension().string();
    if (meshExtension == ".obj" || meshExtension == ".OBJ")
        return QT_LYJ::testOBJ(meshPath.string());

    size_t frameCount = findFrameCount(dataRoot, poseRoot);
    if (argc > 2)
    {
        try
        {
            frameCount = std::min(frameCount, static_cast<size_t>(std::stoul(argv[2])));
        }
        catch (const std::exception&)
        {
            std::cerr << "Invalid frame limit: " << argv[2] << std::endl;
            return 1;
        }
    }
    if (frameCount == 0)
    {
        std::cerr << "No complete image/camera/pose frame set found for data " << dataRoot
                  << " and poses " << poseRoot << std::endl;
        return 1;
    }

    std::cout << "Loading mesh: " << meshPath << std::endl;
    COMMON_LYJ::BaseTriMesh mesh;
    COMMON_LYJ::readPLYMesh(meshPath.string(), mesh);
    if (mesh.getVn() == 0 || mesh.getFn() == 0)
    {
        std::cerr << "Mesh contains no vertices or faces" << std::endl;
        return 1;
    }
    for (Eigen::Vector3f& point : mesh.getVertexs())
        point /= kSceneScale;

    std::vector<COMMON_LYJ::Pose3D> poses(frameCount);
    std::vector<QT_LYJ::ProjectorCamera> cameras;
    std::vector<COMMON_LYJ::CompressedImage> images(frameCount);
    cameras.reserve(frameCount);

    std::cout << "Loading " << frameCount << " frames with poses from: " << poseRoot << std::endl;
    for (size_t index = 0; index < frameCount; ++index)
    {
        const std::string frameName = std::to_string(index);
        cv::Mat image = cv::imread((dataRoot / (frameName + ".jpg")).string(), cv::IMREAD_COLOR);
        if (image.empty())
        {
            std::cerr << "Failed to load image " << index << std::endl;
            return 1;
        }
        cv::pyrDown(image, image);
        if (!images[index].compressCVMat(image))
        {
            std::cerr << "Failed to compress image " << index << std::endl;
            return 1;
        }

        std::vector<double> cameraParameters;
        if (!readCamera(dataRoot / ("cam_" + frameName + ".txt"), cameraParameters))
        {
            std::cerr << "Failed to read camera " << index << std::endl;
            return 1;
        }
        for (double& parameter : cameraParameters)
            parameter /= 2.0;
        cameras.emplace_back(QT_LYJ::ProjectorCameraModel::Pinhole,
            image.cols, image.rows, cameraParameters);

        if (!COMMON_LYJ::readT34((poseRoot / ("rt_" + frameName + ".txt")).string(), poses[index]))
        {
            std::cerr << "Failed to read pose " << index << std::endl;
            return 1;
        }
        poses[index].gett() /= kSceneScale;

        if ((index + 1) % 25 == 0 || index + 1 == frameCount)
            std::cout << "Loaded " << index + 1 << '/' << frameCount << " frames" << std::endl;
    }

    return QT_LYJ::testTcws(argc, argv, mesh, poses, cameras, images,
        QT_LYJ::ProjectorBackend::OpenGL);
}
}

int main(int argc, char* argv[])
{
	return QT_LYJ::testQT(argc, argv);
}
