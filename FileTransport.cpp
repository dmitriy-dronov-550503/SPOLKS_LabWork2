#include "stdafx.h"
#include "FileTransport.h"
#include "LogSystem.h"

FileTransport::FileTransport(socket_ptr sock_in)
{
	sock = sock_in;
}

#define OUTPUT_UPDATE_COUNT 1000

class ShowSpeed
{
private:
	time_point<steady_clock, nanoseconds> start;
	int64_t& fileSizeLeft;
	const int64_t fileSize;
	string output;
	uint32_t lastChordPos;
	char ch;
public:
	ShowSpeed(time_point<steady_clock, nanoseconds> inStart, int64_t& inFileSizeLeft, const int64_t inFileSize)
		: start(inStart), fileSizeLeft(inFileSizeLeft), fileSize(inFileSize)
	{
		output.reserve(256);
		output = '[' + string(50, ' ') + ']';
		lastChordPos = 0;
		ch = 249;
	}

	void show()
	{
		std::cout << '\r';
		time_point<steady_clock, nanoseconds> finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		//std::cout << "Average speed: " << ((chunkCount * chunkSize) / elapsed.count()) / (1024 * 1024) << " MB/s";

		double percentReady = (1.0 - ((double)fileSizeLeft / fileSize));

		//ch = rand()%256;
		/*if (percentReady < 0.33) ch = 250;
		else if (percentReady < 0.66) ch = 249;
		else if (percentReady <= 1.0) ch = 219;*/

		uint32_t percents = (uint32_t)(percentReady * 100);
		uint32_t chords = (uint32_t)(percentReady * 50);
		for (uint32_t i = lastChordPos; i < chords; i++)
		{
			output[i+1] = ch;
		}
		lastChordPos = chords;

		string perc = to_string(percents) + " %";
		for (int i = 0; i < perc.length(); i++)
		{
			output[25 + i] = perc[i];
		}

		int MBs = (fileSize - fileSizeLeft) / elapsed.count() / 1024 / 1024;
		cout << output << to_string(MBs) + " MB/s";
	}
};

void FileTransport::Send(string filenameFrom, string filenameTo)
{
	data = new char[sendBufferSize + 1];

	if (data != nullptr)
	{
		sock->receive(data, sendBufferSize);
		int64_t downloadedSize = std::stoi(data);
		cout << "Download stopped at = " << downloadedSize << endl;
		sock->send("GOT FILESIZE");

		// Open the file
		file.open(filenameFrom, ios::in | ios::binary);

		// Wait receiver ready
		sock->receive(data, sendBufferSize);

		if (string(data) == "I'AM READY")
		{
			cout << "GOT READY" << endl;

			if (file.is_open())
			{
				// Seek
				file.seekg(downloadedSize);
				cout << "Start upload from " << downloadedSize << endl;

				// Send filesize
				int64_t fileSize = SendFileSize(filenameFrom);
				cout << "Filesize = " << data << endl;

				if (fileSize != 0)
				{
					start = std::chrono::high_resolution_clock::now();
					int64_t fileSizeLeft = fileSize - downloadedSize;
					int outputCounter = 0;
					ShowSpeed speed(start, fileSizeLeft, fileSize);
					speed.show();

					try
					{
						// Send file content
						while (true)
						{
							outputCounter++;
							if (outputCounter == OUTPUT_UPDATE_COUNT)
							{
								outputCounter = 0;
								speed.show();
							}

							file.read(data, sendBufferSize);

							uint32_t packetSize = file.gcount();

							size_t sendedSize = sock->send(data, packetSize);

							fileSizeLeft -= sendedSize;

							//cout << "FileSizeLeft = " << fileSizeLeft << "\tSendedSize = " << sendedSize << endl;

							if (packetSize < sendBufferSize)
							{
								speed.show();
								break;
							}
						}
					}
					catch (...)
					{
						cout << endl  << "Transfer was interrupted" << endl;
						file.close();
						delete data;
					}
					cout << endl;

					file.close();

					sock->receive(data, sendBufferSize);

					if (string(data) == "File received")
					{
						cout << endl << "File has been successfully sent" << endl;
					}
				}
				else
				{
					cout << "Empty file, nothing to send" << endl;
				}
			}
			else
			{
				data[0] = '0';
				data[1] = '\0';
				sock->send(data, 2);
				cout << "Filesize = " << data << endl;
				cout << "Can't open file" << endl;
			}
		}
		else
		{
			cout << "Reciever isn't ready. Got data = "<< data << endl;
		}

		delete data;
	}
	else
	{
		cout << "Can't allocate send buffer" << endl;
	}
}

int64_t FileTransport::SendFileSize(string fileName)
{
	ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
	int64_t fileSize = file.tellg();
	file.close();
	string fileSizeStr = std::to_string(fileSize);
	int zeroPos = fileSizeStr.size();
	strcpy_s(data, sendBufferSize, fileSizeStr.c_str());
	data[zeroPos] = '\0';

	sock->send(data, sendBufferSize);

	return fileSize;
}

void FileTransport::Receive(string filenameFrom, string filenameTo)
{
	data = new char[receiveBufferSize + 1];

	remove(filenameTo.c_str());

	string remoteAddress = sock->getRemoteIp();
	string fileDownload = filenameTo + remoteAddress + ".download";

	if (data != nullptr)
	{
		uint32_t downloadedSize = 0;
		ifstream fd(fileDownload);

		if (fd.good())
		{
			downloadedSize = SendFileSize(fileDownload);
			fd.close();
		}
		else
		{
			sock->send("0");
		}

		sock->receive(data, receiveBufferSize);
		cout << data << endl;

		if (string(data) != "GOT FILESIZE")
		{
			delete data;
			return;
		}

		// Create the file
		file.open(fileDownload, ios::out | ios::app | ios::binary);

		if (file.is_open())
		{
			file.seekp(downloadedSize);
			cout  << "Start download from " << downloadedSize << endl;
			sock->send("I'AM READY");

			// Get filesize
			//--------------------------------------------------------
			sock->receive(data, receiveBufferSize);
			int64_t fileSize = std::stoi(data);
			cout << "Filesize = " << fileSize << endl;

			int64_t fileSizeLeft = fileSize - downloadedSize;

			if (fileSizeLeft == 0)
			{
				cout << "Error cause on sent size" << endl;
			}
			else
			{
				start = std::chrono::high_resolution_clock::now();
				int outCounter = 0;
				ShowSpeed speed(start, fileSizeLeft, fileSize);
				speed.show();

				try
				{
					// Get file content
					while (fileSizeLeft > 0)
					{
						outCounter++;
						if (outCounter == OUTPUT_UPDATE_COUNT)
						{
							outCounter = 0;
							speed.show();
						}

						// Get packet
						int64_t readedSize = sock->receive(data, receiveBufferSize);

						fileSizeLeft -= readedSize;

						//cout << "FileSizeLeft = " << fileSizeLeft << "\tReadedSize = " << readedSize << endl;

						file.write(data, readedSize);
					}

					speed.show();
				}
				catch (...)
				{
					cout << endl << "Transfer was interrupted" << endl;
					file.close();
					delete data;
				}

				file.close();

				sock->send("File received");

				int err = rename(fileDownload.c_str(), filenameTo.c_str());

				if (err)
				{
					cout << endl << "Can't rename file" << endl;
				}
				else
				{
					cout << endl << "File renamed from " << fileDownload.c_str() << " to " << filenameTo.c_str() << endl;
				}

				cout << "File has been successfully received" << endl;
			}
		}
		else
		{
			sock->send("I'AM NOT READY");
			cout << "Can't open file" << endl;
		}

		delete data;
	}
	else
	{
		cout << "Can't allocate receive buffer" << endl;
	}
}
