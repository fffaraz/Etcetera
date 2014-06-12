#!/usr/bin/env python
#
# Operation Sharada Sharada
# By James Penguin (jamespenguin@gmail.com)
#
import urllib2, os, BeautifulSoup

###
# Global Config
###
base_url = "http://www.samair.ru/proxy/type-%s.htm"

### End Global Config ###

def determine_total_pages():
    global base_url
    url = base_url % "01"
    page = urllib2.urlopen(url).read()
    return int(page.split("total pages:")[1].split(",")[0].strip())

def retrieve_proxies(page_number):
    global base_url
    page_number = "%02d" % page_number
    url = base_url % page_number
    page = urllib2.urlopen(url).read()
    soup = BeautifulSoup.BeautifulSoup(page)
    proxies = []
    for table in soup.findAll("table"):
        for row in table.findAll("td"):
            row = str(row.string)
            if not "." in row or ":" not in row:
                continue
            proxies.append(row)
    return proxies

def export(proxies):
    proxy_file = open("proxies.txt", "a")
    for proxy in proxies:
        proxy_file.write("%s\n" % proxy)
    proxy_file.close()

if __name__ == "__main__":
    try:
        opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
        opener.addheaders = [('User-agent', 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)')]
        urllib2.install_opener(opener)
        open("proxies.txt", "w").close() # prep a file for writing proxies
        total = determine_total_pages()
        for page_number in range(total):
            page_number += 1
            print "[+] Retrieving proxies from page %d of %d" % (page_number, total)
            export(retrieve_proxies(page_number))
    except KeyboardInterrupt:
        print
