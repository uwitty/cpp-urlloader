#include "Loader.h"
#include <cstdio>
#include <unistd.h>

using namespace std;

shared_ptr<Loader> load_and_print(const string& url)
{
	return Loader::load(url, [](Loader::loader_result res, const void* buf, unsigned size) {
		const char* s = (res == Loader::loader_result::succeeded)? "succeeded" : "ng";
		printf("callback -> {result:%s, buf:%p, size:%d}\n", s, buf, (int)size);

		string response = string((const char*)buf, size);
		printf("--------------------------------------\n");
		printf("%s\n", response.c_str());
		printf("--------------------------------------\n");
	});
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("%s: invalid arguments. \n", argv[0]);
		return 0;
	}

	auto handle = load_and_print(argv[1]);

	handle->wait();

	return 0;
}

