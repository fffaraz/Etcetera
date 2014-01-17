#!/usr/bin/env python
#
# Studio Bebop Presnets
# Progress Bar Control Class
# By James Penguin (jamespenguin@gmail.com)
#

###
# Config
###
bar_width = 40

#!# end config #!#

class progressBar:
    def __init__(self, total):
        self.total = total
        self.pieces = ["|", "/", "-", "\\", "|", "/", "-", "\\"]
        self.index = 0

    def get_bar(self, current):
        try:
            piece = self.pieces[self.index]
            self.index += 1
        except:
            self.index = 0
            piece = self.pieces[self.index]
            self.index += 1

        if current == 0:
            percent = 0
            bars = ""
            bars += " " * (bar_width - len(bars))
        elif current < 0:
            current *= -1
            if current == 1337:
                current = 0
            percent = (current * 100) / self.total
            bars = "=" * ((current * bar_width) / self.total)
            bars += " " * (bar_width - len(bars))
            piece = "X"
        else:
            percent = (current * 100) / self.total
            if percent == 100:
                piece = "O"
            bars = "=" * ((current * bar_width) / self.total)
            if percent != 100:
                bars += ">"
            bars += " " * (bar_width - len(bars))
        line = "[%s] %s (%d%%)" % (bars, piece, percent)
        return line

if __name__ == "__main__":
    import sys, time
    # generate a bar with a maximum count of 500
    p = progressBar(500)
    for i in range(500):
        bar = p.get_bar(i + 1) # update the progress bar with the current count
        sys.stdout.write("\r%s" % bar)
        sys.stdout.flush()
        time.sleep(.1)
    print
