#include "WindowsMatch3D.h"

namespace QT_LYJ
{
	WindowsMatch3D::WindowsMatch3D(QWidget* parent)
		:WindowsLyj(parent)
	{
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
		QFileInfoList mFiles = dir.entryInfoList(QStringList() << "Matches.txt", QDir::Files);
		if (pcFiles.isEmpty() || mFiles.isEmpty()) {
			printLog("No PCs.txt or Matches.txt found in the directory.");
			return;
		}

		std::string logStr = "Loading model from: \n";
		std::string pcFilePath = pcFiles[0].absoluteFilePath().toStdString();
		logStr += pcFilePath + "\n";
		std::string mFilePath = mFiles[0].absoluteFilePath().toStdString();
		logStr += mFilePath + "\n";
		printLog(logStr);

		std::vector<int> psSize;
		{
			std::ifstream pcf(pcFilePath);
			std::vector<float> points;
			std::string header = "";
			int frameSize = 0, flagNormal = 0, flagColor = 0, dataDim = 3, pointSize = 0;
			float x, y, z, nx, ny, nz, r, g, b;
			pcf >> header;
			pcf >> header >> frameSize;
			psSize.resize(frameSize + 1, 0);
			pcf >> header >> flagNormal;
			if (flagNormal)
				dataDim += 3;
			pcf >> header >> flagColor;
			if (flagColor)
				dataDim += 3;
			pcf >> header;
			for (int i = 0; i < frameSize; ++i) {
				pcf >> pointSize;
				psSize[i + 1] = pointSize;
				for (int j = 0; j < pointSize; ++j) {
					pcf >> x >> y >> z;
					points.push_back(x);
					points.push_back(y);
					points.push_back(z);
					if (flagNormal) {
						pcf >> nx >> ny >> nz;
						//points.push_back(nx);
						//points.push_back(ny);
						//points.push_back(nz);
					}
					if (flagColor) {
						pcf >> r >> g >> b;
						//points.push_back(r);
						//points.push_back(g);
						//points.push_back(b);
					}
				}
			}
			openGLWidget_->setPoints(points.data(), points.size() / 3);
			pcf.close();
		}
		for (int i = 1; i < psSize.size(); ++i)
			psSize[i] += psSize[i - 1];
		{
			std::ifstream mf(mFilePath);
			std::vector<int> lines;
			std::string header = "";
			int framePair = 0, frameId1, frameId2, matchSize = 0, mId1, mId2;
			mf >> header;
			mf >> header >> framePair;
			mf >> header;
			for (int i = 0; i < framePair; ++i) {
				mf >> frameId1 >> frameId2 >> matchSize;
				for (int j = 0; j < matchSize; ++j) {
					mf >> mId1 >> mId2;
					lines.push_back(mId1 + psSize[frameId1]);
					lines.push_back(mId2 + psSize[frameId2]);
				}
			}
			openGLWidget_->setLines(lines.data(), lines.size() / 2);
			mf.close();
		}
	}
}