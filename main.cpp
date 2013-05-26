#include "Loader.h"
#include <cstdio>
#include <unistd.h>

using namespace std;

shared_ptr<Loader> load_and_print(const string& url)
{
	return Loader::load(url, [](int result, const void* buf, unsigned size) {
		printf("callback -> {result:%d, buf:%p, size:%d}\n", result, buf, (int)size);

		string s = string((const char*)buf, size);
		printf("--------------------------------------\n");
		printf("%s\n", s.c_str());
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

