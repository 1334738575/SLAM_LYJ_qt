#include <QT_LYJ.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <IO/BaseIO.h>
#include <DataIO/DataWin2D.h>


class MyStruct
    //class MyStruct : public SLAM_LYJ::BaseLYJ
{
public:
    int id = -1;
    std::string name = "null";

    MyStruct() {}
    MyStruct(int _id, std::string _name) :id(_id), name(_name) {}

    // 通过 BaseLYJ 继承
    void write_binary(std::ofstream& os) const
    {
        COMMON_LYJ::writeBinBasic<int>(os, id);
        COMMON_LYJ::writeBinString<std::string>(os, name);
    }
    void read_binary(std::ifstream& os)
    {
        COMMON_LYJ::readBinBasic<int>(os, id);
        COMMON_LYJ::readBinString<std::string>(os, name);
    }
};
void testWriteBinFile()
{
    std::string sv = "12341546";
    std::string fileNameStr = "D:/tmp/testStr.bin";
    COMMON_LYJ::writeBinFile<const std::string>(fileNameStr, sv);
    std::string fileNameStrPtr = "D:/tmp/testStrPtr.bin";
    COMMON_LYJ::writeBinFile<const std::string*>(fileNameStrPtr, &sv);
    std::vector<int> va{ 1,2,3,4 };
    std::string fileNameIntVec = "D:/tmp/testIntVec.bin";
    COMMON_LYJ::writeBinFile<const std::vector<int>>(fileNameIntVec, va);
    std::string fileNameIntVecPtr = "D:/tmp/testIntVecPtr.bin";
    COMMON_LYJ::writeBinFile<const std::vector<int>*>(fileNameIntVecPtr, &va);
    int x0 = 0;
    int x1 = 1;
    int x2 = 2;
    std::vector<int*> vb{ &x0, &x1, &x2 };
    std::string fileNameIntPtrVec = "D:/tmp/testIntPtrVec.bin";
    COMMON_LYJ::writeBinFile<const std::vector<int*>>(fileNameIntPtrVec, vb);
    std::string fileNameIntPtrVecPtr = "D:/tmp/testIntPtrVecPtr.bin";
    COMMON_LYJ::writeBinFile<const std::vector<int*>*>(fileNameIntPtrVecPtr, &vb);
    MyStruct myStrt{ 100, "lyj" };
    std::string fileNameUser = "D:/tmp/testUser.bin";
    COMMON_LYJ::writeBinFile<const MyStruct>(fileNameUser, myStrt);
    std::string fileNameMulti = "D:/tmp/testMulti.bin";
    COMMON_LYJ::writeBinFile< const std::string, const std::vector<int>, const MyStruct>(fileNameMulti, sv, va, myStrt);
}
void testReadBinFile()
{
    std::string sv;
    std::string fileNameStr = "D:/tmp/testStr.bin";
    COMMON_LYJ::readBinFile<std::string>(fileNameStr, sv);
    std::string fileNameStrPtr = "D:/tmp/testStrPtr.bin";
    std::string svTmp;
    std::string* svPtr = &svTmp;
    COMMON_LYJ::readBinFile<std::string*>(fileNameStrPtr, svPtr);
    std::vector<int> va;
    std::string fileNameIntVec = "D:/tmp/testIntVec.bin";
    COMMON_LYJ::readBinFile<std::vector<int>>(fileNameIntVec, va);
    std::string fileNameIntVecPtr = "D:/tmp/testIntVecPtr.bin";
    std::vector<int> vaTmp;
    std::vector<int>* vaPtr = &vaTmp;
    COMMON_LYJ::readBinFile<std::vector<int>*>(fileNameIntVecPtr, vaPtr);
    int x0;
    int x1;
    int x2;
    std::vector<int*> vb{ &x0, &x1, &x2 };
    std::string fileNameIntPtrVec = "D:/tmp/testIntPtrVec.bin";
    COMMON_LYJ::readBinFile<std::vector<int*>>(fileNameIntPtrVec, vb);
    std::string fileNameIntPtrVecPtr = "D:/tmp/testIntPtrVecPtr.bin";
    //std::vector<int*> vbTmp;
    //std::vector<int*>* vbPtr;
    //COMMON_LYJ::readBinFile<std::vector<int*>*>(fileNameIntPtrVecPtr, vbPtr);//TODO 指针中包含指针读取会出错
    MyStruct myStrt;
    std::string fileNameUser = "D:/tmp/testUser.bin";
    COMMON_LYJ::readBinFile<MyStruct>(fileNameUser, myStrt);
    std::string fileNameMulti = "D:/tmp/testMulti.bin";
    std::string sv2;
    std::vector<int> va2;
    MyStruct myStrt2;
    COMMON_LYJ::readBinFile<std::string, std::vector<int>, MyStruct>(fileNameMulti, sv2, va2, myStrt2);
    return;
}

