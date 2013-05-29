#include "Loader.h"

#include <tuple>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <cstdio>

#include <curl/curl.h>

using namespace std;

static size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp)
{
	auto& t = *((tuple<future<bool>&, vector<uint8_t>&, bool&>*)userp);
	future<bool>& canceled = get<0>(t);
	if (canceled.wait_for(chrono::seconds(0)) == future_status::ready) {
		printf("%s() canceled. \n", __func__);
		get<2>(t) = true;
		return 0;
	}

	vector<uint8_t>& data = get<1>(t);
	data.insert(data.end(), (uint8_t*)buffer, (uint8_t*)buffer + size*nmemb);

	return size*nmemb;
}

class Loader::Impl
{
public:
	explicit Impl(const std::function<void(int, const void* buf, unsigned size)>& callback) : callback_(callback)
	{
	}

	static void run(std::shared_ptr<Loader> loader, const std::string& url)
	{
		weak_ptr<Loader> wptr(loader);

		loader->impl_->finished_ = async(launch::async
			, [wptr, url](std::future<bool> canceled) {
				vector<uint8_t> data;
				bool stopped = false;
				auto t = make_tuple(ref(canceled), ref(data), ref(stopped));

				CURL* handle = curl_easy_init();

				curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
				curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
				curl_easy_setopt(handle, CURLOPT_WRITEDATA, &t);

				int result = curl_easy_perform(handle);
				curl_easy_cleanup(handle);

				if (get<2>(t)) {
					printf("%s() stopped. \n", __PRETTY_FUNCTION__);
					return ;
				}

				auto p = wptr.lock();
				if (p) {
					p->impl_->callback_(result, &data[0], data.size());
				} else {
					printf("listener object is null. \n");
				}
			}
			, loader->impl_->canceled_.get_future());
	}

	std::shared_future<void> finished_;
	std::promise<bool> canceled_;
	std::function<void(int, const void* buf, unsigned size)> callback_;
};

std::shared_ptr<Loader> Loader::load(const std::string& url, std::function<void(int, const void* buf, unsigned size)> callback)
{
	shared_ptr<Loader> p(new Loader(callback));
	Loader::Impl::run(p, url);
	return p;
}

void Loader::wait() const
{
	impl_->finished_.wait();
}

void Loader::cancel()
{
	try {
		impl_->canceled_.set_value(true);
	} catch (future_error& e) {
	}
}

Loader::Loader(const std::function<void(int, const void* buf, unsigned size)>& callback) : impl_(new Impl(callback))
{
}

Loader::~Loader()
{
	cancel();
	delete impl_;
}

