#include "PipeSrv.h"

npcomm::PipeServer::PipeServer(std::wstring pipename,size_t opcount)
	: pipename_(pipename), 
	  pipehandle_(INVALID_HANDLE_VALUE), 
	  handlers_(std::vector<std::function<void(BaseRequest*, PipeConnection*)>>(opcount))
	{
	
}

npcomm::PipeServer::~PipeServer() {
	for (PipeConnection* connection : openconnections_) {
		connection->Close();
		delete connection;
	}
}

DWORD 
npcomm::PipeServer::MainRoutine() {
	while (1) {
		pipehandle_ = CreateNamedPipeW(
			pipename_.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			0x1000,
			0x1000,
			0,
			NULL);

		if (pipehandle_ == INVALID_HANDLE_VALUE) {
			return 1;
		}

		if ( ConnectNamedPipe(pipehandle_, NULL) ? 
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)){
			openconnections_.push_back(new PipeConnection(pipehandle_, this));
		} else {
			CloseHandle(pipehandle_);
		}

		Sleep(5);
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////

DWORD WINAPI 
ThreadEntry(npcomm::PipeConnection* thisptr) {
	return thisptr->MainRoutine();
}

npcomm::PipeConnection::PipeConnection(HANDLE pipe, PipeServer* parent) : parent_(parent), pipehandle_(pipe) {
	threadhandle_ = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadEntry, this, 0, 0);
}


DWORD 
npcomm::PipeConnection::MainRoutine() {
	DWORD lastrequestsize;
	while (1) {
		if (ReadFile(pipehandle_, requestbuffer_, BUF_SIZE, &lastrequestsize, NULL)) {
			[this](BaseRequest* request){
				auto func = parent_->handlers_[request->cmd];
				if(func != NULL)
					func(request, this);
			}((BaseRequest*)requestbuffer_);
		}
		Sleep(10);
	}
	return TRUE;
}

void 
npcomm::PipeConnection::Close() {

	TerminateThread(threadhandle_, EXIT_SUCCESS);

	if (pipehandle_) 
	{ 
		DisconnectNamedPipe(pipehandle_);
		CloseHandle(pipehandle_);
	}

}



void 
npcomm::PipeConnection::Send(void* buffer, size_t size) {
	DWORD lastreplysize;
	WriteFile(pipehandle_, buffer, size,
		&lastreplysize, NULL);
}