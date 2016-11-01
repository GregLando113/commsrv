#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <functional>

namespace npcomm{

	class PipeServer;
	class PipeConnection; 


	template <class T>
	using CommHandler = void(__fastcall *)(T request, PipeConnection* reciever);


	struct BaseRequest {
		DWORD cmd;
	};
		

	class PipeConnection 
	{
	public:
		static const DWORD BUF_SIZE = 0x1000;

	public:
		PipeConnection(HANDLE pipe, PipeServer* parent);

		void Close();

		void HandleCommands(BaseRequest* req);

		DWORD MainRoutine();

		inline HANDLE threadhandle() const { return threadhandle_; }
		inline PipeServer* parent() const { return parent_; }

		// send reply by raw buffer
		void Send(void* buffer, size_t size);

		// send reply by value
		template <class _Reply>
		void Send(_Reply reply){
			DWORD lastreply;
			WriteFile(pipehandle_, &reply, sizeof(_Reply),
				&lastreply, NULL);
		}

		HANDLE pipehandle() const { return pipehandle_; }

	private:

		char 		requestbuffer_[BUF_SIZE];
		PipeServer* parent_;
		HANDLE 		pipehandle_;
		HANDLE 		threadhandle_;
	};


	class PipeServer 
	{
		friend class PipeConnection;

	public:

		PipeServer(std::wstring pipename,size_t opcount);
		~PipeServer();

		template <class _Handler>
		void RegisterHandler(DWORD command, CommHandler<_Handler*> handler)
		{
			handlers_[command] = [handler](BaseRequest* request, PipeConnection* reciever){
				handler((_Handler*)request, reciever);
			};
		}

		DWORD MainRoutine();
	private:
		
		std::wstring 													pipename_;
		HANDLE 															pipehandle_;
		HANDLE 															threadhandle_;
		std::vector<PipeConnection*> 									openconnections_;
		std::vector<std::function<void(BaseRequest*, PipeConnection*)>> handlers_;
	};

}