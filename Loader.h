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
	explicit Loader(const std::function<void(int, const void*, unsigned)>& callback);
	Loader& operator=(const Loader&) = delete;
	Loader(const Loader&) = delete;

	class Impl;
	Impl* impl_;
};

#endif

