#ifndef QT_LYJ_DATAWIN2D_H
#define QT_LYJ_DATAWIN2D_H

#include "QT_LYJ_Defines.h"
#include <fstream>
#include <opencv2/opencv.hpp>
#include <IO/BaseIO.h>

namespace QT_LYJ
{
	using Mth = std::pair<int, int>;
	using ImgInd = std::pair<int, int64_t>;
	enum class Data2DType
	{
		DEFAULT=0,
		POINT2D,
		LINE2D,
		EDGE2D
	};
	class Data2DAbr
	{
	public:
		Data2DAbr() {};
		~Data2DAbr() {};

		virtual void write_binary(std::ofstream& os) const = 0;
		virtual void read_binary(std::ifstream& is) = 0;

	public:
		Data2DType dataType = Data2DType::DEFAULT;

	protected:
		void writeMatches(std::ofstream& os, const std::map<int64_t, std::vector<Mth>>& allMatches) const
		{
			std::string header2 = "framePair";
			COMMON_LYJ::writeBin<const std::string&>(os, header2);
			int framePair = allMatches.size();
			COMMON_LYJ::writeBin<const int&>(os, framePair);
			std::string header3 = "data";
			COMMON_LYJ::writeBin<const std::string&>(os, header3);
			//uint64_t pairId;
			int frameId1, frameId2, matchSize;
			std::pair<int, int> frameIdPair;
			for (const auto& matchest : allMatches)
			{
				frameIdPair = int642TwoImagePair(matchest.first);
				COMMON_LYJ::writeBinDatas<const int&, const int&>(os, frameIdPair.first, frameIdPair.second);
				const auto& matches = matchest.second;
				matchSize = matches.size();
				COMMON_LYJ::writeBin<const int&>(os, matchSize);
				uint64_t sz = matchSize * 2;
				COMMON_LYJ::writeBinDirect<int>(os, &(matches[0].first), sz);
				//for (int i = 0; i < matchSize; ++i)
				//{
				//	const auto& m = matches[i];
				//	COMMON_LYJ::writeBinDatas<const int&, const int&>(os, m.first, m.second);
				//}
			}

		}
		void readMatches(std::ifstream& is,
			std::map<int, std::vector<ImgInd>>& allImgPairs,
			std::map<int64_t, std::vector<Mth>>& allMatches)
		{
			std::string header2;
			COMMON_LYJ::readBin<std::string&>(is, header2);
			int framePair;
			COMMON_LYJ::readBin<int&>(is, framePair);
			std::string header3;
			COMMON_LYJ::readBin<std::string&>(is, header3);
			int frameId1, frameId2, matchSize = 0, mId1, mId2;
			uint64_t pairId = 0;
			for (int i = 0; i < framePair; ++i) {
				COMMON_LYJ::readBinDatas<int&, int&>(is, frameId1, frameId2);
				pairId = imagePair2Int64(frameId1, frameId2);
				COMMON_LYJ::readBin<int&>(is, matchSize);
				allMatches[pairId].resize(matchSize);
				allImgPairs[frameId1].emplace_back(frameId2, pairId);
				uint64_t sz = matchSize * 2;
				COMMON_LYJ::readBinDirect<int>(is, &(allMatches[pairId][0].first), sz);
				//for (int j = 0; j < matchSize; ++j) {
				//	COMMON_LYJ::readBinDatas<int&, int&>(is, allMatches[pairId][j].first, allMatches[pairId][j].second);
				//}
			}
		}
	};


	class QT_LYJ_API Data2DPoint : public Data2DAbr
	{
	public:
		Data2DPoint();
		~Data2DPoint();

		// 通过 Data2DAbr 继承
		void write_binary(std::ofstream& os) const override;

		void read_binary(std::ifstream& is) override;

		std::vector<std::vector<cv::Point>> m_allKeyPoints;
		std::map<int, std::vector<ImgInd>> m_allKPImgPairs;
		std::map<int64_t, std::vector<Mth>> m_allPointMatches;
	private:

	};


	class QT_LYJ_API Data2DLine : public Data2DAbr
	{
	public:
		Data2DLine();
		~Data2DLine();

		// 通过 Data2DAbr 继承
		void write_binary(std::ofstream& os) const override;

		void read_binary(std::ifstream& is) override;

		std::vector<std::vector<cv::Vec4f>> m_allKeyLines;
		std::map<int, std::vector<ImgInd>> m_allKLImgPairs;
		std::map<int64_t, std::vector<Mth>> m_allLineMatches;
	private:

	};


	class QT_LYJ_API Data2DEdge : public Data2DAbr
	{
	public:
		Data2DEdge();
		~Data2DEdge();

		// 通过 Data2DAbr 继承
		void write_binary(std::ofstream& os) const override;

		void read_binary(std::ifstream& is) override;


		std::vector<std::vector<cv::Point>> m_allEdgePoints;
		std::map<int, std::vector<ImgInd>> m_allEPImgPairs;
		std::map<int64_t, std::vector<Mth>> m_allEdgeMatches;
	private:

	};



}





#endif // !QT_LYJ_DATAWIN2D_H
