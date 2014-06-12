#!/usr/bin/env python
#
# Internal Speaker Beeping Module for Windows
#
import time
import winsound

###
# Notes Config
###

# Set delay tempo
tempo = 0.15

# Setup Notes
notes = {}
notes["pause"] = 0
notes["c"] = 1
notes["c#"] = 2
notes["d"] = 3
notes["d#"] = 4
notes["e"] = 5
notes["f"] = 6
notes["f#"] = 7
notes["g"] = 8
notes["g#"] = 9
notes["a"] = 10
notes["a#"] = 11
notes["b"] = 12

# Note Types
note_types = {}
note_types["sixteenth"] = 50
note_types["eigth"] = 100
note_types["dotted_eigth"] = 150
note_types["quarter"] = 200
note_types["half"] = 400
note_types["whole"] = 800
note_types["triplet"] = 60

#!# End Notes Config #!#

def play_note(octave, note, note_type):
    """Play a note at a certain octave by calculating the frequency of 
the sound it would represent on the motherboard's speaker."""

    # Match the note and note type to the dictionaries
    note = notes[note]
    note_type = note_types[note_type]

    # Chill for a bit if it's a pause
    if not note:
        time.sleep(note_type/1000)
        return

    # Calculate C for the provided octave
    frequency = 32.7032 * (2**octave)

    # Calculate the frequency of the given note
    frequency *= 1.059463094**note

    # Beep it up
    winsound.Beep(int(frequency), note_type)

    # Delay after the beep so it doesn't all run together
    time.sleep(tempo)

def mission_impossible():
    oct = 3
    play_note(oct, "g", "half")
    play_note(oct, "g", "half")
    play_note(oct+1, "a#", "quarter")
    play_note(oct+1, "c", "quarter")
    play_note(oct, "g", "half")
    play_note(oct, "g", "half")
    play_note(oct, "f", "quarter")
    play_note(oct, "f#", "quarter")
    play_note(oct, "g", "half")
    play_note(oct, "g", "half")
    play_note(oct+1, "a#", "quarter")
    play_note(oct+1, "c", "quarter")
    play_note(oct, "g", "half")
    play_note(oct, "g", "half")
    play_note(oct, "f", "quarter")
    play_note(oct, "f#", "quarter")

def final_fantasy_victory():
    oct = 4
    play_note(oct+1, "c", "triplet")
    play_note(oct+1, "c", "triplet")
    play_note(oct+1, "c", "triplet")
    play_note(oct+1, "c", "quarter")
    play_note(oct, "g#", "quarter")
    play_note(oct, "a#", "quarter")
    play_note(oct+1, "c", "dotted_eigth")
    play_note(oct, "a#", "sixteenth")
    play_note(oct+1, "c", "whole")

def praise_to_the_man():
    oct = 4
    play_note(oct, "c", "quarter")
    play_note(oct, "c", "dotted_eigth")
    play_note(oct, "c", "sixteenth")
    play_note(oct, "e", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "c", "quarter")
    play_note(oct+1, "e", "dotted_eigth")
    play_note(oct+1, "d", "sixteenth")
    play_note(oct+1, "c", "quarter")
    play_note(oct, "g", "quarter")
    play_note(oct, "f", "quarter")
    play_note(oct, "a", "dotted_eigth")
    play_note(oct, "f", "sixteenth")
    play_note(oct, "e", "quarter")
    play_note(oct, "g", "dotted_eigth")
    play_note(oct, "e", "sixteenth")
    play_note(oct, "d", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "d", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "d", "half")
    play_note(oct, "c", "quarter")
    play_note(oct, "c", "dotted_eigth")
    play_note(oct, "c", "sixteenth")
    play_note(oct, "e", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "c", "eigth")
    play_note(oct+1, "e", "dotted_eigth")
    play_note(oct+1, "d", "sixteenth")
    play_note(oct+1, "c", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct, "f", "eigth")
    play_note(oct, "a", "dotted_eigth")
    play_note(oct, "f", "sixteenth")
    play_note(oct, "e", "eigth")
    play_note(oct, "g", "dotted_eigth")
    play_note(oct, "e", "sixteenth")
    play_note(oct, "d", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "d", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "c", "half")
    play_note(oct+1, "c", "eigth")
    play_note(oct+1, "c", "dotted_eigth")
    play_note(oct+1, "c", "sixteenth")
    play_note(oct, "b", "eigth")
    play_note(oct, "a", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "c", "quarter")
    play_note(oct+1, "c", "dotted_eigth")
    play_note(oct+1, "c", "sixteenth")
    play_note(oct, "b", "eigth")
    play_note(oct, "a", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct, "f", "eigth")
    play_note(oct+1, "c", "quarter")
    play_note(oct+1, "c", "dotted_eigth")
    play_note(oct+1, "c", "sixteenth")
    play_note(oct, "b", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "d", "whole")
    play_note(oct+1, "c", "eigth")
    play_note(oct, "b", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "c", "eigth")
    play_note(oct, "a", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct, "f", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "d", "eigth")
    play_note(oct, "c", "quarter")
    play_note(oct, "c", "dotted_eigth")
    play_note(oct, "c", "sixteenth")
    play_note(oct, "e", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "g", "eigth")
    play_note(oct+1, "c", "quarter")
    play_note(oct+1, "e", "dotted_eigth")
    play_note(oct+1, "d", "sixteenth")
    play_note(oct+1, "c", "quarter")
    play_note(oct, "g", "quarter")
    play_note(oct, "f", "quarter")
    play_note(oct, "a", "dotted_eigth")
    play_note(oct, "f", "sixteenth")
    play_note(oct, "e", "quarter")
    play_note(oct, "g", "dotted_eigth")
    play_note(oct, "e", "sixteenth")
    play_note(oct, "d", "eigth")
    play_note(oct, "c", "eigth")
    play_note(oct, "d", "eigth")
    play_note(oct, "e", "eigth")
    play_note(oct, "c", "whole")

def party_time():
    mission_impossible()
    time.sleep(.4)
    final_fantasy_victory()
    time.sleep(.4)
    praise_to_the_man()

if __name__ == '__main__':
    party_time()
