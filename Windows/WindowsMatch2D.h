#ifndef WINDOWS_MATCH2D_H
#define WINDOWS_MATCH2D_H

#include <opencv2/opencv.hpp>
//#include <opencv2/line_descriptor/descriptor.hpp>
//#include <opencv2/features2d.hpp>
//#include <opencv2/xfeatures2d.hpp>

namespace QT_LYJ
{
	class WindowsMatch2D
	{
	public:
		using Mth = std::pair<int, int>;
		using ImgInd = std::pair<int, int64_t>;
		enum ShowStatus
		{
			SDEFAULT = 0,
			SPOINT,
			SPOINTMATCH,
			SLINE,
			SLINEMATCH
		};
		WindowsMatch2D(std::function<void(const std::string&)> _printFunc = nullptr);
		~WindowsMatch2D();

		void loadModel(const std::string& _path);
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
		int64_t imagePair2Int64(int _i1, int _i2) const
		{
			if(_i1 <= _i2)
				return (static_cast<int64_t>(_i1) << 32) | static_cast<int64_t>(_i2);
			else
				return (static_cast<int64_t>(_i2) << 32) | static_cast<int64_t>(_i1);
		}
		std::pair<int, int> int642TwoImagePair(int64_t _pair) const
		{
			int i1 = static_cast<int>(_pair >> 32);
			int i2 = static_cast<int>(_pair & 0xFFFFFFFF);
			return { i1, i2 };
		}

		int drawFeatures(cv::Mat& _img, int _imgId, ShowStatus _status);
		int drawMatches(const cv::Mat& _img1, const cv::Mat& _img2, const int _imgId1, const ImgInd _imgId2,
			ShowStatus _status, cv::Mat& _img2Show);

		void drawKeyPoints(cv::Mat& _img, const std::vector<cv::Point>& _keyPoints, const cv::Scalar& _color);
		void drawKeyLines(cv::Mat& _img, const std::vector<cv::Vec4f>& _keyLines, const cv::Scalar& _color);
		void drawKeyPointMatches(const cv::Mat& _img1, const cv::Mat& _img2,
			const std::vector<cv::Point>& _keyPoints1, const std::vector<cv::Point>& _keyPoints2,
			const std::vector<Mth>& _matches, cv::Mat& _img2Show, const cv::Scalar& _color);
		void drawKeyLineMatches(const cv::Mat& _img1, const cv::Mat& _img2,
			const std::vector<cv::Vec4f>& _keyLines1, const std::vector<cv::Vec4f>& _keyLines2,
			const std::vector<Mth>& _matches, cv::Mat& _img2Show, const cv::Scalar& _color);
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
		std::function<void(const std::string&)> m_printFunc = nullptr; // ‰≥ˆdebug–≈œ¢
		std::vector<std::vector<cv::Point>> m_allKeyPoints;
		std::map<int, std::vector<ImgInd>> m_allKPImgPairs;
		std::map<int64_t, std::vector<Mth>> m_allPointMatches;
		std::vector<std::vector<cv::Vec4f>> m_allKeyLines;
		std::map<int, std::vector<ImgInd>> m_allKLImgPairs;
		std::map<int64_t, std::vector<Mth>> m_allLineMatches;
		std::vector<std::string> m_allImageNames;
	};
}


#endif // !WINDOWS_MATCH2D_H
