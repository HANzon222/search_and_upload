// SearchAndUpload.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//
//�����й���ʵ������Windowsϵͳ,ȫ����������²�����ָ�����Ӻ�׺�ļ�������docx��PDF��PPT�ȣ�������ͨ�����ѹ�������ϴ��ļ���
//�����ϴ����������̣����Կ���dropbox���̣���Ҫ��VPn,�ٶ����̵ȣ����ϴ��ɹ���֧��rar��ѹ�����ļ���

//֧��������������ļ�

#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <direct.h>
#include <iostream>
#include <io.h>
#include <fstream>
#include <cstring>
#include <string.h>
#include <windows.h>
#include <stack>
#include <thread>
#include <mutex>
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <time.h>
#include <handleapi.h>


using namespace std;

mutex my_mutex;
int counts = 0;  //count����ͳ���ļ���
using namespace std;
char* volName = (char*)"w:\\";
HANDLE hVol = INVALID_HANDLE_VALUE;
USN_JOURNAL_DATA UsnInfo; // ����USN��־�Ļ�����Ϣ
#define BUF_LEN 4096

ofstream fout("E:\\log.txt");
long counter = 0;




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
        counts += 1;
        cout << "counts:" << counts << endl;
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
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];  //curPathΪ��ǰ����·����nextPathΪ��һ���ļ���·��
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //ִ��curPath=path+"\\*.*"
    handle = _findfirst(curPath, &findData);    // ����Ŀ¼�еĵ�һ���ļ�
    if (handle == -1) {
        cout << "Failed to find first file!\n";
        return;
    }

    //ofstream output;
    //output.open("files.txt");

    do {
        if ((findData.attrib == _A_SUBDIR) && (findData.name[0] != '.')) { // �Ƿ�����Ŀ¼���Ҳ�Ϊ"."��".."
            strcpy(curFileName, findData.name);
            strcpy(nextPath, path);
            strcat(nextPath, "\\");
            strcat(nextPath, curFileName);  //ִ��nextPath=path+"\\"+findData.name���γ����ļ���·��
            listFiles(nextPath, name, children);  //�ݹ��������ļ���
        } else if (findData.name[0] != '.') {
            if (file_type(name, findData.name)) { //�Ƚϵ�ǰ�ļ��Ƿ��������ַ���ƥ��
                DWORD sf = GetFileType((HANDLE*)handle);
                cout << path << "/" << findData.name << "\t" << findData.size << endl;
                counts++;
            }

        }
    } while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�
    _findclose(handle);    // �ر��������
}

