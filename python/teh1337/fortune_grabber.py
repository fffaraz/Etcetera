#!/usr/bin/env python
#
# Fortune Cookie Saying Grabber
# By James Penguin (brandon.smith@studiobebop.net)
#
import BeautifulSoup
import urllib2

def get_fortune():
    try:
        page = urllib2.urlopen("http://www.fortunecookiemessage.com/")
    except:
        return "No fortune for you."
    soup = BeautifulSoup.BeautifulSoup(page)
    for tag in soup.findAll("a"):
        if not tag.has_key("href") or not tag["href"].startswith("cookie"):
            continue
        if not len(tag.contents):
            return "No fortune for you."
        return tag.contents[0]
    return "No fortune for you."

if __name__ == '__main__':
    print get_fortune()
