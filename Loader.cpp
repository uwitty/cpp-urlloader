#include "Loader.h"

#include <tuple>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <cstdio>

#include <curl/curl.h>

using namespace std;

class Loader::Impl
{
public:
	explicit Impl(const loader_callback& callback) : callback_(callback)
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

				int retcode = curl_easy_perform(handle);
				curl_easy_cleanup(handle);

				if (get<2>(t)) {
					printf("%s() stopped. \n", __PRETTY_FUNCTION__);
					return ;
				}

				auto p = wptr.lock();
				if (p) {
					if (retcode == 0) {
						p->impl_->callback_(loader_result::succeeded, &data[0], data.size());
					} else {
						p->impl_->callback_(loader_result::failed, NULL, 0);
					}
				} else {
					printf("listener object is null. \n");
				}
			}
			, loader->impl_->canceled_.get_future());
	}

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
		data.insert(data.end(), static_cast<uint8_t*>(buffer), static_cast<uint8_t*>(buffer) + size*nmemb);

		return size*nmemb;
	}

	std::shared_future<void> finished_;
	std::promise<bool> canceled_;
	Loader::loader_callback callback_;
};

std::shared_ptr<Loader> Loader::load(const std::string& url, const loader_callback& callback)
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

Loader::Loader(const loader_callback& callback) : impl_(new Impl(callback))
{
}

Loader::~Loader()
{
	cancel();
}