BOOL get_handle(char* volume_name, HANDLE&  volume_handle) {
    volume_handle = CreateFile(volume_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (volume_handle == INVALID_HANDLE_VALUE) return false;
    return true;

}


int main() {
    std::cout << "Hello World!\n";

    string dir = "W:/*.txt";
    //listFiles((char*)"D:", (char*)".cpp", true);
    // thread t1(counters, 1, 6);
    // thread t2(counters, 2, 4);

    // //���û��join��main�������������̺߳����������������߳�Ҳ��ֹ
    // //����ȷ�����߳�һֱ���У�ֱ�������̶߳�ִ�����
    // t1.join();
    // t2.join();





    //if (!get_handle(volName, hVol)) {
    //  cout << "error handle" << endl;
    //}



    BOOL status;
    BOOL isNTFS = false;
    BOOL getHandleSuccess = false;
    BOOL initUsnJournalSuccess = false;

    //�ж��������Ƿ�NTFS��ʽ
    cout << "step 01. �ж��������Ƿ�NTFS��ʽ\n";
    char sysNameBuf[MAX_PATH] = { 0 };
    status = GetVolumeInformationA(volName,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   sysNameBuf, // �����̵�ϵͳ��
                                   MAX_PATH);
    cout << status << endl;
    if (0 != status) {
        cout << "�ļ�ϵͳ��:" << sysNameBuf << "\n";
        // �Ƚ��ַ���
        if (0 == strcmp(sysNameBuf, "NTFS")) {
            cout << "����������NTFS��ʽ��ת��step-02.\n";
            isNTFS = true;
        } else
            cout << "�������̷�NTFS��ʽ\n";

    }

    if (isNTFS) {
        //step 02. ��ȡ�����̾��
        cout << "step 02. ��ȡ�����̾��\n";
        char fileName[MAX_PATH];
        fileName[0] = '\0';
        strcpy_s(fileName, "\\\\.\\");//������ļ���
        strcat_s(fileName, volName);
        string fileNameStr = (string)fileName;
        fileNameStr.erase(fileNameStr.find_last_of(":") + 1);
        cout << "�����̵�ַ:" << fileNameStr.data() << "\n";
        // hVol = CreateFile((LPCWSTR)fileNameStr.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        hVol = CreateFileA(fileNameStr.data(),//�ɴ򿪻򴴽����¶��󣬲����ؿɷ��ʵľ��������̨��ͨ����Դ��Ŀ¼��ֻ���򿪣����������������ļ�
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_READONLY,
                          NULL);
        cout << hVol << endl;

        if (INVALID_HANDLE_VALUE != hVol) {
            cout << "��ȡ�����̾���ɹ���ת��step-03.\n";
            getHandleSuccess = true;
        } else
            cout << "��ȡ�����̾��ʧ�� ���� handle:" << hVol << " error:" << GetLastError() << "\n";
    }

    if (getHandleSuccess) {
        //step 03. ��ʼ��USN��־�ļ�
        cout << "step 03. ��ʼ��USN��־�ļ�\n";
        DWORD br;
        CREATE_USN_JOURNAL_DATA cujd;
        cujd.MaximumSize = 0;
        cujd.AllocationDelta = 0;
        status = DeviceIoControl(hVol,
                                 FSCTL_CREATE_USN_JOURNAL,
                                 &cujd,
                                 sizeof(cujd),
                                 NULL,
                                 0,
                                 &br,
                                 NULL);

        if (0 != status) {
            cout << "��ʼ��USN��־�ļ��ɹ���ת��step-04.\n";
            initUsnJournalSuccess = true;
        } else
            cout << "��ʼ��USN��־�ļ�ʧ�� ���� status:" << status << " error:" << GetLastError() << "\n";
    }
    JOURNAL_INFO journal_info;
    if (initUsnJournalSuccess) {

        BOOL getBasicInfoSuccess = false;


        //step 04. ��ȡUSN��־������Ϣ(���ں�������)
        cout << "step 04. ��ȡUSN��־������Ϣ(���ں�������)\n";
        DWORD br;
        status = DeviceIoControl(hVol,
                                 FSCTL_QUERY_USN_JOURNAL,
                                 NULL,
                                 0,
                                 &UsnInfo,
                                 sizeof(UsnInfo),
                                 &br,
                                 NULL);

        DWORD bytes_returned;
        USN_JOURNAL_DATA ujd;

        if (DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &bytes_returned, NULL)) {
            journal_info.journal_id = ujd.UsnJournalID;
            journal_info.high_usn = ujd.NextUsn;
            cout << "  safasf ��ȡUSN��־������Ϣ�ɹ���ת��step-05.\n";
        } else {
            CloseHandle(hVol);
            return false;
        }


        if (0 != status) {
            cout << "��ȡUSN��־������Ϣ�ɹ���ת��step-05.\n";
            getBasicInfoSuccess = true;
        } else
            cout << "��ȡUSN��־������Ϣʧ�� ���� status:" << status << " error:" << GetLastError() << "\n";
        if (getBasicInfoSuccess) {
            cout << "UsnJournalID: " << UsnInfo.UsnJournalID << "\n";
            cout << "lowUsn: " << UsnInfo.FirstUsn << "\n";
            cout << "highUsn: " << UsnInfo.NextUsn << "\n";

            //step 05. ö��USN��־�ļ��е����м�¼
            cout << "step 05. ö��USN��־�ļ��е����м�¼\n";
            MFT_ENUM_DATA med;
            med.StartFileReferenceNumber = 0;
            med.LowUsn = 0;
            med.HighUsn = journal_info.high_usn;

            CHAR buffer[BUF_LEN]; //�����¼�Ļ���,�����㹻�ش� buf_len = 4096
            DWORD usnDataSize;
            PUSN_RECORD UsnRecord;
            long clock_start = clock();

            USN_JOURNAL_DATA ujd;

            while (0 != DeviceIoControl(hVol,
                                        FSCTL_ENUM_USN_DATA,
                                        &med,
                                        sizeof(med),
                                        buffer,
                                        BUF_LEN,
                                        &usnDataSize,
                                        NULL)) {
                DWORD dwRetBytes = usnDataSize - sizeof(USN);

                UsnRecord = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN));// �ҵ���һ��USN��¼
                while (dwRetBytes > 0) {
                    const int strLen = UsnRecord->FileNameLength;
                    char fileName[MAX_PATH] = { 0 };
                    //char filePath[MAX_PATH] = {0};
                    WideCharToMultiByte(CP_OEMCP, NULL, UsnRecord->FileName, strLen / 2, fileName, strLen, NULL, FALSE);

                    //cout << "FileName: " << fileName << "\n";
                    //cout << "FileReferenceNumber: " << UsnRecord->FileReferenceNumber << "\n";
                    //cout << "ParentFileReferenceNumber: " << UsnRecord->ParentFileReferenceNumber << "\n";
                    ////cout<< "FilePath: " << filePath << "\n\n";

                    fout << "FileName:" << fileName << endl;
                    fout << "FileReferenceNumber:" << UsnRecord->FileReferenceNumber << endl;
                    fout << "ParentFileReferenceNumber:" << UsnRecord->ParentFileReferenceNumber << endl;
                    //fout << "FilePath:" << filePath << endl;
                    fout << endl;
                    counter++;

                    // ��ȡ��һ����¼
                    DWORD recordLen = UsnRecord->RecordLength;
                    dwRetBytes -= recordLen;
                    UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + recordLen);
                }
                med.StartFileReferenceNumber = *(USN*)&buffer;

            }
            cout << "��" << counter << "���ļ�\n";
            long clock_end = clock();
            cout << "����" << clock_end - clock_start << "����" << endl;
            fout << "��" << counter << "���ļ�" << endl;
            fout << flush;
            fout.close();
        }


        //step 06. ɾ��USN��־�ļ�(��ȻҲ���Բ�ɾ��)
        cout << "step 06. ɾ��USN��־�ļ�(��ȻҲ���Բ�ɾ��)\n";
        DELETE_USN_JOURNAL_DATA dujd;
        dujd.UsnJournalID = UsnInfo.UsnJournalID;
        dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;

        status = DeviceIoControl(hVol,
                                 FSCTL_DELETE_USN_JOURNAL,
                                 &dujd,
                                 sizeof(dujd),
                                 NULL,
                                 0,
                                 &br,
                                 NULL);

        if (0 != status)
            cout << "�ɹ�ɾ��USN��־�ļ�!\n";
        else
            cout << "ɾ��USN��־�ļ�ʧ�� ���� status:" << status << " error:" << GetLastError() << "\n";
    }
    if (getHandleSuccess)
        CloseHandle(hVol);
    //�ͷ���Դ




    //usn_manager usn;
    //usn.start();


    //   char* volName = (char*)"w:\\";
    //   // memset(volName, 0, sizeof(volName)/sizeof(char *));
    //   HANDLE hVol = new HANDLE;
    //USN_JOURNAL_DATA UsnInfo = {}; // ����USN��־�Ļ�����Ϣ

    //   usn.watch_usn(volName, hVol, UsnInfo);
}

// ���г���: Ctrl + F5 ����� >����ʼִ��(������)���˵�
// ���Գ���: F5 ����� >����ʼ���ԡ��˵�

// ����ʹ�ü���:
//   1. ʹ�ý��������Դ�������������/�����ļ�
//   2. ʹ���Ŷ���Դ�������������ӵ�Դ�������
//   3. ʹ��������ڲ鿴���������������Ϣ
//   4. ʹ�ô����б��ڲ鿴����
//   5. ת������Ŀ��>���������Դ����µĴ����ļ�����ת������Ŀ��>�����������Խ����д����ļ���ӵ���Ŀ
//   6. ��������Ҫ�ٴδ򿪴���Ŀ����ת�����ļ���>���򿪡�>����Ŀ����ѡ�� .sln �ļ�
