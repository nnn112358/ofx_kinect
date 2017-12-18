#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<windows.h>

using namespace std;


//フォルダ内のファイルを列挙
void get_filelist(string path, string filenamelist[], long &nn_max, string f_type) {
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	//	FILETIME ft;
	//	SYSTEMTIME st;

	string type = "*" + f_type;
	path = path + type;

	/* 最初のファイル検索 */
	hFind = FindFirstFile(path.c_str(), &fd);

	/* 検索失敗? */
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("検索失敗\n");
		return; /******** エラー終了 ********/
	}
	int num = 0;

	do {
		/* 結果の表示 */
		printf("ファイル名: %s\n", fd.cFileName);
		if (num<1000)
			filenamelist[num++] = fd.cFileName;

	} while (FindNextFile(hFind, &fd));

	nn_max = num;
	/* 検索終了 */
	FindClose(hFind);
}