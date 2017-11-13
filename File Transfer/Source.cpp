#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
#include <time.h>
#include <WinSock2.h>
#include "WgSocket.h"
#include "File.h"
using namespace std;
#define PORT 7200

void server();
void client();

int main(int argc, char* argv[])
{
	char c;
	cout << "<1>傳送檔案-Server端" << endl;
	cout << "<2>接收檔案-Client端" << endl;
	cin >> c;
	if (c == '1'){
		server();
	}
	else if (c == '2'){
		client();
	}
	else{
		cout << "錯誤選項 87" << endl;
	}

	system("pause");
	return 0;
}

void server()
{
	WgSocket server_socket;
	File input;
	int ip1, ip2, ip3, ip4;
	string filename;

	cout << "---Server端---" << endl;
	while (1)
	{
		cout << "建立Server中..." << endl;
		if (!server_socket.Listen(PORT)) {
			cout << "建立Server失敗" << endl;
			Sleep(1000);
			continue;
		}
		else{
			cout << "建立Server成功" << endl;
		}
		if (server_socket.GetHostIP(NULL, ip1, ip2, ip3, ip4)){
			cout << "本機Ip : " << ip1 << "." << ip2 << "." << ip3 << "." << ip4 << endl;
		}
		cout << "傳送的檔案名稱" << endl;
		cin.ignore();
		getline(cin, filename);
		if (!input.in_open(filename)){
			cout << "找不到檔案" << endl; //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			continue;
		}
		cout << "檔案大小 " << input.showSize() << endl;
		cout << "準備好傳送" << endl;

		while (1)
		{
			SOCKET socket;
			while (!server_socket.Accept(socket)){
				cout << "與Client連線失敗" << endl;
			}
			cout << "與Client連線成功" << endl;

			WgSocket client_socket;
			client_socket.SetSocket(socket);
			if (!input.sendFileDetail(client_socket)){
				cout << "傳送檔案詳情失敗" << endl;
				client_socket.Close();
				continue;
			}
			cout << "傳送中..." << endl;
			clock_t t = clock();

			if (input.send(client_socket)){
				cout << "傳送完成" << endl;
				cout << "用了 " << (float)(clock() - t) / CLOCKS_PER_SEC << " 秒" << endl;
			}
			else {
				cout << "傳送失敗" << endl;
				client_socket.Close();
				continue;
			}
			client_socket.Close();
			cout << "與Client斷線" << endl;
		}
		break;
	}
}


void client()
{
	File output;
	string hostip;

	cout << "---Client端---" << endl;
	cout << "輸入Sever Ip" << endl;
	cin >> hostip;
	while (1)
	{
		WgSocket socket;
		cout << "連線至Server中..." << endl;
		if (!socket.Open(hostip.c_str(), PORT))
		{
			cout << "連線至Server失敗" << endl;
			Sleep(1000);
			continue;
		}
		cout << "連線至Server成功" << endl;

		if (!output.recvFileDetail(socket)){
			cout << "接收檔案詳情失敗" << endl;
			socket.Close();
		}
		cout << "檔案名稱 " << output.getNmae() << endl;
		cout << "檔案大小 " << output.showSize() << endl;

		string filepath;
		while (1)
		{
			filepath.clear();
			filepath = output.getNmae();
			if (!output.out_open(filepath)){
				cout << "存檔失敗" << endl;
				continue;
			}
			else{
				break;
			}
		}
		cout << "接收中..." << endl;
		clock_t t = clock();

		if (output.recv(socket)){
			cout << "接收完成" << endl;
			cout << "用了 " << (float)(clock() - t) / CLOCKS_PER_SEC << " 秒" << endl;
			output.close();
			socket.Close();
		}
		else{
			cout << "接收失敗" << endl;
			output.close();
			socket.Close();
			system("pause");
			continue;
		}
		break;
	}
}