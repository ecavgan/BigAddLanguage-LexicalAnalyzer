#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char keyword_list[10][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline"};
char recognized_char_list[9][1] = {'.', ',', '-', '_', '[', ']', '{', '}', '"'};


int isKeyword(char s[]);
int isCharRecognized(char ch);


int main(int argc, char* argv[])
{	
	FILE* inputFile;
	FILE* outputFile;
	
	char inputFileName[30] = "";
	
	strcat(inputFileName, argv[1]);
	strcat(inputFileName, ".ba");
	
	inputFile = fopen(inputFileName, "r");
	outputFile = fopen("myscript.lx", "w");
	
	if (inputFile == NULL)
	{
		fprintf(outputFile, "FileNotFound");
		printf("FileNotFound\n");
		exit(1);
	}
	
	printf("FileReadSuccessful\n");
	
	char *word = calloc(5, sizeof(char));
	
	int state = 0; // initial state
	int i = 0; // for storing word's lenght
	char ch;
	
	int commentIsClosed = 1; // flag
	int stringConstantIsClosed = 1; // flag
		
	while(!feof(inputFile))
	{
		switch (state)
		{
			case 0: // determining the next state or failure
				ch = fgetc(inputFile);	
				i = 0; // resetting word index

				if (isspace(ch) || ch == EOF)
				{
					state = 0;
					break;	
				}
				if (ch == '.')
				{
					state = 0;
					fprintf(outputFile, "EndOfLine\n");
					break;
				}
				if (ch == ',')
				{
					state = 0;
					fprintf(outputFile, "Seperator\n");
					break;
				}
				if (ch == '_')
				{
					state = 0;
					fprintf(outputFile, "UnderScore\n");
					break;	
				}
				if (ch == '[')
				{
					fprintf(outputFile, "OpenBlock\n");
					state = 0;
					break;	
				}
				if (ch == ']')
				{
					fprintf(outputFile, "CloseBlock\n");
					state = 0;
					break;	
				}
				if (!isCharRecognized(ch))
				{
					fprintf(outputFile, "UnrecognizedCharacter %c\n", ch);
					exit(1);
				}
				if (isalpha(ch))
				{
					word = calloc(25, sizeof(char));
					
					word[i++] = ch;
					state = 1;	
					break;
				}
				if (isdigit(ch) || ch == '-')
				{
					word = calloc(105, sizeof(char));
					
					word[i++] = ch;
					state = 2;
					break;
				}
				if (ch == '"')
				{
					word = calloc(1005, sizeof(char));
					
					word[i++] = ch;
					stringConstantIsClosed = 0;
					state = 3;
					break;
				}
				if (ch == '{')
				{
					commentIsClosed = 0;
					state = 4;
					break;	
				}
				if (ch == '}' && commentIsClosed)
				{
					fprintf(outputFile, "LeftCurlyBracketMissing");
					exit(1);
				}
				
				break;	
				
			case 1: // reading identifier or keyword
				ch = fgetc(inputFile);

				if (!isalnum(ch) && ch != '_')
				{
					word[i] = '\0';
					
					if (isKeyword(word))
						fprintf(outputFile, "Keyword %s\n", word);
					else
						fprintf(outputFile, "Identifier %s\n", word);
					
					// if ch is end of file and we fseek 1 back, this would create an infinite loop
					if (ch != EOF)
						fseek(inputFile, -1, SEEK_CUR);

					state = 0;
					break;	
				}				
				
				word[i++] = ch;
				
				if (strlen(word) == 21)
				{
					fprintf(outputFile, "VariableNameTooLong");
					exit(1);
				}
				
				break;
				
			case 2: // reading integer constant
				ch = fgetc(inputFile);
				
				
				if (word[0] == '-' && ch == '0') // -0 or -023 is not valid int. constant
				{
					fprintf(outputFile, "IntConstantCannotStartWithZero");
					exit(1);
				}
				
				if (word[0] == '0' && isdigit(ch)) // 053 is not valid int. constant
				{
					fprintf(outputFile, "IntConstantCannotStartWithZero");
					exit(1);
				}

				if (!isdigit(ch))
				{
					word[i] = '\0';
					
					if (strlen(word) == 1 && word[0] == '-') // if word == -
						fprintf(outputFile, "MinusSign\n");
					else
						fprintf(outputFile, "IntConstant %s\n", word);
					
					// if ch is end of file and we fseek 1 back, this would create an infinite loop
					if (ch != EOF)
						fseek(inputFile, -1, SEEK_CUR);
						
					state = 0;
					break;
				}
				
				word[i++] = ch;
				
				if ((word[0] != '-' && strlen(word) == 101) || (word[0] == '-' && strlen(word) == 102)) 
				{
					fprintf(outputFile, "IntConstantTooBig");
					exit(1);
				}

				break;
			
			case 3: // reading string constant
				ch = fgetc(inputFile);
				word[i++] = ch;
 				
				if (ch == '"')
				{
					word[i] = '\0';
					fprintf(outputFile, "StringConstant %s\n", word);
					
					stringConstantIsClosed = 1;
					state = 0;
					break;
				}
				
				if (strlen(word) == 1002)
				{
					fprintf(outputFile, "StringConstantTooLong");
					exit(1);
				}
				
				break;
				
			case 4: // comment
				ch = fgetc(inputFile);
				
				// ignoring everything unless ch is }
				if (ch == '}')
				{
					commentIsClosed = 1;
					state = 0;
					break;
				}
				
				break;
				
		}
	}
	
	fclose(inputFile);
	
	if (!commentIsClosed)
		fprintf(outputFile, "CommentLeftOpen");
	
	if (!stringConstantIsClosed)
		fprintf(outputFile, "StringConstantLeftOpen");
	
	
	
	return 0;
}

int isKeyword(char s[])
{
	// max. keyword is newline and its length is 7
	// so if s's length is greater then 7 it cannot be a keyword
	// there is no need to search it in the list
	if (strlen(s) > 7)
		return 0;
		
	int i;
	
	for (i = 0; i < 10; i++)
		if (strcmp(s, keyword_list[i]) == 0)
			return 1;
	
	return 0;
}

int isCharRecognized(char ch)
{
	// if ch is alphanumerical then it is recognized
	// there is no need to search it in the list
	if (isalnum(ch))
		return 1;
		
	int i;
	
	for(i = 0; i < 10; i++)
		if (*recognized_char_list[i] == ch)
			return 1;
	
	return 0;
}

