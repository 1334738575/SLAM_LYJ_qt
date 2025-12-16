#include <QT_LYJ.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <IO/BaseIO.h>


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

int main(int argc, char* argv[])
{
    //testWriteBinFile();
    //testReadBinFile();

	using namespace QT_LYJ;
	debugWindows(argc, argv);


	return 0;
}