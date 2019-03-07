// CS270 Project 4
// Authors: Josh Luttrell, Devon Schleyer
// 4/1/2018
// Purpose: The program creates a simpler version of the SSH shell which consists
//	    of Built-in commands that include setvar, setprompt, setdir, showprocs,
//	    and done. The program also accepts the following Program-control commands
//	    that include run, fly, and tovar. If the user inputs an invalid command
//	    or uses the incorrect amount of parameters, an error message will 
//	    print to standard error.

using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iterator>
#include <stdio.h>
#include <fstream>

#define sizeof2 2
#define sizeof3 3

vector<string> tokenize(string command, vector<string> variables, vector<string> showTok, int &varIndex)
{
	vector<string> tokens;
	istringstream iss(command);
	string token;


	// Tokenizes the users input
	// by spaces
	while(getline(iss, token, ' ') )
	{	
		tokens.push_back(token);
	}


	// Scans token vector for comment and 
	// empty elements
	int commentIndex = -1;	
	int numE = 0;
	for (int i = 0; i< tokens.size(); i++)
	{
		if (tokens[i] == "")
			numE++;
		if (tokens[i][0] == '#')
			commentIndex = i;
	}


	// Removes comments ie. '#'
	if (commentIndex != -1)
	{
		for (int i = tokens.size()-1; i >= commentIndex; i--)
			tokens.pop_back();
	}

	// Runs loop until all extra spaces are removed
	while(numE != 0)
	{
		for (int i = 0; i < tokens.size(); i++)
		{

			// Finds empty elements and shifts all elements
			// left and pops back to remove empty elements
			if(tokens[i] == "")
			{	// MAY NOT NEED THESE

				for (int j = i; j < tokens.size() - 1; j++)
				{
					tokens[j] = tokens[j + 1];
				}
			}	//MAY NOT NEED THESE

			tokens.pop_back();
			numE--;
		}
	}



	// Index of vector that has beginning quote "
	int startQ = -1;

	// Index of element that has end quote "
	int endQ = -2;


	// Finds the above respective indices
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i][0] == '\"')
		{
			startQ = i; 

		}

		if (tokens[i][tokens[i].size()-1] == '\"')
			endQ = i;
	}        


	// This if statement is for the case that
	// the users input that contained quotes 
	// with space(s) within the quotes
	// Tokenization above broke the quote element 
	// over spaces
	if (startQ != endQ && startQ != -1)
	{

		// Creates as string that will hold everthing within the qotes
		string inQ = tokens[startQ].substr(1, tokens[startQ].size()-1) + " ";

		// Appends to the above string the contents of the elements that 
		// are within the quotes
		for (int i = startQ + 1; i < endQ + 1; i++)
		{

			// Does concatenation for word containing end quote
			if (tokens[i][tokens[i].size()-1] == '\"'){
				inQ += tokens[i].substr(0, tokens[i].size()-1);

			}

			// Does concatentation for words within the quotes that were 
			// separated by spaces
			else {
				inQ += tokens[i];
				inQ += " ";
			}
		}

		// Sets the element that contained the 
		// beginning quote to be the full 
		// contents within the quotes
		tokens[startQ] = inQ;

		// Removes the remaining elements that were
		// contained within the quotes that got
		// separated over spaces
		int toRemove = endQ - startQ;
		int count = 0;
		while (count != toRemove){
			for (int i = startQ + 1; i < tokens.size() - 1; i++)
				tokens[i] = tokens[i+1];
			tokens.pop_back();
			count++;
		}

	}

	// If the user entered no spaces within the quotes 
	// it removes the quotes from the beginning and end
	else if(startQ == endQ)
	{
		tokens[startQ] = tokens[startQ].substr(1, tokens[startQ].size()-2);
	}







	// Prints tokens of the next command when ShowTokens has a value of 1
	if(showTok[1] == "1")
	{
		for(int i = 0; i < tokens.size(); i++)
		{
			cout << "Token = " << tokens[i] << endl;
		}
	}

	// gets the number of carrots in 
	// the users input
	int numCarrs = 0; 
	for (int n = 0; n < command.size(); n++)
	{
		if(command[n] == '^')
			numCarrs++;	

	}

	int carrot = -1;
	int expansionPres = 0;
	if (numCarrs != 0)
	{
		while(numCarrs != 0)
		{
			// index of token that has '^'
			int tokenCarrIndex = -1;
			// Changes the carrot value when an expansion happens
			for(int m = 0; m < tokens.size(); m++)
			{
				carrot = tokens[m].find('^');	
				if (carrot != -1)
				{
					tokenCarrIndex = m;
					expansionPres = 1;
				}
			}


			// Stores token[i] into a character array arg
			// (token[i] has multiple words within it)
			char arg[tokens[tokenCarrIndex].size()+1];
			for(int k = 0; k < tokens[tokenCarrIndex].size(); k++)
			{
				arg[k] = tokens[tokenCarrIndex][k];
			}	
			arg[tokens[tokenCarrIndex].size()] = '\0';


			// Creates a vector of tokens from each word in
			// the token with double quotes
			vector<string> carrToks;
			char *subtoken = strtok(arg, " \r\t");
			while(subtoken != NULL)
			{	
				carrToks.push_back(subtoken);
				subtoken = strtok(NULL, " \r\t");
			}

			// carrToks index that contains '^'
			int subCarrIndex = -1;

			// Finds the index of the token with a '^'
			// in the vector of strings
			int carrTest = 0;
			for (int k = 0; k < carrToks.size(); k++)
			{	
				carrTest = carrToks[k].find('^');
				if (carrTest != -1)
					subCarrIndex = k;
			}

			// Stores the name of a variable with a ^
			// into a string
			string varName = "";
			for (int k = 1; k < carrToks[subCarrIndex].size(); k++)
				varName += carrToks[subCarrIndex][k];

			// Searches to see if the variable name has already
			// been defined in the vector variables. If it has, then
			// it replaces the token with its value in the carrTokens vector
			//int varIndex = -1;
			for (int y = 0; y < variables.size(); y += 2)

			{	
				if ( variables[y] == varName)
				{
					carrToks[subCarrIndex] = variables[y + 1];
					varIndex = 1;
				}
			}
			if(varIndex != 1)
			{
				cerr << "No such shell variable: " << varName << endl;
				varIndex = -1;
			}

			// Stores the token that has just been modified 
			// along with the rest of its contents into a
			// string.
			string newTokenVal = "";
			for (int z = 0; z < carrToks.size(); z++)
			{
				newTokenVal += carrToks[z];
				if (z != carrToks.size()-1)
					newTokenVal += " ";
			}

			// Stores the newTokenVal into tokens[] vector
			tokens[tokenCarrIndex] = newTokenVal;

			numCarrs--;
		}

	}

	// Prints the tokens after the expansion
	if(showTok[1] == "1" && expansionPres == 1)
	{
		cout << "After Expansion:" << endl;
		for (int i = 0; i < tokens.size(); i++)
		{
			cout << "Token = " << tokens[i] << endl;
		}
	}		

	return tokens;
}

