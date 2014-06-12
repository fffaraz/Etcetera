#!/usr/bin/env python
#
# Simon Says
# By James Penguin (jamespenuin@gmail.com)
#
import sys, time, os, random

###
# Config
###
score  = 0
fail   = 0
level  = 1
hiscore = 0
sequence = ""
chars  = list("asdfjkl;")

### End Config ###

def clear_screen():
    if os.name == "posix":
        os.system("clear")
    else:
        os.system("cls")

def print_score():
    global score, level, hiscore
    print "-" * 50
    print "|" + " " * 48 + "|"
    temp = "| Level: %d" % level
    print temp + " " * (49 - len(temp)) + "|"
    temp = "| Current Score: %d" % score
    print temp + " " * (49 - len(temp)) + "|"
    temp = "| High Score: %d" % hiscore
    print temp + " " * (49 - len(temp)) + "|"
    print "|" + " " * 48 + "|"
    print "-" * 50
    print "\n" * 2

def update_hiscore():
    global hiscore
    f = open('hiscore', 'w')
    f.write(str(hiscore))
    f.close()

def display_colors():
    global level, chars, fail, score, sequence, hiscore

    for i in range(3, 0, -1):
        sys.stdout.write("\r\tNext round begins in %d   " % i)
        sys.stdout.flush()
        time.sleep(1)

    print "\r\tRound begin!            "
    print "\tSee if you can remember the sequence I'm about show you."
    time.sleep(1)

    if not sequence:
        for i in range(2):
            random.shuffle(chars)
            try:
                while chars[0] == sequence[len(sequence) - 1]:
                    random.shuffle(chars)
            except:
                pass
            sys.stdout.write("\r\t%s  " % chars[0])
            sys.stdout.flush()
            sequence += chars[0]
            time.sleep(1)
    else:
        while chars[0] == sequence[len(sequence) - 1]:
            random.shuffle(chars)
        sequence += chars[0]
        for char in sequence:
            sys.stdout.write("\r\t%s  " % char)
            sys.stdout.flush()
            time.sleep(1)

    print "\r\tDo you remember the sequence you just saw?"
    guess = raw_input("\tEnter it: ").lower()
    sys.stdout.write("\tHmm, let's see...")
    sys.stdout.flush()
    time.sleep(2)

    if guess == sequence:
        score += 10
        level += 1
        if score > hiscore:
            hiscore = score
            update_hiscore()
        print "\r\tWell done, you were correct!"
        time.sleep(1)
    else:
        fail = 1
        print "\r\tToo bad, it seems luck was not on your side this time."
        print "\n" * 2


if __name__ == "__main__":
    while not fail:
        if not os.path.isfile('hiscore'):
            a = open('hiscore', 'w')
            a.write('0')
            a.close()
        hiscore = int(open('hiscore', 'r').read())
        clear_screen()
        print_score()
        display_colors()
