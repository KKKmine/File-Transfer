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
	cout << "<1>�ǰe�ɮ�-Server��" << endl;
	cout << "<2>�����ɮ�-Client��" << endl;
	cin >> c;
	if (c == '1'){
		server();
	}
	else if (c == '2'){
		client();
	}
	else{
		cout << "���~�ﶵ 87" << endl;
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

	cout << "---Server��---" << endl;
	while (1)
	{
		cout << "�إ�Server��..." << endl;
		if (!server_socket.Listen(PORT)) {
			cout << "�إ�Server����" << endl;
			Sleep(1000);
			continue;
		}
		else{
			cout << "�إ�Server���\" << endl;
		}
		if (server_socket.GetHostIP(NULL, ip1, ip2, ip3, ip4)){
			cout << "����Ip : " << ip1 << "." << ip2 << "." << ip3 << "." << ip4 << endl;
		}
		cout << "�ǰe���ɮצW��" << endl;
		cin.ignore();
		getline(cin, filename);
		if (!input.in_open(filename)){
			cout << "�䤣���ɮ�" << endl; //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			continue;
		}
		cout << "�ɮפj�p " << input.showSize() << endl;
		cout << "�ǳƦn�ǰe" << endl;

		while (1)
		{
			SOCKET socket;
			while (!server_socket.Accept(socket)){
				cout << "�PClient�s�u����" << endl;
			}
			cout << "�PClient�s�u���\" << endl;

			WgSocket client_socket;
			client_socket.SetSocket(socket);
			if (!input.sendFileDetail(client_socket)){
				cout << "�ǰe�ɮ׸Ա�����" << endl;
				client_socket.Close();
				continue;
			}
			cout << "�ǰe��..." << endl;
			clock_t t = clock();

			if (input.send(client_socket)){
				cout << "�ǰe����" << endl;
				cout << "�ΤF " << (float)(clock() - t) / CLOCKS_PER_SEC << " ��" << endl;
			}
			else {
				cout << "�ǰe����" << endl;
				client_socket.Close();
				continue;
			}
			client_socket.Close();
			cout << "�PClient�_�u" << endl;
		}
		break;
	}
}


void client()
{
	File output;
	string hostip;

	cout << "---Client��---" << endl;
	cout << "��JSever Ip" << endl;
	cin >> hostip;
	while (1)
	{
		WgSocket socket;
		cout << "�s�u��Server��..." << endl;
		if (!socket.Open(hostip.c_str(), PORT))
		{
			cout << "�s�u��Server����" << endl;
			Sleep(1000);
			continue;
		}
		cout << "�s�u��Server���\" << endl;

		if (!output.recvFileDetail(socket)){
			cout << "�����ɮ׸Ա�����" << endl;
			socket.Close();
		}
		cout << "�ɮצW�� " << output.getNmae() << endl;
		cout << "�ɮפj�p " << output.showSize() << endl;

		string filepath;
		while (1)
		{
			filepath.clear();
			filepath = output.getNmae();
			if (!output.out_open(filepath)){
				cout << "�s�ɥ���" << endl;
				continue;
			}
			else{
				break;
			}
		}
		cout << "������..." << endl;
		clock_t t = clock();

		if (output.recv(socket)){
			cout << "��������" << endl;
			cout << "�ΤF " << (float)(clock() - t) / CLOCKS_PER_SEC << " ��" << endl;
			output.close();
			socket.Close();
		}
		else{
			cout << "��������" << endl;
			output.close();
			socket.Close();
			system("pause");
			continue;
		}
		break;
	}
}