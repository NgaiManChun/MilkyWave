// =======================================================
// CSVResource.cpp
// 
// CSV�t�@�C����ǂݍ��݊֐�
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/08/06
// 2024/12/09 �ǂݍ��ݎ���reserve�����ǉ�����
// =======================================================
#include "CSVResource.h"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "windows.h"

using std::string;
using std::map;
using std::vector;

void ReadCSVDataLine(string& line, D_TABLE& result);

void ReadCSVFromResource(int resourceId, D_TABLE& result) {
    // ���\�[�X�n���h���̎擾
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(resourceId), L"CSV");
    if (hRes == NULL) {
        return;
    }

    // ���\�[�X�̃��[�h
    HGLOBAL hData = LoadResource(hModule, hRes);
    if (hData == NULL) {
        return;
    }

    // ���\�[�X�f�[�^�̃��b�N
    LPVOID pData = LockResource(hData);
    if (pData == NULL) {
        return;
    }

    // ���\�[�X�T�C�Y�̎擾
    DWORD dataSize = SizeofResource(hModule, hRes);
    if (dataSize == 0) {
        return;
    }

    // ���\�[�X�f�[�^�𕶎���ɕϊ�
    string csvData(static_cast<char*>(pData), dataSize);

    ReadCSVData(csvData, result);
}

void ReadCSVFromPath(string path, D_TABLE& result) {
    std::ifstream file(path);
    std::string line;
    if (file.is_open()) {
        unsigned int lineNum = 0;
        while (getline(file, line)) {
            lineNum++;
        }
        result.reserve(lineNum);
        file.clear();
        file.seekg(0);
        while (getline(file, line)) {
            ReadCSVDataLine(line, result);
        }
    }
}

void ReadCSVData(string &csvData, D_TABLE &result) {
    std::stringstream ss(csvData);
    string line;
    unsigned int lineNum = 0;
    while (getline(ss, line)) {
        lineNum++;
    }
    result.reserve(lineNum);
    ss.clear();
    ss.seekg(0);
    while (getline(ss, line)) {
        ReadCSVDataLine(line, result);
    }
}

void ReadCSVDataLine(string &line, D_TABLE& result) {
    std::stringstream lineStream(line);
    string cell;
    D_ROW row;

    unsigned int colNum = 0;
    while (getline(lineStream, cell, ',')) {
        colNum++;
    }
    row.reserve(colNum);
    lineStream.clear();
    lineStream.seekg(0);
    while (getline(lineStream, cell, ',')) {
        row.push_back(cell.data());
    }
    result.push_back(row);
}

void TableToKeyValuePair(const string key, D_TABLE &data_table, D_KVTABLE &result) {
    D_ROW header = data_table[0];
    int keyIndex = 0;
    for (int i = 0; i < header.size(); i++) {
        if (header[i] == key) {
            keyIndex = i;
            break;
        }
    }
    for (int y = 1; y < data_table.size(); y++) {
        D_KVPAIR pair;
        for (int x = 0; x < data_table[y].size(); x++) {
            pair[header[x]] = data_table[y][x];
        }
        result[data_table[y][keyIndex]] = pair;
    }
}