#include "WindowsMatch2D.h"

namespace QT_LYJ
{
	WindowsMatch2D::WindowsMatch2D(std::function<void(const std::string&)> _printFunc)
		:m_printFunc(_printFunc)
	{}
	WindowsMatch2D::~WindowsMatch2D()
	{}

	void WindowsMatch2D::loadModel(const std::string& _path)
	{
		{
			std::string kpPath = _path + "/KeyPoints.txt";
			std::ifstream kpf(kpPath);
			std::string header = "";
			int pointSize = 0;
			int imgId = 0;
			kpf >> header;
			kpf >> header >> m_imgSize;
			m_allKeyPoints.resize(m_imgSize);
			kpf >> header;
			float x, y;
			for (int i = 0; i < m_imgSize; ++i) {
				kpf >> imgId;
				kpf >> pointSize;
				m_allKeyPoints[i].resize(pointSize);
				for (int j = 0; j < pointSize; ++j) {
					kpf >>x >> y;
					m_allKeyPoints[i][j].x = x;
					m_allKeyPoints[i][j].y = y;
				}
			}
			kpf.close();
		}
		{
			std::string mPath = _path + "/Matches.txt";
			std::ifstream mf(mPath);
			std::string header = "";
			int framePair = 0, frameId1, frameId2, matchSize = 0, mId1, mId2;
			mf >> header;
			mf >> header >> framePair;
			mf >> header;
			uint64_t pairId = 0;
			for (int i = 0; i < framePair; ++i) {
				mf >> frameId1 >> frameId2 >> matchSize;
				pairId = imagePair2Int64(frameId1, frameId2);
				m_allPointMatches[pairId].resize(matchSize);
				m_allKPImgPairs[frameId1].emplace_back(frameId2, pairId);
				for (int j = 0; j < matchSize; ++j) {
					mf >> m_allPointMatches[pairId][j].first >> m_allPointMatches[pairId][j].second;
				}
			}
			mf.close();
		}

		{
			std::string klPath = _path + "/KeyLines.txt";
			std::ifstream klf(klPath);
			std::string header = "";
			int lineSize = 0;
			int imgId = 0;
			klf >> header;
			klf >> header >> m_imgSize;
			m_allKeyLines.resize(m_imgSize);
			klf >> header;
			for (int i = 0; i < m_imgSize; ++i) {
				klf >> imgId;
				klf >> lineSize;
				m_allKeyLines[i].resize(lineSize);
				for (int j = 0; j < lineSize; ++j) {
					klf >> m_allKeyLines[i][j][0] >> m_allKeyLines[i][j][1] >> m_allKeyLines[i][j][2] >> m_allKeyLines[i][j][3];
				}
			}
			klf.close();
		}
		{
			std::string mPath = _path + "/LineMatches.txt";
			std::ifstream mf(mPath);
			std::string header = "";
			int framePair = 0, frameId1, frameId2, matchSize = 0, mId1, mId2;
			mf >> header;
			mf >> header >> framePair;
			mf >> header;
			uint64_t pairId = 0;
			for (int i = 0; i < framePair; ++i) {
				mf >> frameId1 >> frameId2 >> matchSize;
				pairId = imagePair2Int64(frameId1, frameId2);
				m_allLineMatches[pairId].resize(matchSize);
				m_allKLImgPairs[frameId1].emplace_back(frameId2, pairId);
				for (int j = 0; j < matchSize; ++j) {
					mf >> m_allLineMatches[pairId][j].first >> m_allLineMatches[pairId][j].second;
				}
			}
			mf.close();
		}

		{
			std::string egPath = _path + "/EdgePoints.txt";
			std::ifstream egf(egPath);
			std::string header = "";
			int edgeSize = 0;
			int imgId = 0;
			egf >> header;
			egf >> header >> m_imgSize;
			m_allEdgePoints.resize(m_imgSize);
			egf >> header;
			float x, y;
			for (int i = 0; i < m_imgSize; ++i) {
				egf >> imgId;
				egf >> edgeSize;
				m_allEdgePoints[i].resize(edgeSize);
				for (int j = 0; j < edgeSize; ++j) {
					egf >> x >> y;
					m_allEdgePoints[i][j].x = x;
					m_allEdgePoints[i][j].y = y;
				}
			}
			egf.close();
		}
		{
			std::string egmPath = _path + "/EdgeMatches.txt";
			std::ifstream egmf(egmPath);
			std::string header = "";
			int framePair = 0, frameId1, frameId2, edgeMatchSize = 0, mId1, mId2;
			egmf >> header;
			egmf >> header >> framePair;
			egmf >> header;
			uint64_t pairId = 0;
			for (int i = 0; i < framePair; ++i) {
				egmf >> frameId1 >> frameId2 >> edgeMatchSize;
				pairId = imagePair2Int64(frameId1, frameId2);
				m_allEdgeMatches[pairId].resize(edgeMatchSize);
				m_allEPImgPairs[frameId1].emplace_back(frameId2, pairId);
				for (int j = 0; j < edgeMatchSize; ++j) {
					egmf >> m_allEdgeMatches[pairId][j].first >> m_allEdgeMatches[pairId][j].second;
				}
			}
			egmf.close();
		}

		m_imgTmp1 = cv::imread(_path + "/images/0.png");
		m_w = m_imgTmp1.cols;
		m_h = m_imgTmp1.rows;
		m_rect1 = cv::Rect(0, 0, m_w, m_h);
		m_rect2 = cv::Rect(m_w, 0, m_w, m_h);
		m_imgTmp2 = cv::Mat(m_h, m_w, CV_8UC3);
		m_m2Show = cv::Mat(m_h, 2 * m_w, CV_8UC3);
		for(int i=0;i<m_imgSize;++i)
			m_allImageNames.push_back(_path + "/images/" + std::to_string(i) + ".png");

		show();
		return;
	}
	void WindowsMatch2D::show()
	{
		cv::namedWindow("Windows Match 2D", cv::WINDOW_NORMAL);
		int imgId1 = 0;
		int ind2 = -1;
		int s = 1;
		int pairSize = 0;
		std::vector<ImgInd> imgInds;

		ImgInd imgId2 = { -1, -1 };
		int fSize = 0;
		int mSize = 0;
		int kpSize = 0;
		int kpMSize = 0;
		int klSize = 0;
		int klMSize = 0;
		int egSize = 0;
		int egMSize = 0;

		int key = -1;
		std::string logStr = "Windows Match 2D, status: " + getStatusString() \
			+ ", imgId1: " + std::to_string(imgId1) + ", imgId2: " + std::to_string(imgId2.first) + "\n" \
			+ ", kp size: " + std::to_string(kpSize) + ", kp match size: " + std::to_string(kpMSize) \
			+ ", kl size: " + std::to_string(klSize) + ", kl match size: " + std::to_string(klMSize) \
			+ ", eg size: " + std::to_string(egSize) + ", eg match size: " + std::to_string(egMSize) + "\n" \
			+ ", step: " + std::to_string(s) + "\n";
		while (key != 27 || m_status != SDEFAULT) //esc
		{
			if (key == 'w') {
				if (m_status == SDEFAULT || m_status == SPOINT || m_status == SLINE || m_status == SEDGE)
					imgId1 = (imgId1 + s) >= m_imgSize ? m_imgSize - 1 : (imgId1 + s);
				else if (m_status == SPOINTMATCH || m_status == SLINEMATCH || m_status == SEDGEMATCH)
					ind2 = (ind2 + s) >= pairSize ? pairSize - 1 : (ind2 + s);
			}
			else if (key == 's') {
				if (m_status == SDEFAULT || m_status == SPOINT || m_status == SLINE || m_status == SEDGE)
					imgId1 = (imgId1 - s) < 0 ? 0 : (imgId1 - s);
				else if (m_status == SPOINTMATCH || m_status == SLINEMATCH || m_status == SEDGEMATCH)
					ind2 = (ind2 - s) < 0 ? 0 : (ind2 - s);
			}
			else if (key == 13) { // enter
				if (m_status == SPOINT) {
					m_status = SPOINTMATCH;
					imgInds = m_allKPImgPairs[imgId1];
					ind2 = 0;
				}
				else if (m_status == SLINE) {
					m_status = SLINEMATCH;
					imgInds = m_allKLImgPairs[imgId1];
					ind2 = 0;
				}
				else if (m_status == SEDGE) {
					m_status = SEDGEMATCH;
					imgInds = m_allEPImgPairs[imgId1];
					ind2 = 0;
				}
				pairSize = imgInds.size();
			}
			else if (key == 27) { // esc
				ind2 = -1;
				imgId2 = { -1, -1 };
				imgInds.clear();
				pairSize = 0;
				if (m_status == SPOINTMATCH)
					m_status = SPOINT;
				else if (m_status == SLINEMATCH)
					m_status = SLINE;
				else if (m_status == SEDGEMATCH)
					m_status = SEDGE;
				else
					m_status = SDEFAULT;
			}
			else if (key == 'd') {
				if (m_status == SDEFAULT)
					m_status = SPOINT;
				else if (m_status == SPOINT)
					m_status = SLINE;
				else if (m_status == SLINE)
					m_status = SEDGE;
				else if (m_status == SEDGE)
					m_status = SDEFAULT;
			}
			else if (key == 'a') {
				if (m_status == SDEFAULT)
					m_status = SEDGE;
				else if (m_status == SEDGE)
					m_status = SLINE;
				else if (m_status == SLINE)
					m_status = SPOINT;
				else if (m_status == SPOINT)
					m_status = SDEFAULT;
			}
			else if (key == 'e') {
				s *= 2;
				if (s >= m_imgSize)
					s = m_imgSize - 1;
			}
			else if (key == 'q') {
				s /= 2;
				if (s < 1)
					s = 1;
			}

			//imgid1 imgid2 ind2 to mat
			if (ind2 != -1 && !imgInds.empty())
				imgId2 = imgInds[ind2];
			kpSize = 0;
			kpMSize = 0;
			klSize = 0;
			klMSize = 0;
			egSize = 0;
			egMSize = 0;
			fSize = drawFeatures(m_imgTmp1, imgId1, m_status);
			if (m_status == SPOINT || m_status == SPOINTMATCH)
				kpSize = fSize;
			else if(m_status == SLINE || m_status == SLINEMATCH)
				klSize = fSize;
			else if (m_status == SEDGE || m_status == SEDGEMATCH)
				egSize = fSize;
			drawFeatures(m_imgTmp2, imgId2.first, m_status);
			mSize = drawMatches(m_imgTmp1, m_imgTmp2, imgId1, imgId2, m_status, m_m2Show);
			if (m_status == SPOINT || m_status == SPOINTMATCH)
				kpMSize = mSize;
			else if (m_status == SLINE || m_status == SLINEMATCH)
				klMSize = mSize;
			else if (m_status == SEDGE || m_status == SEDGEMATCH)
				egMSize = mSize;

			logStr = "Windows Match 2D, status: " + getStatusString() \
				+ ", imgId1: " + std::to_string(imgId1) + ", imgId2: " + std::to_string(imgId2.first) + "\n" \
				+ ", kp size: " + std::to_string(kpSize) + ", kp match size: " + std::to_string(kpMSize) \
				+ ", kl size: " + std::to_string(klSize) + ", kl match size: " + std::to_string(klMSize) \
				+ ", eg size: " + std::to_string(egSize) + ", eg match size: " + std::to_string(egMSize) + "\n" \
				+ ", step: " + std::to_string(s) + "\n";
			m_printFunc(logStr);
			cv::imshow("Windows Match 2D", m_m2Show);
			key = cv::waitKey();
		}
		cv::destroyWindow("Windows Match 2D");
	}

