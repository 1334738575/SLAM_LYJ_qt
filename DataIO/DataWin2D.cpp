#include "DataWin2D.h"


namespace QT_LYJ
{

	Data2DPoint::Data2DPoint()
	{
		dataType = Data2DType::POINT2D;
	}
	Data2DPoint::~Data2DPoint()
	{
	}
	void Data2DPoint::write_binary(std::ofstream& os) const
	{
		{
			std::string header = "point2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			std::string header2 = "frameSize";
			COMMON_LYJ::writeBin<const std::string&>(os, header2);
			int imgSize = m_allKeyPoints.size();
			COMMON_LYJ::writeBin<const int&>(os, imgSize);
			std::string header3 = "data";
			COMMON_LYJ::writeBin<const std::string&>(os, header3);
			int imgId;
			int kpSize;
			for (int i = 0; i < imgSize; ++i)
			{
				const auto& kps = m_allKeyPoints[i];
				imgId = i;
				kpSize = kps.size();
				COMMON_LYJ::writeBin<const int&>(os, imgId);
				COMMON_LYJ::writeBin<const int&>(os, kpSize);
				for (int j = 0; j < kpSize; ++j)
				{
					const cv::Point& kp = kps[j];
					const int& x = kp.x;
					const int& y = kp.y;
					COMMON_LYJ::writeBinDatas<const int&, const int&>(os, x, y);
				}
			}
		}

		{
			std::string header = "pointmatch2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			writeMatches(os, m_allPointMatches);
		}
	}
	void Data2DPoint::read_binary(std::ifstream& is)
	{
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			std::string header2;
			COMMON_LYJ::readBin<std::string&>(is, header2);
			int imgSize;
			COMMON_LYJ::readBin<int&>(is, imgSize);
			std::string header3;
			COMMON_LYJ::readBin<std::string&>(is, header3);

			int imgId = 0;
			int pointSize = 0;
			m_allKeyPoints.resize(imgSize);
			for (int i = 0; i < imgSize; ++i) {
				COMMON_LYJ::readBin<int&>(is, imgId);
				COMMON_LYJ::readBin<int&>(is, pointSize);
				m_allKeyPoints[i].resize(pointSize);
				for (int j = 0; j < pointSize; ++j) {
					COMMON_LYJ::readBinDatas<int&, int&>(is, m_allKeyPoints[i][j].x, m_allKeyPoints[i][j].y);
				}
			}
		}
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			readMatches(is, m_allKPImgPairs, m_allPointMatches);
		}
	}




	Data2DLine::Data2DLine()
	{
		dataType = Data2DType::LINE2D;
	}
	Data2DLine::~Data2DLine()
	{
	}
	void Data2DLine::write_binary(std::ofstream& os) const
	{
		{
			std::string header = "line2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			std::string header2 = "frameSize";
			COMMON_LYJ::writeBin<const std::string&>(os, header2);
			int imgSize = m_allKeyLines.size();
			COMMON_LYJ::writeBin<const int&>(os, imgSize);
			std::string header3 = "data";
			COMMON_LYJ::writeBin<const std::string&>(os, header3);
			int imgId;
			int lSize;
			for (int i = 0; i < imgSize; ++i)
			{
				const auto& ls = m_allKeyLines[i];
				imgId = i;
				lSize = ls.size();
				COMMON_LYJ::writeBin<const int&>(os, imgId);
				COMMON_LYJ::writeBin<const int&>(os, lSize);
				for (int j = 0; j < lSize; ++j)
				{
					const cv::Vec4f& kp = ls[j];
					COMMON_LYJ::writeBinDatas<const float&, const float&, const float&, const float&>(os, kp[0], kp[1], kp[2], kp[3]);
				}
			}
		}

		{
			std::string header = "linematch2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			writeMatches(os, m_allLineMatches);
		}
	}
	void Data2DLine::read_binary(std::ifstream& is)
	{
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			std::string header2;
			COMMON_LYJ::readBin<std::string&>(is, header2);
			int imgSize;
			COMMON_LYJ::readBin<int&>(is, imgSize);
			std::string header3;
			COMMON_LYJ::readBin<std::string&>(is, header3);

			int imgId = 0;
			int lineSize = 0;
			m_allKeyLines.resize(imgSize);
			for (int i = 0; i < imgSize; ++i) {
				COMMON_LYJ::readBin<int&>(is, imgId);
				COMMON_LYJ::readBin<int&>(is, lineSize);
				m_allKeyLines[i].resize(lineSize);
				for (int j = 0; j < lineSize; ++j) {
					COMMON_LYJ::readBinDatas<float&, float&, float&, float&>(is, m_allKeyLines[i][j][0], m_allKeyLines[i][j][1], m_allKeyLines[i][j][2], m_allKeyLines[i][j][3]);
				}
			}
		}
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			readMatches(is, m_allKLImgPairs, m_allLineMatches);
		}
	}






	Data2DEdge::Data2DEdge()
	{
		dataType = Data2DType::EDGE2D;
	}
	Data2DEdge::~Data2DEdge()
	{
	}
	void Data2DEdge::write_binary(std::ofstream& os) const
	{
		{
			std::string header = "edge2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			std::string header2 = "frameSize";
			COMMON_LYJ::writeBin<const std::string&>(os, header2);
			int imgSize = m_allEdgePoints.size();
			COMMON_LYJ::writeBin<const int&>(os, imgSize);
			std::string header3 = "data";
			COMMON_LYJ::writeBin<const std::string&>(os, header3);
			int imgId;
			int kpSize;
			for (int i = 0; i < imgSize; ++i)
			{
				const auto& kps = m_allEdgePoints[i];
				imgId = i;
				kpSize = kps.size();
				COMMON_LYJ::writeBin<const int&>(os, imgId);
				COMMON_LYJ::writeBin<const int&>(os, kpSize);
				for (int j = 0; j < kpSize; ++j)
				{
					const cv::Point& kp = kps[j];
					const int& x = kp.x;
					const int& y = kp.y;
					COMMON_LYJ::writeBinDatas<const int&, const int&>(os, x, y);
				}
			}
		}

		{
			std::string header = "edgematch2d";
			COMMON_LYJ::writeBin<const std::string&>(os, header);
			writeMatches(os, m_allEdgeMatches);
		}
	}
	void Data2DEdge::read_binary(std::ifstream& is)
	{
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			std::string header2;
			COMMON_LYJ::readBin<std::string&>(is, header2);
			int imgSize;
			COMMON_LYJ::readBin<int&>(is, imgSize);
			std::string header3;
			COMMON_LYJ::readBin<std::string&>(is, header3);

			int imgId = 0;
			int pointSize = 0;
			m_allEdgePoints.resize(imgSize);
			for (int i = 0; i < imgSize; ++i) {
				COMMON_LYJ::readBin<int&>(is, imgId);
				COMMON_LYJ::readBin<int&>(is, pointSize);
				m_allEdgePoints[i].resize(pointSize);
				for (int j = 0; j < pointSize; ++j) {
					COMMON_LYJ::readBinDatas<int&, int&>(is, m_allEdgePoints[i][j].x, m_allEdgePoints[i][j].y);
				}
			}
		}
		{
			std::string header;
			COMMON_LYJ::readBin<std::string&>(is, header);
			readMatches(is, m_allEPImgPairs, m_allEdgeMatches);
		}
	}


}