void testRecord2DBin()
{
    using namespace QT_LYJ;

    Data2DPoint data2DPoint;
    std::vector<std::vector<cv::Point>>& allKPs = data2DPoint.m_allKeyPoints;
    {
        allKPs.resize(2);
        allKPs[0].resize(3);
        allKPs[0][0].x = 1;
        allKPs[0][0].y = 1;
        allKPs[0][1].x = 50;
        allKPs[0][1].y = 50;
        allKPs[0][2].x = 100;
        allKPs[0][2].y = 50;
        allKPs[1].resize(3);
        allKPs[1][0].x = 1;
        allKPs[1][0].y = 1;
        allKPs[1][1].x = 52;
        allKPs[1][1].y = 50;
        allKPs[1][2].x = 100;
        allKPs[1][2].y = 50;
    }
    std::map<int64_t, std::vector<Mth>>& allPMs = data2DPoint.m_allPointMatches;
    {
        uint64_t framePairId = imagePair2Int64(0, 1);
        std::vector<Mth>& ms = allPMs[framePairId];
        ms.resize(3);
        for (int i = 0; i < 3; ++i) {
            ms[i].first = i;
            ms[i].second = i;
        }
    }
    Data2DLine data2DLine;
    std::vector<std::vector<cv::Vec4f>>& allKLs = data2DLine.m_allKeyLines;
    {
        allKLs.resize(2);
        allKLs[0].resize(3);
        allKLs[0][0][0] = 1;
        allKLs[0][0][1] = 1;
        allKLs[0][0][2] = 20;
        allKLs[0][0][3] = 20;
        allKLs[0][1][0] = 50;
        allKLs[0][1][1] = 50;
        allKLs[0][1][2] = 70;
        allKLs[0][1][3] = 90;
        allKLs[0][2][0] = 100;
        allKLs[0][2][1] = 50;
        allKLs[0][2][2] = 10;
        allKLs[0][2][3] = 10;
        allKLs[1].resize(3);
        allKLs[1][0][0] = 1;
        allKLs[1][0][1] = 1;
        allKLs[1][0][2] = 20;
        allKLs[1][0][3] = 20;
        allKLs[1][1][0] = 52;
        allKLs[1][1][1] = 50;
        allKLs[1][1][2] = 70;
        allKLs[1][1][3] = 90;
        allKLs[1][2][0] = 100;
        allKLs[1][2][1] = 50;
        allKLs[1][2][2] = 10;
        allKLs[1][2][3] = 10;
    }
    std::map<int64_t, std::vector<Mth>>& allLMs = data2DLine.m_allLineMatches;
    {
        uint64_t framePairId = imagePair2Int64(0, 1);
        std::vector<Mth>& ms = allLMs[framePairId];
        ms.resize(3);
        for (int i = 0; i < 3; ++i) {
            ms[i].first = i;
            ms[i].second = i;
        }
    }
    Data2DEdge data2DEdge;
    std::vector<std::vector<cv::Point>>& allEPs = data2DEdge.m_allEdgePoints;
    {
        allEPs.resize(2);
        allEPs[0].resize(3);
        allEPs[0][0].x = 100;
        allEPs[0][0].y = 100;
        allEPs[0][1].x = 50;
        allEPs[0][1].y = 50;
        allEPs[0][2].x = 100;
        allEPs[0][2].y = 50;
        allEPs[1].resize(3);
        allEPs[1][0].x = 1;
        allEPs[1][0].y = 1;
        allEPs[1][1].x = 10;
        allEPs[1][1].y = 10;
        allEPs[1][2].x = 80;
        allEPs[1][2].y = 80;
    }
    std::map<int64_t, std::vector<Mth>>& allEMs = data2DEdge.m_allEdgeMatches;
    {
        uint64_t framePairId = imagePair2Int64(0, 1);
        std::vector<Mth>& ms = allEMs[framePairId];
        ms.resize(3);
        for (int i = 0; i < 3; ++i) {
            ms[i].first = i;
            ms[i].second = i;
        }
    }


    std::string dataHome = "D:/SLAM_LYJ_Packages/SLAM_LYJ_qt/data/dataBin1/";
    std::string dataHome2D = dataHome + "2D/";
    //std::string kpPath = dataHome2D + "KeyPointsAndMatches.bin";
    //std::string klPath = dataHome2D + "KeyLinesAndMatches.bin";
    //std::string epPath = dataHome2D + "EdgePointsAndMatches.bin";
    //std::string data2DPath = dataHome2D + "FeaturesAndMatches.bin";
    //COMMON_LYJ::writeBinFile<const Data2DPoint&, const Data2DLine&, const Data2DEdge&>(data2DPath, data2DPoint, data2DLine, data2DEdge);
    recordBin2D(dataHome2D, data2DPoint, data2DLine, data2DEdge);
}

int main(int argc, char* argv[])
{
    //testWriteBinFile();
    //testReadBinFile();

	using namespace QT_LYJ;
    testRecord2DBin();
    debugWindows(argc, argv);


	return 0;
}