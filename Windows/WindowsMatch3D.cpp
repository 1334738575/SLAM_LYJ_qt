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
			QString directoryPath = QFileDialog::getExistingDirectory(this, "Open directory", "../QT/data");
			if (!directoryPath.isEmpty()) {
				printLog("load module in " + directoryPath.toStdString());
				loadModel(directoryPath);
			}
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
			pcf >> header >> m_frameSize;
			m_allPsSize.resize(m_frameSize + 1, 0);
			m_allPoints.resize(m_frameSize);
			m_allNormals.resize(m_frameSize);
			m_allColors.resize(m_frameSize);
			pcf >> header >> m_enableNormal;
			pcf >> header >> m_enableColor;
			pcf >> header >> m_iter;
			pcf >> header;
			for (int i = 0; i < m_frameSize; ++i) {
				pcf >> pointSize;
				m_allPsSize[i + 1] = pointSize;
				m_allPoints[i].resize(pointSize);
				m_allNormals[i].resize(pointSize);
				m_allColors[i].resize(pointSize);
				for (int j = 0; j < pointSize; ++j) {
					pcf >> m_allPoints[i][j](0) >> m_allPoints[i][j](1) >> m_allPoints[i][j](2);
					if (m_enableNormal)
						pcf >> m_allNormals[i][j](0) >> m_allNormals[i][j](1) >> m_allNormals[i][j](2);
					if (m_enableColor)
						pcf >> m_allColors[i][j](0) >> m_allColors[i][j](1) >> m_allColors[i][j](2);
				}
			}
			//openGLWidget_->setPoints(points.data(), points.size() / 3);
			pcf.close();
		}
		for (int i = 1; i < m_allPsSize.size(); ++i)
			m_allPsSize[i] += m_allPsSize[i - 1];

		m_allFrameRwcs.resize(m_iter);
		m_allFrametwcs.resize(m_iter);
		for (int i = 0; i < m_iter; ++i) {
			std::ifstream Tf(TFilePaths[i]);
			int frameSize = 0;
			std::string header = "";
			Tf >> header;
			Tf >> header >> frameSize;
			m_allFrameRwcs[i].resize(frameSize);
			m_allFrametwcs[i].resize(frameSize);
			Tf >> header;
			for (int ii = 0; ii < frameSize; ++ii) {
				Tf >> m_allFrameRwcs[i][ii](0, 0) >> m_allFrameRwcs[i][ii](0, 1) >> m_allFrameRwcs[i][ii](0, 2)
					>> m_allFrameRwcs[i][ii](1, 0) >> m_allFrameRwcs[i][ii](1, 1) >> m_allFrameRwcs[i][ii](1, 2)
					>> m_allFrameRwcs[i][ii](2, 0) >> m_allFrameRwcs[i][ii](2, 1) >> m_allFrameRwcs[i][ii](2, 2);
				Tf >> m_allFrametwcs[i][ii](0) >> m_allFrametwcs[i][ii](1) >> m_allFrametwcs[i][ii](2);
			}
			Tf.close();
		}

		m_allCorrs.resize(m_iter);
		for (int i = 0; i < m_iter; ++i)
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
				m_allCorrs[i][id64].resize(matchSize);
				for (int j = 0; j < matchSize; ++j)
					mf >> m_allCorrs[i][id64][j](0) >> m_allCorrs[i][id64][j](1);
			}
			//openGLWidget_->setLines(lines.data(), lines.size() / 2);
			mf.close();
		}

		openGLWidget_->setMatchData(m_iter, m_frameSize, m_enableNormal, m_enableColor,
			&m_allPsSize, &m_allPoints, &m_allNormals, &m_allColors,
			&m_allCorrs, &m_allFrameRwcs, &m_allFrametwcs);

		return;
	}
}