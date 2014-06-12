#!/bin/bash

# Lists installed packages and outputs to file. 
dpkg --get-selections > /home/fille/scripts/installed_packages.txt
# In order to install packages in case of system failiure - run the following:
# dpkg --set-selections < installed_packages.txt
# apt-get autoremove
# apt-get dselect-upgrade