	int WindowsMatch2D::drawFeatures(cv::Mat& _img, int _imgId,
		ShowStatus _status)
	{
		if (_imgId < 0 || _imgId >= m_imgSize) {
			_img = cv::Mat::zeros(m_h, m_w, CV_8UC3);
			return 0;
		}
		_img = cv::imread(m_allImageNames[_imgId]);
		if (_status == SPOINT || _status == SPOINTMATCH) {
			drawKeyPoints(_img, m_allKeyPoints[_imgId], cv::Scalar(0, 255, 0));
			return m_allKeyPoints[_imgId].size();
		}
		else if (_status == SLINE || _status == SLINEMATCH) {
			drawKeyLines(_img, m_allKeyLines[_imgId], cv::Scalar(0, 0, 255));
			return m_allKeyLines[_imgId].size();
		}
		else if (_status == SEDGE || _status == SEDGEMATCH) {
			drawKeyPoints(_img, m_allEdgePoints[_imgId], cv::Scalar(0, 0, 255));
			return m_allEdgePoints[_imgId].size();
		}
		return 0;
	}
	int WindowsMatch2D::drawMatches(const cv::Mat& _img1, const cv::Mat& _img2,
		const int _imgId1, const ImgInd _imgId2,
		ShowStatus _status, cv::Mat& _img2Show)
	{
		int imgId2 = _imgId2.first;
		_img1.copyTo(_img2Show(m_rect1));
		_img2.copyTo(_img2Show(m_rect2));
		if (_imgId1 < 0 || _imgId1 >= m_imgSize || imgId2 < 0 || imgId2 >= m_imgSize)
			return 0;
		if (m_status == SPOINTMATCH) {
			const std::vector<Mth>& matches = m_allPointMatches[_imgId2.second];
			const std::vector<cv::Point>& keyPoints1 = m_allKeyPoints[_imgId1];
			const std::vector<cv::Point>& keyPoints2 = m_allKeyPoints[imgId2];
			drawKeyPointMatches(_img1, _img2, keyPoints1, keyPoints2, matches, _img2Show, cv::Scalar(255, 0, 0));
			return matches.size();
		}
		else if (m_status == SLINEMATCH) {
			const std::vector<Mth>& matches = m_allLineMatches[_imgId2.second];
			const std::vector<cv::Vec4f>& keyLines1 = m_allKeyLines[_imgId1];
			const std::vector<cv::Vec4f>& keyLines2 = m_allKeyLines[imgId2];
			drawKeyLineMatches(_img1, _img2, keyLines1, keyLines2, matches, _img2Show, cv::Scalar(255, 0, 0));
			return matches.size();
		}
		else if (m_status == SEDGEMATCH) {
			const std::vector<Mth>& matches = m_allEdgeMatches[_imgId2.second];
			const std::vector<cv::Point>& edgePoints1 = m_allEdgePoints[_imgId1];
			const std::vector<cv::Point>& edgePoints2 = m_allEdgePoints[imgId2];
			drawKeyPointMatches(_img1, _img2, edgePoints1, edgePoints2, matches, _img2Show, cv::Scalar(255, 0, 0));
			return matches.size();
		}
		return 0;
	}

