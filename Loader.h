#ifndef __LOADER_H__
#define __LOADER_H__

#include <future>
#include <memory>
#include <string>
#include <functional>

class Loader
{
public:
	enum class loader_result {
		succeeded,
		failed,
	};
	typedef std::function<void(loader_result, const void*, unsigned)> loader_callback;

	static std::shared_ptr<Loader> load(const std::string& url, const loader_callback& callback);
	void wait() const;
	void cancel();

	~Loader();

private:
	explicit Loader(const loader_callback& callback);
	Loader& operator=(const Loader&) = delete;
	Loader(const Loader&) = delete;

	class Impl;
	std::unique_ptr<Impl> impl_;
};

#endif

