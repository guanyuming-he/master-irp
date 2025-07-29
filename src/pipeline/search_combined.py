"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The files defines the function used to invoke both my custom search engine and
commercial third-party search engine, and combine and filter their results.
"""

import datetime
from googleapiclient.discovery import build
import subprocess

from config import Config, SearchConf

def custom_search(
	search_prompt: str,
	start_date : datetime.date, end_date : datetime.date
) -> str:
	"""
	Search search_prompt using my custom search engine.

	@returns the search results, trimmed out of the first few info lines.
	@raises RuntimeError if an error occurred.
	"""
	# My custom search engine is Xapian, which supports
	# start_date..end_date queries
	search_prompt += \
		f" {start_date.isoformat()}..{end_date.isoformat()}";

	try:
		cmd = ["./bin/searcher", "./db", search_prompt]
		result = subprocess.run(
			cmd,
			capture_output=True,
			text=True,
			timeout=3  # 3 seconds
		)
		
		if result.returncode != 0:
			warning_msg = (f"Search command failed for '{search_prompt}': "
						f"{result.stderr}")
			raise RuntimeError(warning_msg)
		
		# trim the first two lines.
		# which are:
		# query_str = ...
		# Found ... results
		lines = result.stdout.splitlines()
		return '\n'.join(lines[2:])
		
	except subprocess.TimeoutExpired:
		raise RuntimeError(f"Search timeout for prompt: {search_prompt}")
	except Exception as e:
		raise RuntimeError(f"Search error for '{search_prompt}': {e}")


def google_search(
	conf: SearchConf, 
	search_prompt : str,
	start_date : datetime.date, end_date : datetime.date
):
	# The first string must be customsearch, representing Google's 
	# custom search service.
	service = build("customsearch", "v1", developerKey=conf.google_api_key)
	res = service.cse().list(
		q=search_prompt, cx=conf.google_engine_id, 
		# let's not search too much at once so that
		# my quota isn't reached.
		num=10,
		# usage: https://developers.google.com/custom-search/docs/
		# structured_search#restrict_to_range
		sort= (
			f"date:r:{start_date.strftime('%Y%m%d')}:" +
			f"{end_date.strftime('%Y%m%d')}"
		)
	).execute()
	# Return the results in my custom search format:
	# url \t title \n
	# snippet
	res = res["items"]

	ret : str = ""
	for item in res:
		ret += (
			f"{item['link']}\t{item['title']}\n" +
			f"{item['snippet']}\n\n"
		)
	return ret


def search_filter_combine(
	conf : SearchConf,
	search_prompt : str,
	start_date : datetime.date, end_date : datetime.date
):
	"""
	Search with both my custom search engine and Google search engine,
	filter by date,
	and finally combine the results.
	"""
	ret1 = custom_search(
		search_prompt, start_date, end_date
	)
	ret2 = google_search(
		conf, search_prompt,
		start_date, end_date
	)
	return ret1 + ret2

# Main is for tests only. The file is used by directly calling the above
# functions.
if __name__ == "__main__":
	config = Config.load_default()
	print(google_search(
		config.search_conf,
		"Trump tariff",
		datetime.date(2025,1,1),
		datetime.date(2025,7,10)
	))
