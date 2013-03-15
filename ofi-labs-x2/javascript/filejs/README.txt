File processing using V8.

This tool requires a working V8 library. V8 is the JavaScript engine for
Google Chrome. It is open-source and available from the official site:
http://code.google.com/p/v8/.

Please follow the instructions on Google V8 wiki page in order to build it.
It is expected that you have it under a sub-directory called lib. Some
important wiki pages are:
http://code.google.com/p/v8/wiki/Source
http://code.google.com/p/v8/wiki/BuildingOnWindows
http://code.google.com/p/v8/wiki/Contributing

Please pay attention to the required tools necessary to build V8 from source,
i.e. Python and scons.

Build steps:

  svn checkout http://v8.googlecode.com/svn/trunk lib
  cd lib && scons mode=release
  cd ..
  g++ -o filejs filejs.cpp -Ilib/include/ -lv8 -Llib -lpthread

Note 1: On 64-bit system, you may need to add 'arch=x64' when running scons.

Note 2: I tested with V8 version 2.4.9 (revision 5610). If it does not work with
later revision, try to check out exactly this revision (e.g. pass "-r 5610" as
the option when using svn).

Note 3: On some system, you need to add '-lpthread' when compiling filejs.

How to use:

  ./filejs.cpp yourscript.js [arguments]

Some included examples:

  rot13.js       Encyrpt a text file using ROT-13 substitution cipher
  countlines.js  Count non-empty lines in a file

