﻿// SearchAndUpload.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//命令行工具实现搜索Windows系统,全磁盘如果有新产生的指定多钟后缀文件（比如docx、PDF、PPT等），立即通过打包压缩加密上传文件；
//可以上传第三方网盘（可以考虑dropbox网盘，需要挂VPn,百度网盘等），上传成功后，支持rar解压加密文件；

//支持搜索多国语言文件


#include "SearchAndUpload.h"
#include "usn_manager.h"
#include "global.h"
#include "zip_manager.h"
#include "upload_manager.h"

//vector <string> change_files_path;
typedef struct {
    DWORDLONG journal_id;
    USN high_usn;
} JOURNAL_INFO;

typedef struct {
    string name;
    DWORDLONG PFRN;
} FILE_NODE;

typedef struct {
    string name;
    long long int file_cnt;
} VOLUME_INFO;




std::vector<std::string> get_all_files(std::string path, std::string suffix) {
    std::vector<std::string> files;
    //    files.clear();
    regex reg_obj(suffix, regex::icase);

    std::vector<std::string> paths;
    paths.push_back(path);
    return files;
}



void counters(int id, int numIter) {
    for (int i = 0; i < numIter; ++i) {
        cout << "counter id:" << id << endl;
        cout << "iteraion:" << i << endl;
        my_mutex.lock();
        thread_test += 1;
        cout << "counts:" << thread_test << endl;
        my_mutex.unlock();
    }
}


int file_type(char* patName, char* relName) {
    string pat;
    string allname;
    pat = patName;
    allname = relName;
    int index = allname.find(pat, allname.length() - pat.length());
    if (index != allname.npos)
        return 1;
    else
        return 0;

}


void listFiles(char* path, char* name, bool children = false) {
    intptr_t handle;
    _finddata_t findData;
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];  //curPath为当前搜索路径，nextPath为其一子文件夹路径
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //执行curPath=path+"\\*.*"
    handle = _findfirst(curPath, &findData);    // 查找目录中的第一个文件
    if (handle == -1) {
        cout << "Failed to find first file!\n";
        return;
    }

    //ofstream output;
    //output.open("files.txt");

    do {
        if ((findData.attrib == _A_SUBDIR) && (findData.name[0] != '.')) { // 是否是子目录并且不为"."或".."
            strcpy(curFileName, findData.name);
            strcpy(nextPath, path);
            strcat(nextPath, "\\");
            strcat(nextPath, curFileName);  //执行nextPath=path+"\\"+findData.name，形成子文件夹路径
            listFiles(nextPath, name, children);  //递归搜索子文件夹
        } else if (findData.name[0] != '.') {
            if (file_type(name, findData.name)) { //比较当前文件是否与搜索字符串匹配
                DWORD sf = GetFileType((HANDLE*)handle);
                cout << path << "/" << findData.name << "\t" << findData.size << endl;
                counts++;
            }

        }
    } while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
    _findclose(handle);    // 关闭搜索句柄
}

BOOL get_handle(char* volume_name, HANDLE&  volume_handle) {
    volume_handle = CreateFile(volume_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (volume_handle == INVALID_HANDLE_VALUE) return false;
    return true;

}

void scan_all_drives() {

    UINT nType;

    for (char i = 'A'; i <= 'Z'; i++) {
        char rootPath[10] = { 0 }, driveType[21] = { 0 };
        sprintf(rootPath, "%c:\\\\", i);
        nType = GetDriveType(rootPath);
        if (nType != DRIVE_NO_ROOT_DIR) {                // DRIVE_NO_ROOT_DIR: 路径无效
            cout << "detected " << rootPath << endl;
            exits_drives.push_back(rootPath);
        }
    }
}


int main() {
    std::cout << "Hello World!\n";

    string dir = "W:/*.txt";
    //listFiles((char*)"D:", (char*)".cpp", true);

    //获取所有盘符
    scan_all_drives();



    //for (int i = 0; i < exits_drives.size(); i++) {
    //    //
    //    cout << exits_drives.at(i) << endl;
    //}

    usn_manager usn;
    DWORD startTimeStamp = GetTickCount();
    while (true) {
        DWORD endTimeStamp = GetTickCount();
        //50秒循环清理一次
        if (change_files_path.size() && (endTimeStamp - startTimeStamp > 40000)) {
            for (int i = 0 ; i < change_files_path.size(); i++)
                cout << change_files_path.at(i) << endl;
            zip_manager zip_packer;


			zip_packer.start((char*)"W:\\test_add_file.zip", (char*) "CBR", change_files_path);

            upload_manager uploader;
            uploader.start("W:\\test_add_file.zip");
			
            change_files_path.erase(change_files_path.begin(), change_files_path.end());
            startTimeStamp = GetTickCount();

        } else {
            cout << "准备全盘扫描" << endl;
            usn.start(exits_drives);
			DWORD tmp = GetTickCount();
			cout << endl << "全盘扫描完毕 花费" << (tmp - startTimeStamp)/1000 <<"秒" << endl << endl;
        }
    }


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