// Stores a variable and its value into the vector<stirng> variable.
// Even index have the variable name and odd index have the value
void setvar(vector<string> token, vector<string> &variables, vector<string> &showTok)
{	

	// Used to check if a variable is already in the vector
	int present = -1;

	// Fills the vector showTok with "ShowTokens" and its value
	if(token[1] == "ShowTokens")
	{
		showTok[0] = token[1];
		showTok[1] = token[2];
	}

	// Goes through the vector "variable" and increments present
	// if the variable in token[1] is already present in the vector "variable"
	for(int i = 0; i < variables.size(); i+=2)
	{
		if (token[1] != "ShowTokens")
		{
			if(token[1] == variables[i])
			{
				present = i;
			}
		}
	}

	if (token[1] != "ShowTokens")
	{
		// If the variable is already in there then it stores 
		// token[2] (the value of the users variable) as the value
		// of the variable that is already present
		if(present != -1)
		{	
			variables[present+1] = token[2];
		}

		// When the variable is not already in the vector "variable"
		// then it puts the variable and it's value onto the end of 
		// vector "variable"
		else
		{
			variables.push_back(token[1]);
			variables.push_back(token[2]);
		}
	}

}

// Sets the prompt to the user's input rather than "msh >"
void setprompt(string &prompt, vector<string> token)
{
	prompt = token[1] + " > ";
}

