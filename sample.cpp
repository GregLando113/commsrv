#include <Windows.h>
#include <cstdio>
#include "PipeSrv.h"

using namespace npcomm;

struct PipePak1 : public BaseRequest {
};

struct PipePak2 : public BaseRequest {
	DWORD age;
};

struct PipePak3 : public BaseRequest {
	char name[0x10];
};


int main() {
	PipeServer pipe(L"\\\\.\\pipe\\testpipe_lel", 3);

	pipe.RegisterHandler<PipePak1>(0, [](PipePak1* pak, PipeConnection* conn){
		printf("Hello, I see you client! :D\n");
		conn->Send<DWORD>(1);
	});
	pipe.RegisterHandler<PipePak2>(1, [](PipePak2* pak, PipeConnection* conn){
		printf("Recieved age %d. Sending back birthday age :D\n",pak->age);
		conn->Send<DWORD>(pak->age + 1);
	});
	pipe.RegisterHandler<PipePak3>(2, [](PipePak3* pak, PipeConnection* conn){
		printf("Recieved name %s. Haider!\n",pak->name);
	});

	return pipe.MainRoutine();
}