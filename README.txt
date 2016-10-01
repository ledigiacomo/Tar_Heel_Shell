Language: C

Authors: Luke DiGiaocmo, Marcus Wallace

Written for: COMP 530 - Operating Systems course at the University of North 
Carolina at Chapel Hill Fall 2016

Tar Heel Shell
==============

Basic Operation
===============
	This program is intended to serve as a basic shell that works on a small 
	subset of Bash's functionality

	The basic control flow of the program is divided up into three main 
	functions: read input, checkCmd, and execute. The read input 
	functionality is handled in main and is not is own method. In main, input
	is read from the user until '\n' is reached. At this point the shell will 
	attempt to execute the user input. The shell begins by "cleaning up" the 
	user input by putting whitespace around any special characters. '<', '>', 
	'|', '&' are considered special characters. Once the input is "pretty", 
	it is passed into checkCmd which determines if the command is a built in 
	command or if the shell needs to search through $PATH. If the command is 
	found at the end of $PATH, checkCmd will return the return value of execv.
	Otherwise it will return a similar value for built in or not found commands. 

	checkCmd(char* cmd, char** params)
		cmd will be passed into execv as the command to be executed

		params is a list of the parameters to be passed into execv. the last 
		value of this array must be NULL.

	execute(char* cmd, char** params)
		cmd will be passed into execv as the command to be executed

		params is a list of the parameters to be passed into execv. the last 
		value of this array must be NULL.

	printHeel()
		this commaand takes no parameters and prints an ascii art picture 
		that is found at $HOME/heel.txt

		it goes through the file character by character and prints wht is read 
		stdout.

Built-in Commands
=================
	cd [path]
		changes the current directory to path
			path = '' will change to $HOME
			path = '-' will change to last directory

	set Variable=Value
		will create a new environment variable called the contents of Variable
		with the value of the contents of Value

	exit
		terminates the shell

	goheels
		prints an ascii-art picture of a Tar Heel

Known Bugs
==========
	1. this shell does not currently support job control.  
	2. Pipes are also not fully supported. Use at your own risk
	3. This shell does not support "" to distinguish filenames with whitespace