// Changes the directory to the value of token[1]  
void setdir(vector<string> token, string &prompt)
{

	// The name of the directory
	const char *dirName = token[1].c_str();

	// Checks to see if chdir() succeeds based on a valid directory
	if(chdir(dirName) == 0)
	{

		// Changes directory to back one	
		if(token[1] == "..")
		{
			// The directory that's back one from 
			// the current directory
			char *prev;
			prev = getcwd(prev,0);

			// Changes the directory
			chdir(prev);
			free(prev);
			return;	
		}
		chdir(dirName);
	}
	else
	{
		cerr << "Could not change directory." << endl;
		return;
	}

	/*	char *buf;
		size_t size;
		string cwd = getcwd(buf, size);
		cout << cwd << endl;
	 */

}

// Exits the program with a specific status
void exitProgram(vector<string> token)
{
	// Converts the exit value from a string type to an integer
	int exitVal = atoi(token[1].c_str());

	// Exits with value
	exit(exitVal);
}

void runProgram(vector<string> token, vector<string> variables)
{

	// Creates a string containing the cmd and its parameters
	char *argv[token.size()];
	for(int i = 1 ; i < token.size(); i++)
	{
		argv[i-1] = const_cast<char *>(token[i].c_str());
	}
	argv[token.size()-1] = NULL;

	pid_t pid = fork();

	// This a parent so wait for the child 
	// to finish before being finishing itself
	if (pid > 0)
	{
		int status;
		waitpid(pid,&status,0);
	}

	// This is a child so it executes the child
	else if (pid == 0)
	{
		// Executes child when it has a full path name
		// or is in the current directory
		if(argv[0][0] == '/' || argv[0][0] == '.')
		{
			if(execv(argv[0],argv) == -1)	
			{
				cerr << "No such file or directory " << token[1] << endl;
			}
		}

		// Finds the file where the command is and then executes
		else
		{	


			vector<string> directories;
			istringstream iss(variables[1]);

			string directory;


			// Tokenizes the users input
			// by ':'
			while(getline(iss, directory, ':') )
			{	
				directories.push_back(directory);
			}

			int fails = 0;
			// Runs exec for all directory path possibilites
			for (int i = 0; i < directories.size(); i++)
			{
				directories[i] += "/";
				directories[i] += token[1];
				argv[0] = const_cast<char *>(directories[i].c_str());

				if(execv(argv[0], argv) == -1)
				{
					fails++;
				}			
			}

			if(fails == directories.size())
			{
				cerr << "No such file or directory " << token[1] << endl;
			}

		}
	}
}

void flyProgram(vector<string> token, vector<string> variables, pid_t &flypid)
{	
	// Creates a string containing a command and its parameters
	char *argv[token.size()];
	for(int i = 1 ; i < token.size(); i++)
	{
		argv[i-1] = const_cast<char *>(token[i].c_str());
	}
	argv[token.size()-1] = NULL;

	flypid = fork();

	// This is a child so it executes the child
	if (flypid == 0)
	{
		// Executes child when it has a full path name
		// or is in the current directory
		if(argv[0][0] == '/' || argv[0][0] == '.')
		{
			if(execv(argv[0],argv) == -1)
			{
				cerr << "No such file or directory " << token[1] << endl;
			}

		}

		// Finds the file where the command is and then executes it
		else
		{	


			vector<string> directories;
			istringstream iss(variables[1]);

			string directory;


			// Tokenizes the users input
			// by ':'
			while(getline(iss, directory, ':') )
			{	
				directories.push_back(directory);
			}

			int fails = 0;
			// Runs exec for a directory paths possible
			for (int i = 0; i < directories.size(); i++)
			{
				directories[i] += "/";
				directories[i] += token[1];
				argv[0] = const_cast<char *>(directories[i].c_str());

				if(execv(argv[0], argv) == -1)
				{
					fails++;

				}				
			}

			if(fails == directories.size())
			{
				cerr << "No such file or directory " << token[1] << endl;
			}	

		}
	}


}

