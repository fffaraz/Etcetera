#!/usr/bin/env python
import urllib2, ClientForm, threading, sys

email = ""        # Email address of account to post comments
password = ""     # Password of account to post comments
friendID = ""     # Friend ID of recipient of comments
message =  ""     # Message to leave in comments
thread_limit = 40 # Number of bots to run in parallel

class postComment(threading.Thread):
	def __init__(self): threading.Thread.__init__(self)
	def run(self):
		print "Posting comment %d!" % counter
		req = urllib2.Request("http://comment.myspace.com/index.cfm?fuseaction=user.viewProfile_commentForm&friendID=%s" % friendID)
		res = opener.open(req)
		forms = ClientForm.ParseResponse(res)
		form = forms[1]
		form["ctl00$cpMain$postComment$commentTextBox"] = message
		res = opener.open(form.click())
		forms = ClientForm.ParseResponse(res)
		form = forms[1]
		opener.open(form.click())

# login
opener = urllib2.build_opener(urllib2.HTTPCookieProcessor())
opener.addheaders = [('User-agent', 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)')]
urllib2.install_opener(opener)
req = urllib2.Request("http://login.myspace.com/index.cfm?fuseaction=login.process")
res = opener.open(req)
forms = ClientForm.ParseResponse(res)
form = forms[1]
form["email"] = email
form["password"] = password
opener.open(form.click())

# post comment
counter = 0
while 1:
	try:
		if threading.activeCount() < thread_limit: 
			print counter
			postComment().start()
			counter+=1
			sys.stdout.write("\r%d Comments Posted." % counter)
			sys.stdout.flush()
	except KeyboardInterrupt: break
