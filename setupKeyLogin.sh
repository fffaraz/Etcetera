#!/bin/bash
# Written by: Hans-Filip Elo
# Usage: 
#	setupKeyLogin.sh USERNAME SERVER


# --------------------------------
# Enters username

if [ -z $1 ]
then
    echo " "
    echo "Please enter your username on remote server:"
    read USERNAME
else
    USERNAME=$1
fi

# --------------------------------
# Enters server adress

if [ -z $2 ]
then
    echo " "
    echo "Please enter server adress:"
    read SERVER
else
    SERVER=$2
fi

# --------------------------------

echo "------------------------------"

# Creates pair of keys

if [ -e ~/.ssh/id_rsa.pub ]
then
	echo " "
	echo "Found RSA-key in homefolder."
else
	echo "No key RSA-key found. Creating one."
	echo " "
	echo "LEAVE THE FOLLOWING THREE FIELDS EMPTY"
	echo "CONFIRM WITH ENTER"
	echo " "
	# Creates keypair
	ssh-keygen -t rsa
	# Secures file
	chmod 600 ~/.ssh/id_rsa
	chmod 600 ~/.ssh/id_rsa.pub
fi

# Cats key to a temporary file
cat ~/.ssh/id_rsa.pub >> ~/authorized_keys

echo " "
echo "TYPE PASSWORD FOR YOUR REMOTE ACCOUNT WHEN ASKED"
echo " "

echo "------------------------------"

# Uploads keys to ixtab and sets safe permissions
scp ~/authorized_keys $USERNAME@$SERVER:
ssh $USERNAME@$SERVER << ENDSSH
mkdir -p ~/.ssh
touch ~/authorized_keys
cat ~/authorized_keys >> ~/.ssh/authorized_keys
rm ~/authorized_keys
chmod 700 ./.ssh/authorized_keys
ENDSSH

echo "------------------------------"

if [ $? == 0 ]
then
    echo " "
    echo "Key-login configured!"
    echo " "
else
    echo " "
    echo "Key-login configuration FAILED!"
    echo " "
fi

# Removes temporary file
rm ~/authorized_keys

exit