void tovar(vector<string> token, vector<string> & variables, vector<string> showTok)
{
	// Creates a string containing the cmd and its parameters
	char *argv[token.size()];
	for(int i = 2 ; i < token.size(); i++)
	{
		argv[i-2] = const_cast<char *>(token[i].c_str());
	}
	argv[token.size()-2] = NULL;
	pid_t pid = fork();

	// Writes stdout to the file
	int file = dup(fileno(stdout));
	freopen("tovar.txt","w",stdout);

	// This a parent so wait for the child 
	// to finish before being finishing itself
	if (pid > 0)
	{
		int status;
		waitpid(pid,&status,0);

		// Reopens stdout and closes file
		dup2(file,fileno(stdout));
		close(file);
	}

	// This is a child so it executes the child
	else if (pid == 0)
	{
		// Executes child when it has a full path name
		// or is in the current directory
		if(argv[0][0] == '/' || argv[0][0] == '.')
		{

			if(execv(argv[0],argv) == -1)
			{
				cerr << "No such file or directory " << argv[0] << endl;
			}
		}

		// Finds the file where the command is and then executes
		else
		{	


			vector<string> directories;
			istringstream iss(variables[1]);

			string directory;


			// Tokenizes the users input
			// by ':'
			while(getline(iss, directory, ':') )
			{	
				directories.push_back(directory);
			}

			int fails = 0;
			// Runs exec for all directory path possibilites
			for (int i = 0; i < directories.size(); i++)
			{
				directories[i] += "/";
				directories[i] += token[2];
				argv[0] = const_cast<char *>(directories[i].c_str());

				if(execv(argv[0], argv) != -1)
				{
					fails++;
				}
			}

			if(fails == directories.size())
			{
				cerr << "No such file or directory " << token[1] << endl;
			}
		}
	}

	string userVar = token[1];
	int varIndex = -1;

	// Checks to see if the variable has alredy been 
	// defined in the vector "variables"
	for(int i = 0; i < variables.size(); i++)
	{
		if (userVar == variables[i])
		{
			varIndex = i;
		}
	}

	// Creates a string that has the content of tovar.txt 
	ifstream ifs("tovar.txt");
	string fcontent((istreambuf_iterator<char>(ifs)),(istreambuf_iterator<char>()));
	close(file);
	// Variable is present so set its value equal to the content of tovar.txt
	if(varIndex != -1)
	{
		variables[varIndex + 1] = fcontent;
	}

	// Variable isn't present so run setvar to put the variable
	// into the vector "variables" and set its value equal to 
	// the content of tovar.txt
	else
	{
		variables.push_back(token[1]);
		variables.push_back(fcontent);
	}

}

void showprocs(vector<string> token, vector<string> flyprocs, vector<pid_t> pidVals )
{
	int status;
	int count = 0;

	// Prints all the processes that were completed
	// since the last command
	for(int i = 0; i < flyprocs.size(); i++)
	{
		pid_t result = waitpid(pidVals[i], &status, WNOHANG);

		if(result != 0 && result != -1)
		{
			cout << "Completed: " << flyprocs[i] << endl;
		}	
	}

	// Prints the processes that are still running
	// based on its running status
	for(int i = 0; i < flyprocs.size(); i++)
	{
		int result = waitpid(pidVals[i],&status,WNOHANG);
		if(result == 0)
		{
			if(count == 0)
			{
				cout << "Background Processes: " << endl;
			}

			cout << "\t" << flyprocs[i] << endl;
			count++;
		}
	}

	if(count == 0)
	{
		cout << "No background proccesses." << endl;
	}
}


