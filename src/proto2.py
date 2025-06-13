import requests
from bs4 import BeautifulSoup

def crawl(url):
    """
    Returns
    -------
    The full text of the url will be returned.
    """
    try:
        r = requests.get(url, timeout=5)
        r.raise_for_status()
        soup = BeautifulSoup(r.text, 'html.parser')
        text = soup.get_text(separator=' ', strip=True)
        return text
    except Exception as e:
        print(f"Failed to crawl {url}: {e}")
        return ""

def load_urls():
    url_file = "./urls"
    with open(url_file, 'r', encoding='utf-8') as f:
        return [line.strip() for line in f if line.strip()]

