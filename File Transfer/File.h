#ifndef File_H
#define File_H
#include <fstream>
#include <sstream>
#include "WgSocket.h"
using namespace std;
#define BYTE_PER_SEND 1 //多傳??

class File
{
private:
	fstream stream;
	long size;
	string name;
public:
	bool in_open(string);
	bool out_open(string);
	void sendSign(WgSocket&);
	bool recvSign(WgSocket&);
	bool sendFileDetail(WgSocket&);
	bool recvFileDetail(WgSocket&);
	bool send(WgSocket&);
	bool recv(WgSocket&);
	void close();

	long fileSize();
	string showSize() const;
	inline long getSize() const{ return size; }
	inline string getNmae() const{ return name; }
};

bool File::in_open(string file_name)
{
	stream.close();
	stream.open(file_name, ios::in | ios::binary);
	if (!stream.is_open()){
		return false;
	}
	name = file_name;
	fileSize();
	return true;
}

bool File::out_open(string file_name)
{
	stream.close();
	stream.open(file_name, ios::out | ios::binary);
	if (!stream.is_open()){
		return false;
	}
	return true;
}

bool File::sendFileDetail(WgSocket& wgsocket)
{
	stringstream str_size;
	char sign;
	long ret_len;
	if (!wgsocket.Write(name.c_str(), name.length())){
		return false;
	}
	str_size << size;
	if (!recvSign(wgsocket)){
		return false;
	}


	if(!wgsocket.Write(str_size.str().c_str(), str_size.str().length())){
		return false;
	}
	if (!recvSign(wgsocket)){
		return false;
	}
	return true;
}

bool File::recvFileDetail(WgSocket& wgsocket)
{
	stringstream str_size;
	char data[255];
	long ret_len;
	if (!wgsocket.Read(data, 255, ret_len)){
		return false;
	}
	name = data;
	name.resize(ret_len);
	memset(data, 0, 32 * sizeof(char));
	sendSign(wgsocket);

	if (!wgsocket.Read(data, 32, ret_len)){
		return false;
	}
	str_size << data;
	str_size >> size;
	sendSign(wgsocket);
	return true;
}

bool File::send(WgSocket& wgsocket)
{
	stream.seekg(0, ios::beg);
	//while (!stream.eof())
	while (stream.tellg() < size)
	{
		char data[BYTE_PER_SEND];
		stream.read(data, BYTE_PER_SEND);
		if (!wgsocket.Write(data, BYTE_PER_SEND)){
			return false;
		}
	}
	if (!recvSign(wgsocket)){
		return false;
	}
	return true;
}

bool File::recv(WgSocket& wgsocket)
{
	stream.seekp(0, ios::beg);
	while (1)
	{
		char data[BYTE_PER_SEND];
		long ret_len;
		if (!wgsocket.WaitInputData(10)) // 10秒沒回應便斷線 //
			break;

		if (!wgsocket.Read(data, BYTE_PER_SEND, ret_len)){
			return false;
		}
		if (ret_len == 0){ // 對方已斷線 //
			break;
		}
		stream.write(data, BYTE_PER_SEND);
	}
	sendSign(wgsocket);
	if ((int)stream.tellg() != size){
		cout << "接收了 " << stream.tellg() << "Byte 應該有 " << size << " Byte" << endl;
		return false;
	}
	return true;
}

void File::close()
{
	stream.close();
}

long File::fileSize()
{
	stream.seekg(0, ios::end);
	size = stream.tellg();
	stream.seekg(0, ios::beg);
	return size;
}

string File::showSize() const
{
	stringstream str_size;
	if (size <  (1 << 10)){
		str_size << size << "  B";
	}
	else if (size < (1 << 20)){
		str_size << (size >> 10) << " KB";
	}
	else if (size <  (1 << 30)){
		str_size << (size >> 20) << " MB";
	}
	else {
		str_size << (size >> 30) << " GB";
	}
	return str_size.str();
}

void File::sendSign(WgSocket& wgsocket)
{
	const char sign = 'Y';
	wgsocket.Write(&sign, 1);
}

bool File::recvSign(WgSocket& wgsocket)
{
	char sign;
	long ret_len;
	if (!wgsocket.Read(&sign, 1, ret_len)){
		return false;
	}
	if (sign != 'Y' || ret_len != 1){
		return false;
	}
	return true;
}

#endif