	void WindowsMatch2D::drawKeyPoints(cv::Mat& _img, const std::vector<cv::Point>& _keyPoints, const cv::Scalar& _color)
	{
		for (const auto& kp : _keyPoints)
			cv::circle(_img, kp, 3, _color);
	}
	void WindowsMatch2D::drawKeyLines(cv::Mat& _img, const std::vector<cv::Vec4f>& _keyLines, const cv::Scalar& _color)
	{
		for (const auto& kl : _keyLines)
			cv::line(_img, cv::Point(kl[0], kl[1]), cv::Point(kl[2], kl[3]), _color, 2);
	}
	void WindowsMatch2D::drawKeyPointMatches(const cv::Mat& _img1, const cv::Mat& _img2,
		const std::vector<cv::Point>& _keyPoints1, const std::vector<cv::Point>& _keyPoints2,
		const std::vector<Mth>& _matches, cv::Mat& _img2Show, const cv::Scalar& _color)
	{
		//_img2Show = cv::Mat::zeros(_img1.rows, _img1.cols + _img2.cols, CV_8UC3);
		//_img1.copyTo(_img2Show(cv::Rect(0, 0, _img1.cols, _img1.rows)));
		//_img2.copyTo(_img2Show(cv::Rect(_img1.cols, 0, _img2.cols, _img2.rows)));
		cv::Point pt2;
		for (const auto& m : _matches) {
			if (m.first < 0 || m.first >= static_cast<int>(_keyPoints1.size()) || m.second < 0 || m.second >= static_cast<int>(_keyPoints2.size()))
				continue;
			const cv::Point& pt1 = _keyPoints1[m.first];
			pt2 = _keyPoints2[m.second] + cv::Point(_img1.cols, 0);
			cv::line(_img2Show, pt1, pt2, _color);
			//cv::circle(_img2Show, pt1, 3, _color);
			//cv::circle(_img2Show, pt2, 3, _color);
		}
	}
	void WindowsMatch2D::drawKeyLineMatches(const cv::Mat& _img1, const cv::Mat& _img2,
		const std::vector<cv::Vec4f>& _keyLines1, const std::vector<cv::Vec4f>& _keyLines2,
		const std::vector<Mth>& _matches, cv::Mat& _img2Show, const cv::Scalar& _color)
	{
		//_img2Show = cv::Mat::zeros(_img1.rows, _img1.cols + _img2.cols, CV_8UC3);
		//_img1.copyTo(_img2Show(cv::Rect(0, 0, _img1.cols, _img1.rows)));
		//_img2.copyTo(_img2Show(cv::Rect(_img1.cols, 0, _img2.cols, _img2.rows)));
		cv::Vec4f kl2;
		for (const auto& m : _matches) {
			if (m.first < 0 || m.first >= static_cast<int>(_keyLines1.size()) || m.second < 0 || m.second >= static_cast<int>(_keyLines2.size()))
				continue;
			const cv::Vec4f& kl1 = _keyLines1[m.first];
			kl2 = _keyLines2[m.second] + cv::Vec4f(_img1.cols, 0, _img1.cols, 0);
			cv::line(_img2Show, cv::Point(kl1[0] + kl1[2], kl1[1] + kl1[3])/2, cv::Point(kl2[0] + kl2[2], kl2[1] + kl2[3])/2, _color);
			//cv::circle(_img2Show, cv::Point(kl1[0], kl1[1]), 3, _color);
			//cv::circle(_img2Show, cv::Point(kl2[0], kl2[1]), 3, _color);
		}
	}

}