int main(){

	// User inputs command
	string command;

	// Prints the user prompt
	string prompt = "msh > ";
	cout << prompt;

	// Reads and stores command into a string
	getline(cin,command);

	// Used to hold the name of variables (even index)
	// and their values (odd index)
	vector<string> variables;

	int varIndex = 0;

	// Default variable and value
	variables.push_back("PATH");
	variables.push_back("/bin:/usr/bin");

	// Stores "ShowTokens" value into a vector
	vector<string> showTok(2);

	// Runs the tokenize function and stores the return
	// into a vector of tokens. This is the user's tokens
	vector<string> token = tokenize(command,variables,showTok,varIndex);

	// The pid used for the fly command and the 
	// vector of processes currently being ran
	pid_t flypid;
	vector<string> flyprocs;
	vector<pid_t> pidVals;

	while(!cin.eof())
	{

		// Executes exitProgram() when token[0] is done
		if(token[0] == "done")
		{

			if(token.size() == sizeof2 || token.size() == 1)
			{
				// Exits with status 0 if user did not specify exit status
				if(token.size() == 1)
				{
					exit(0);
				}
				// Exits with status that the user specified
				else if(atoi(token[1].c_str()) >= 0)
				{
					exitProgram(token);
				}
				else
				{
					cerr << "Exit status must be a non-negative integer." << endl;
				}
			}
			else
			{
				cerr << "Too many parameters to done." << endl;
			}

		}

		// Considers executing setvar() when token[0] is setvar
		else if(token[0] == "setvar")
		{
			// Executes when given 3 tokens
			if(token.size() == sizeof3)
			{
				setvar(token, variables, showTok);
			}
			else
				cerr << "Expected 3 tokens, got " << token.size() << " tokens." << endl;

		}

		// Considers executing setprompt() when token[0] is setprompt
		else if(token[0] == "setprompt")
		{
			// Executes when given 2 tokens
			if(token.size() == sizeof2)
			{
				setprompt(prompt,token);
			}
			else
				cerr << "Expected 2 tokens, got " << token.size() << " tokens." << endl;
		}

		// Considers executing setdir() when token[0] is setdir
		else if(token[0] == "setdir")
		{
			// Executes when given 2 tokens
			if(token.size() == sizeof2)
			{
				setdir(token,prompt);
			}
			else
				cerr << "Expected 2 tokens, got " << token.size() << " tokens." << endl;
		}

		// Executes runProgram() when token[0] is run
		else if(token[0] == "run")
		{
			if(varIndex == 1 || varIndex == 0)
				runProgram(token, variables);
		}

		// Executes flyProgram when token[0] is fly
		else if(token[0] == "fly")
		{
			if(varIndex == 1 || varIndex == 0)
			{
				flyProgram(token,variables,flypid);
				flyprocs.push_back(token[1]);
				pidVals.push_back(flypid);
			}
		}

		else if(token[0] == "tovar")
		{
			tovar(token,variables,showTok);
		}

		else if(token[0] == "showprocs")
		{
			showprocs(token,flyprocs,pidVals);	
		}
		else if(command == "")
		{
			;
		}
		else
		{
			cerr << "Invalid command " << token[0] << endl;
		}

		// Checks the status of each process and prints the ones 
		// that have just completed
		int status;
		for(int i = 0; i < flyprocs.size(); i++)
		{
			pid_t result = waitpid(pidVals[i], &status, WNOHANG);

			if(result != 0 && result != -1)
			{
				cout << "Completed: " << flyprocs[i] << endl;
			}	
		}

		// User inputs command and tokenizes the command
		// Prints the prompt, reads in the command and tokenizes it 
		cout << prompt;
		getline(cin,command);
		token = tokenize(command,variables,showTok,varIndex);
	}		

	return 0;
}
