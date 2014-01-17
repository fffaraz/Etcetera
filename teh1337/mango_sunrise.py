#!/usr/bin/env python
import random, time, threading, sys

thread_count = 50
counts = map(lambda x: 0, range(thread_count))
answerdb = {}

class save_africa(threading.Thread):
    def __init__(self, i):
        threading.Thread.__init__(self)
        self.number = i

    def run(self):
        import urllib, urllib2
        # config
        url = "http://www.freerice.com/index.php"
        opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
        opener.addheaders = [('User-agent', 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)')]
        urllib2.install_opener(opener)

        grains_earned = "0"

        # get the main page
        page = urllib2.urlopen(url).read().split("\n")

        while True:
            try:
                if "CORRECT" in "\n".join(page) and answers[0] not in answerdb:
                    answerdb[answers[0]] = answers[choice]

                # parse question word
                for line in page:
                    if 'name="PAST" value="' in line:
                        PAST = line.split("value=")[1].split('"')[1]
                    elif 'name="INFO" value=' in line:
                        INFO  = line.split("value=")[1].split('"')[0].split()[0]
                    elif 'name="INFO2" value=' in line:
                        INFO2 = line.split("value=")[1].split('"')[0].split()[0]
                    elif 'name="INFO3" value="' in line:
                        INFO3 = line.split("value=")[1].split('"')[1].replace(" ", "+")
                    elif 'grains of rice.' in line:
                        grains_earned = line.split("grains of rice.")[0].split()
                        grains_earned = grains_earned[len(grains_earned) - 1]
                        if grains_earned[0] == "/":
                            grains_earned = grains_earned[2:]
                        break

                # parse choices
                answers = filter(lambda x: len(x), INFO3.split("+"))

                if answers[0] in answerdb:
                    choice = 0
                    for word in answers[1:]:
                        if word == answerdb[answers[0]]:
                            choice = answers.index(word)
                            break
                    if choice == 0:
                        choice = random.randint(1,4)                

                else:       
                    # search google
                    google_url = "http://www.google.com/search?hl=en&q=define\x3A%s&btnG=Search" % answers[0]
                    google_search = urllib2.urlopen(google_url).read()

                    choice = 0
                    for word in answers[1:]:
                        if word in google_search:
                            choice = answers.index(word)
                            break
                    if choice == 0:
                        choice = random.randint(1,4)

                # build answer
                data = {"SELECTED":'',
                        "PAST":PAST,
                        "INFO":INFO,
                        "INFO2":INFO2,
                        "INFO3":INFO3,
                        "SELECTED":"+%d+" % choice}
                data = urllib.urlencode(data)

                # send answer
                req = urllib2.Request(url, data)
                page = urllib2.urlopen(req).read().split("\n")

                counts[self.number] = int(grains_earned)
                time.sleep(1)
            except: pass

if __name__ == "__main__":
    print "[+] Earth!"
    print "[+] Fire!"    
    print "[+] Wind!"
    print "[+] Water!"
    print "[+] Heart!"
    print "[+] With your powers combined, I am Captain Planet!"
    print "[+] GOOOOOOOOOO PYTHON!"

    for i in range(thread_count):
        save_africa(i).start()

    old = 0
    while True:
        new = 0
        for slot in counts:
            new += slot
        if new != old:
            sys.stdout.write("\r[+] Total Rice Earned: %d            " % new)
            sys.stdout.flush()
        if not new < old:
            old = new
        else:
            new += old
