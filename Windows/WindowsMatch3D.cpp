#include "WindowsMatch3D.h"

namespace QT_LYJ
{
	WindowsMatch3D::WindowsMatch3D(QWidget* parent)
		:WindowsLyj(parent)
	{
		openGLWidget_->setPrintFunc([&](const std::string& _info) {
			printLog(_info);
			});
		//bottons
		addBotton("load module", [&]() {
			openGLWidget_->makeCurrent();
			// 释放关键资源（可选）
			openGLWidget_->doneCurrent();
			// 延迟触发对话框
			QTimer::singleShot(0, this, [&]() {
				QString directoryPath = QFileDialog::getExistingDirectory(nullptr, "Open directory", "../QT/data");
				if (!directoryPath.isEmpty()) {
					printLog("load module in " + directoryPath.toStdString());
					loadModel(directoryPath);
				}
				//loadModel("../QT/data/3D");
			});
		});
	}
	WindowsMatch3D::~WindowsMatch3D()
	{}

	void WindowsMatch3D::loadModel(const QString _path)
	{
		//// 初始化一些三维点
		//std::vector<QVector3D> points;
		//points.push_back(QVector3D(0.0f, 0.0f, 0.0f));
		//points.push_back(QVector3D(0.5f, 0.0f, 0.1f));
		//points.push_back(QVector3D(0.0f, 1.0f, 0.0f));
		//points.push_back(QVector3D(0.0f, 0.0f, 1.0f));
		//points.push_back(QVector3D(1.0f, 1.0f, 1.0f));
		//openGLWidget_->setPoints(points);
		//openGLWidget_->addLine(0, 1);
		//openGLWidget_->addTriangle(0, 1, 2);
		//openGLWidget_->addQuad(0, 1, 2, 3);
		//std::vector<int> polygon = { 0, 1, 2, 3, 4 };
		//openGLWidget_->addPolygon(polygon);

		int& iter = openGLWidget_->m_iter;
		int& frameSize = openGLWidget_->m_frameSize;
		int& enableNormal = openGLWidget_->m_enableNormal;
		int& enableColor = openGLWidget_->m_enableColor;
		std::vector<int>& allPsSize = openGLWidget_->m_allPsSize;
		std::vector<std::vector<Eigen::Vector3f>>& allPoints = openGLWidget_->m_allPoints;
		std::vector<std::vector<Eigen::Vector3f>>& allNormals = openGLWidget_->m_allNormals;
		std::vector<std::vector<Eigen::Vector3f>>& allColors = openGLWidget_->m_allColors;
		//std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>>& allCorrs = m_allCorrs;
		std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>>& allCorrs = openGLWidget_->m_allCorrs;
		std::vector<std::vector<Eigen::Matrix3f>>& allFrameRwcs = openGLWidget_->m_allFrameRwcs;
		std::vector<std::vector<Eigen::Vector3f>>& allFrametwcs = openGLWidget_->m_allFrametwcs;
		//int& iter = m_iter;
		//int& frameSize = m_frameSize;
		//int& enableNormal = m_enableNormal;
		//int& enableColor = m_enableColor;
		//std::vector<int>& allPsSize = m_allPsSize;
		//std::vector<std::vector<Eigen::Vector3f>>& allPoints = m_allPoints;
		//std::vector<std::vector<Eigen::Vector3f>>& allNormals = m_allNormals;
		//std::vector<std::vector<Eigen::Vector3f>>& allColors = m_allColors;
		//std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>>& allCorrs = m_allCorrs;
		//std::vector<std::vector<Eigen::Matrix3f>>& allFrameRwcs = m_allFrameRwcs;
		//std::vector<std::vector<Eigen::Vector3f>>& allFrametwcs = m_allFrametwcs;

		QDir dir(_path);
		QFileInfoList pcFiles = dir.entryInfoList(QStringList() << "PCs.txt", QDir::Files);
		QFileInfoList mFiles = dir.entryInfoList(QStringList() << "Matches*.txt", QDir::Files);
		QFileInfoList TFiles = dir.entryInfoList(QStringList() << "Twcs*.txt", QDir::Files);
		if (pcFiles.isEmpty() || mFiles.isEmpty() || TFiles.isEmpty()) {
			printLog("No PCs.txt or Matches.txt found in the directory.");
			return;
		}

		std::string logStr = "Loading model from: \n";
		std::string pcFilePath = pcFiles[0].absoluteFilePath().toStdString();
		logStr += pcFilePath + "\n";
		std::vector<std::string> mFilePaths(mFiles.size());
		for (int i = 0; i < mFiles.size(); ++i) {
			mFilePaths[i] = mFiles[i].absoluteFilePath().toStdString();
			logStr += mFilePaths[i] + "\n";
		}
		std::vector<std::string> TFilePaths(TFiles.size());
		for (int i = 0; i < TFiles.size(); ++i) {
			TFilePaths[i] = TFiles[i].absoluteFilePath().toStdString();
			logStr += TFilePaths[i] + "\n";
		}
		printLog(logStr);

		{
			std::ifstream pcf(pcFilePath);
			std::string header = "";
			int pointSize = 0;
			pcf >> header;
			pcf >> header >> frameSize;
			allPsSize.resize(frameSize + 1, 0);
			allPoints.resize(frameSize);
			allNormals.resize(frameSize);
			allColors.resize(frameSize);
			pcf >> header >> enableNormal;
			pcf >> header >> enableColor;
			pcf >> header >> iter;
			pcf >> header;
			for (int i = 0; i < frameSize; ++i) {
				pcf >> pointSize;
				allPsSize[i + 1] = pointSize;
				allPoints[i].resize(pointSize);
				allNormals[i].resize(pointSize);
				allColors[i].resize(pointSize);
				for (int j = 0; j < pointSize; ++j) {
					pcf >> allPoints[i][j](0) >> allPoints[i][j](1) >> allPoints[i][j](2);
					if (enableNormal)
						pcf >> allNormals[i][j](0) >> allNormals[i][j](1) >> allNormals[i][j](2);
					if (enableColor)
						pcf >> allColors[i][j](0) >> allColors[i][j](1) >> allColors[i][j](2);
				}
			}
			pcf.close();
		}
		for (int i = 1; i < allPsSize.size(); ++i)
			allPsSize[i] += allPsSize[i - 1];

		allFrameRwcs.resize(iter);
		allFrametwcs.resize(iter);
		for (int i = 0; i < iter; ++i) {
			std::ifstream Tf(TFilePaths[i]);
			int frameSize = 0;
			std::string header = "";
			Tf >> header;
			Tf >> header >> frameSize;
			allFrameRwcs[i].resize(frameSize);
			allFrametwcs[i].resize(frameSize);
			Tf >> header;
			for (int ii = 0; ii < frameSize; ++ii) {
				Tf >> allFrameRwcs[i][ii](0, 0) >> allFrameRwcs[i][ii](0, 1) >> allFrameRwcs[i][ii](0, 2)
					>> allFrameRwcs[i][ii](1, 0) >> allFrameRwcs[i][ii](1, 1) >> allFrameRwcs[i][ii](1, 2)
					>> allFrameRwcs[i][ii](2, 0) >> allFrameRwcs[i][ii](2, 1) >> allFrameRwcs[i][ii](2, 2);
				Tf >> allFrametwcs[i][ii](0) >> allFrametwcs[i][ii](1) >> allFrametwcs[i][ii](2);
			}
			Tf.close();
		}

		allCorrs.resize(iter);
		for (int i = 0; i < iter; ++i)
		{
			std::ifstream mf(mFilePaths[i]);
			std::string header = "";
			int64_t id64 = 0;
			int framePair = 0, frameId1, frameId2, matchSize = 0;
			mf >> header;
			mf >> header >> framePair;
			mf >> header;
			for (int ii = 0; ii < framePair; ++ii) {
				mf >> frameId1 >> frameId2 >> matchSize;
				id64 = imagePair2Int64(frameId1, frameId2);
				allCorrs[i][id64].resize(matchSize);
				for (int j = 0; j < matchSize; ++j)
					mf >> allCorrs[i][id64][j](0) >> allCorrs[i][id64][j](1);
			}
			mf.close();
		}

		//for (const auto& crs : allCorrs)
		//{
		//	openGLWidget_->m_allPairs.push_back(std::vector<int64_t>());
		//	openGLWidget_->m_allCorrs.push_back(std::vector<std::vector<Eigen::Vector2i>>());
		//	for (const auto& cr : crs)
		//	{
		//		openGLWidget_->m_allPairs.back().push_back(cr.first);
		//		openGLWidget_->m_allCorrs.back().push_back(cr.second);
		//	}
		//}

		//openGLWidget_->setMatchData(m_iter, m_frameSize, m_enableNormal, m_enableColor,
		//	&m_allPsSize, &m_allPoints, &m_allNormals, &m_allColors,
		//	&m_allCorrs, &m_allFrameRwcs, &m_allFrametwcs);

		return;
	}
}