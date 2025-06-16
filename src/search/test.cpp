#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

	auto handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, "https://stallman.org/");

	auto* outfile = fdopen(1, "w");
	if (!outfile) {
		std::cerr << "Could not open stdout.\n";
		return -1;
	}

	// curl writes output data to a file, by default.
	curl_easy_setopt(handle, CURLOPT_FILE, outfile);

	auto success = curl_easy_perform(handle);
	if (CURLE_OK != success)
	{
		std::cerr << "Error transferring url.\n";
		return -1;
	}

	return 0;
}
