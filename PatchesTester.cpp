//
// Project name: Patches Tester
// Date of the project: 2019.1.27
// The author : K0rz3n
// Blog : www.k0rz3n.com
//

#include "stdafx.h"
#include "PatchesTester.h"
#include <Windows.h>
#include <iostream>
#include <atlbase.h>
#include <Wuapi.h>
#include <wuerror.h>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <atlbase.h>
#include <atlstr.h>
#include <comutil.h>
#include <MsXml.h>
#include <map>
#include <list>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//指定 options 的数量
const unsigned int optionnum = 5;
CWinApp theApp;//windows 操作系统的应用程序的初始化

using namespace std;

int writeIntotmp(string &outputtmp, BSTR KBValue, BSTR updateName, BSTR level)
{
	USES_CONVERSION;
	stringstream tmp;
	tmp << W2A(CString("KB")) << W2A(CString(KBValue)) << " --- " << W2A(CString(level)) << " --- " << W2A(CString(updateName)) << endl;
	outputtmp = tmp.str();
	return 0;
}


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])//_tmain 是 main 的 unicode 扩展
{
	wcout << " ____       _       _                 _____         _              " << endl;
	wcout << "|  _ \ __ _| |_ ___| |__   ___  ___  |_   _|__  ___| |_ ___ _ __   " << endl;
	wcout << "| |_) / _` | __/ __| '_ \ / _ \/ __|   | |/ _ \/ __| __/ _ \ '__|  " << endl;
	wcout << "|  __/ (_| | || (__| | | |  __/\__ \   | |  __/\__ \ ||  __/ |     " << endl;
	wcout << "|_|   \__,_|\__\___|_| |_|\___||___/   |_|\___||___/\__\___|_|     " << endl;
	wcout << "                                                                   " << endl;
	wcout << "                                                                   " << endl;
	wcout << "                                              The author : K0rz3n  " << endl;
	wcout << "                                              Blog : www.k0rz3n.com" << endl;



	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);//获取  exe 的基址

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("ERROR: MFC Initialization Failed\n"));
			nRetCode = 1;
		}
		else
		{
			try
			{
				HRESULT hr;
				hr = CoInitialize(NULL);//初始化 COM 平台
				const char * filename = "result.txt";
				IUpdateSession* iUpdate;
				IUpdateSearcher* searcher;
				ISearchResult* results;

				//设置查询条件https://msdn.microsoft.com/en-us/library/windows/desktop/aa386526%28v=vs.85%29.aspx
				BSTR criteria = SysAllocString(L"IsInstalled=0 "); // and IsHidden=0 or IsPresent=1
				//创建 COM 服务器组件实例，获得组件接口指针
				hr = CoCreateInstance(CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&iUpdate);
				hr = iUpdate->CreateUpdateSearcher(&searcher);//返回此会话的IUpdateSearcher接口
				hr = searcher->put_ServerSelection(ssWindowsUpdate);//该值指示要搜索更新的服务器
				hr = searcher->put_CanAutomaticallyUpgradeService(VARIANT_TRUE);//此属性的有效值对应于对该方法的调用不自动升级WUA的选项
				hr = searcher->put_IncludePotentiallySupersededUpdates(VARIANT_TRUE);//包含由搜索结果中的其他更新取代的更新
				hr = searcher->put_Online(VARIANT_TRUE);//是否联机搜索更新


				wcout << L"Searching for uninstalled patches ..." << endl;
				hr = searcher->Search(criteria, &results); //如果成功则返回S_OK 否则，返回COM或Windows错误代码
				SysFreeString(criteria);

				switch (hr)
				{
				case S_OK:
					break;
				case WU_E_LEGACYSERVER://可选服务
					wcout << L"No server selection enabled" << endl;
					return 0;
				case WU_E_INVALID_CRITERIA://无效的标准
					wcout << L"Invalid search criteria" << endl;
					return 0;
				}

				IUpdateCollection *updateList;
				IUpdate *updateItem;
				LONG updateSize;
				LONG totalKB = 0;
				results->get_Updates(&updateList);//获取更新
				updateList->get_Count(&updateSize);//获取数量

				if (updateSize == 0)
				{
					wcout << L"No uninstalled patches found" << endl;
					return 0;
				}

				wcout << "Uninstalled patches found and the total KBs : " << updateSize << endl;
				wcout << "There are five levels of patches you can choose to output: [a]Critical [b]Important [c]Moderate [d]Low [e]Undefined  " << endl;
				wcout << "Input format: " << endl;
				wcout << "Example: 11010 means to choose [a][b][d]" << endl;

				string inputNum;
				string myoptions[optionnum] = { "Critical","Important","Moderate","Low","Undefined" };
				std::map<string, list<string>> optset;

				while (1)
				{
					wcout << "Now input num: ";
					cin >> inputNum;
					if (inputNum.length() != optionnum)
						wcout << "Input format error" << endl;
					else
					{
						bool flag = true;
						for (int i = 0; i < optionnum; i++)
							if (inputNum[i] != '1' && inputNum[i] != '0')
							{
								flag = false;
								break;
							}
						if (flag)
							break;
					}
				}
				list<string> *p = NULL;
				for (int i = 0; i < optionnum; i++)
					if (inputNum[i] == '1')
					{
						p = new list<string>;
						optset.insert(pair<string, list<string>>(myoptions[i], *p));
					}


				ofstream outputFile;
				outputFile.open(filename, ios::out);

				for (LONG i = 0; i < updateSize; i++)
				{
					IStringCollection *KBCollection;
					BSTR updateName;
					LONG KBCount;
					BSTR level = NULL;
					updateList->get_Item(i, &updateItem);
					updateItem->get_Title(&updateName); //获取补丁标题
					updateItem->get_KBArticleIDs(&KBCollection);
					KBCollection->get_Count(&KBCount);
					for (int i = 0; i < KBCount; i++)
					{
						BSTR KBValue;
						totalKB += 1;
						KBCollection->get_Item(i, &KBValue);
#ifdef _DEBUG
						level = SysAllocString(L"Important");
#else
						updateItem->get_MsrcSeverity(&level);
#endif
						map<string, list<string>>::iterator iter;
						if (level == NULL)
							level = SysAllocString(L"Undefined");
						string levelStr = _com_util::ConvertBSTRToString(level);
						SysFreeString(level);
						if ((iter = optset.find(levelStr)) != optset.end())
						{
							string tmp;
							writeIntotmp(tmp, KBValue, updateName, level);
							iter->second.push_back(tmp);
						}
					}
				}
				for (int i = 0; i < optionnum; i++)
				{
					map<string, list<string>>::iterator iter;
					//按照 myoptions 的顺序
					if (((iter = optset.find(myoptions[i])) != optset.end()) && (iter->second.size() > 0))
					{
						for (auto j = iter->second.begin(); j != iter->second.end(); j++)
							outputFile << j->data();
					}
				}
				outputFile.close();
				::CoUninitialize();
			}
			catch (const std::exception & ex)
			{
				cout << ex.what();
				::CoUninitialize();
			}
		}
	}
	else
	{
		_tprintf(_T("ERROR: GetModuleHandle Failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
