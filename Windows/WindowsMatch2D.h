#ifndef WINDOWS_MATCH2D_H
#define WINDOWS_MATCH2D_H

#include <opencv2/opencv.hpp>
#include <IO/DataWin2D.h>
#include <common/CompressedImage.h>

namespace QT_LYJ
{
	using namespace COMMON_LYJ;
	class WindowsMatch2D
	{
	public:
		enum ShowStatus
		{
			SDEFAULT = 0,
			SPOINT,
			SPOINTMATCH,
			SLINE,
			SLINEMATCH,
			SEDGE,
			SEDGEMATCH
		};
		WindowsMatch2D(std::function<void(const std::string &)> _printFunc = nullptr);
		~WindowsMatch2D();

		void loadModel(const std::string &_path);
		void show();
		std::string getStatusString() const
		{
			switch (m_status)
			{
			case SDEFAULT:
				return "Default";
			case SPOINT:
				return "Point";
			case SPOINTMATCH:
				return "Point Match";
			case SLINE:
				return "Line";
			case SLINEMATCH:
				return "Line Match";
			default:
				return "Unknown";
			}
		}

	private:
		int drawFeatures(cv::Mat &_img, int _imgId, ShowStatus _status);
		int drawMatches(const cv::Mat &_img1, const cv::Mat &_img2, const int _imgId1, const ImgInd _imgId2,
						ShowStatus _status, cv::Mat &_img2Show);

		void drawKeyPoints(cv::Mat &_img, const std::vector<cv::Point> &_keyPoints, const cv::Scalar &_color);
		void drawKeyLines(cv::Mat &_img, const std::vector<cv::Vec4f> &_keyLines, const cv::Scalar &_color);
		void drawKeyPointMatches(const cv::Mat &_img1, const cv::Mat &_img2,
								 const std::vector<cv::Point> &_keyPoints1, const std::vector<cv::Point> &_keyPoints2,
								 const std::vector<Mth> &_matches, cv::Mat &_img2Show, const cv::Scalar &_color);
		void drawKeyLineMatches(const cv::Mat &_img1, const cv::Mat &_img2,
								const std::vector<cv::Vec4f> &_keyLines1, const std::vector<cv::Vec4f> &_keyLines2,
								const std::vector<Mth> &_matches, cv::Mat &_img2Show, const cv::Scalar &_color);

	private:
		int m_w = 0;
		int m_h = 0;
		int m_imgSize = 0;
		cv::Mat m_m2Show;
		cv::Mat m_imgTmp1;
		cv::Mat m_imgTmp2;
		cv::Rect m_rect1;
		cv::Rect m_rect2;
		ShowStatus m_status = SDEFAULT;
		std::function<void(const std::string &)> m_printFunc = nullptr; //  ‰≥ˆdebug–≈œ¢

		Data2DPoint m_dt2DP;
		Data2DLine m_dt2DL;
		Data2DEdge m_dt2DE;
		// std::vector<std::vector<cv::Point>> m_allKeyPoints;
		// std::map<int, std::vector<ImgInd>> m_allKPImgPairs;
		// std::map<int64_t, std::vector<Mth>> m_allPointMatches;
		// std::vector<std::vector<cv::Vec4f>> m_allKeyLines;
		// std::map<int, std::vector<ImgInd>> m_allKLImgPairs;
		// std::map<int64_t, std::vector<Mth>> m_allLineMatches;
		// std::vector<std::vector<cv::Point>> m_allEdgePoints;
		// std::map<int, std::vector<ImgInd>> m_allEPImgPairs;
		// std::map<int64_t, std::vector<Mth>> m_allEdgeMatches;

		std::vector<std::string> m_allImageNames;
		std::vector<COMMON_LYJ::CompressedImage> m_comImgs;
		bool m_useComImg = false;
	};
}

#endif // !WINDOWS_MATCH2D_H
