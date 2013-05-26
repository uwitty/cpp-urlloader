#ifndef __LOADER_H__
#define __LOADER_H__

#include <future>
#include <memory>
#include <string>
#include <functional>

class Loader
{
public:
	static std::shared_ptr<Loader> load(const std::string& url, std::function<void(int, const void*, unsigned)> callback);
	void wait() const;
	void cancel();

	~Loader();

private:
	Loader();
	static void run(std::shared_ptr<Loader> loader, std::function<void(int, const void*, unsigned)> callback, const std::string& url);

	std::shared_future<void> finished_;
	std::promise<bool> canceled_;
};

#endif

