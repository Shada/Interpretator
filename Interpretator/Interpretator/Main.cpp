#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ObjectTypes.h"

char ActionMatrix[17][17]=
  //       ;   (   )           :=                  >       +   -   x   ÷   F(
{ /* ;*/ {'A','S','E','?','?','S','?','?','?','?','S','?','S','S','S','S','E'}, // ;
  /* (*/ {'E','S','P','?','?','S','?','?','?','?','S','?','S','S','S','S','E'}, // (
  /* )*/ {'E','E','E','E','E','E','E','E','E','E','E','E','E','E','E','E','E'}, // )
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /*:=*/ {'U','S','E','?','?','E','?','?','?','?','S','?','S','S','S','S','E'}, // :=
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /* >*/ {'U','S','U','?','?','E','?','?','?','?','E','?','S','S','S','S','E'}, // >
  /*  */ {'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','E'}, //
  /* +*/ {'U','S','U','?','?','E','?','?','?','?','U','?','U','U','S','S','E'}, // +
  /* -*/ {'U','S','U','?','?','E','?','?','?','?','U','?','U','U','S','S','E'}, // -
  /* x*/ {'U','S','U','?','?','E','?','?','?','?','U','?','U','U','U','U','E'}, // x
  /* ÷*/ {'U','S','U','?','?','E','?','?','?','?','U','?','U','U','U','U','E'}, // ÷
  /*F(*/ {'E','S','L','?','?','S','?','?','?','?','S','?','S','S','S','S','E'}, // F(
  //       ;   (   )           :=                  >       +   -   x   ÷   F(  
};

Res *RESULT = nullptr;

int currentBlock = 0;

std::vector<bool> ifstatementresults;

std::ifstream myfile;

bool loadFile(std::string);
void interpretProgram();
void processToken(Token);

void U();
void L();
void muladdop(int);
void eqop();
void compareop();

void printoutsymbols();
void loadBlock(std::string, Block&);
void loadSymbol(std::string, Symbol&);
void loadToken(std::string, Token&);

std::vector<std::pair<TOKENTYPE, int>> *symbolValues;

std::string getStringElement(std::string, int);
void printLoadedStuff();

int main(int argc, char** argv)
{
	symbolValues = new std::vector<std::pair<TOKENTYPE, int>>();
	//if(argc != 2)
	//{
	//	std::cout << "Usage: preterp [filepath]" << std::endl;
	//	return 0;
	//}
	std::string filepath = "tst7.p1";// argv[1];

	if(!loadFile(filepath))
	{
		return 0;
	}

	// print loaded stuff to check
	std::cout << "Printing loaded program:" << std::endl;
	printLoadedStuff();	
	
	std::cout << "\n---------------------------\nStarting program" << std::endl;

	RESULT->CALLBLOCKS.push_back(RESULT->BLOCKS[0]);
	interpretProgram();
	
	std::cout << "\n---------------------------\nProgram has reached it's end. Exiting program" << std::endl;

	//std::cout << "\n---------------------------\nAll the symbols" << std::endl;
	//printoutsymbols();
	_sleep(10000.0);
	return 0;
}

void interpretProgram()
{
	while(true)
	{
		Token t = RESULT->GETCODE(currentBlock);
		
		if(t.GETID() == 2 || t.TYPE == TOKENTYPE::TOKENTYPE_NULL)
		{
			int temp = RESULT->CALLBLOCKS[currentBlock].SF;
		
			if(temp == -1)
				break;
			else
			{
				std::cout << "End of block. Returning to SF: " << temp << std::endl;
				if(!RESULT->CALLBLOCKS[temp].stack.empty() && RESULT->CALLBLOCKS[temp].stack.back().GETID() > 18)
				{
					Symbol *s = RESULT->SEARCH(currentBlock, RESULT->CALLBLOCKS[temp].stack.back().GETID());
					if(s)
					{
						RESULT->CALLBLOCKS[temp].stack.back().CODE = symbolValues->at(s->VALUEPTR).second;
						RESULT->CALLBLOCKS[temp].stack.back().STRING = std::to_string(RESULT->CALLBLOCKS[temp].stack.back().CODE).c_str();
						RESULT->CALLBLOCKS[temp].stack.back().TYPE = symbolValues->at(s->VALUEPTR).first;
					}
				}
				RESULT->CALLBLOCKS.pop_back();
				RESULT->NUMCALLBLOCKS--;
				currentBlock = temp;
			}
		}
		else if(t.TYPE == TOKENTYPE::TOKENTYPE_CALL)
		{
			std::cout << "Calling new block: " << t.CODE << std::endl;
			RESULT->CALLBLOCKS.push_back(RESULT->BLOCKS[t.CODE]);
			RESULT->CALLBLOCKS.back().SF = currentBlock;
			currentBlock = RESULT->NUMCALLBLOCKS;
			RESULT->NUMCALLBLOCKS++;
		}
		else if(t.GETID() == 10)
		{
			std::cout << "if-statement found" << std::endl;
			continue;
		}
		else if(t.GETID() == 11)
		{
			compareop();
		}
		else if(t.GETID() == 12)
		{
			while(!RESULT->CALLBLOCKS[currentBlock].stack.empty())
				U();
			if(ifstatementresults.back())
			{
				while(RESULT->GETCODE(currentBlock).GETCODE() != 1)
				{
					if(RESULT->GETCODE(currentBlock).GETID() == 2)
					{
						RESULT->CALLBLOCKS[currentBlock].CURRENTTOKEN--;
						break;
					}
				}
			}
			else
			{
				std::cout << "Performing else-part of if-statement" << std::endl;
			}
			ifstatementresults.pop_back();
		}
		else
		{
			processToken(t);
		}
	}
}

void processToken(Token t)
{
	if(t.GETCODE() == -1)
	{
		std::cout << "Stacking Token: " << t.STRING << std::endl;
		RESULT->CALLBLOCKS[currentBlock].stack.push_back(t);
	}
	else
	{
		char action;
		if(RESULT->CALLBLOCKS[currentBlock].operatorstack.empty())
		{
			action = ActionMatrix[0][t.GETCODE() - 1];
		}
		else
		{
			action = ActionMatrix[RESULT->CALLBLOCKS[currentBlock].operatorstack.back().GETCODE() - 1][t.GETCODE() - 1];
		}
		switch(action)
		{
		case 'U': // Execute operation. pop the top
			{
				std::cout << "perform operation" << std::endl;
				U();
				if(t.GETCODE() == 3)
				{
					processToken(t);
				}
				else if(t.CODE != 1)
				{
					RESULT->CALLBLOCKS[currentBlock].operatorstack.push_back(t);
				}
				else
				{
					while (!RESULT->CALLBLOCKS[currentBlock].operatorstack.empty())
					{
						std::cout << "perform operation" << std::endl;
						U();
					}
				}
				break;
			}
		case 'S': // Put the new operator on the stack
			{
				std::cout << "Stacking operator: " << t.STRING << std::endl; 
				if(t.CODE == 2 && RESULT->CALLBLOCKS[currentBlock].stack.back().GETID() > 18)
				{
					int lookingBlock = currentBlock;
					while (lookingBlock >= 0)
					{
						Symbol *s = RESULT->SEARCH(lookingBlock, RESULT->CALLBLOCKS[currentBlock].stack.back().GETID());
						if(s)
						{
							if(s->KIND == 2)
							{
								t.CODE = 17;
								RESULT->CALLBLOCKS.push_back(RESULT->BLOCKS[s->INFO2]);
								RESULT->CALLBLOCKS.back().SF = currentBlock;
								RESULT->NUMCALLBLOCKS++;
								break;
							}
						}
						else
							break;
						lookingBlock = RESULT->CALLBLOCKS[lookingBlock].SF;
					}
				}
				RESULT->CALLBLOCKS[currentBlock].operatorstack.push_back(t);
				break;
			}
		case 'E': // Error
			{
				std::cout << "ERROR!!!!" << std::endl;
				break;
			}
		case 'P': // Pop the top
			{
				std::cout << "Popping Token" << std::endl; 
				RESULT->CALLBLOCKS[currentBlock].operatorstack.pop_back();
				break;
			}
		case 'L': // Perform function call
			{
				std::cout << "Calling a function" << std::endl; 
				L();
				break;
			}
		}
	}
}

void U()
{
	Token operatorToken = RESULT->CALLBLOCKS[currentBlock].operatorstack.back();
	RESULT->CALLBLOCKS[currentBlock].operatorstack.pop_back();

	switch (operatorToken.CODE)
	{
	case 6:
		{
			eqop();
			break;
		}
	case 13: case 14: case 15: case 16:
		{
			muladdop(operatorToken.CODE);
			break;
		}
	default:
		break;
	}
}

void L()
{
	int lastParam = 0;
	if(RESULT->CALLBLOCKS[currentBlock].stack.back().GETID() > 18)
	{
		Symbol *paramSymbol = RESULT->SEARCH(currentBlock, RESULT->CALLBLOCKS[currentBlock].stack.back().GETID());
		lastParam = symbolValues->at(paramSymbol->VALUEPTR).second;
	}
	else
	{
		lastParam = RESULT->CALLBLOCKS[currentBlock].stack.back().CODE;
	}
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();
	
	Symbol *paramSymbol = &RESULT->CALLBLOCKS.back().SYMBOLS.back();

	paramSymbol->VALUEPTR = symbolValues->size();
	symbolValues->push_back(std::make_pair(TOKENTYPE::TOKENTYPE_INTEGER, lastParam));

	RESULT->CALLBLOCKS[currentBlock].operatorstack.pop_back();

	currentBlock = RESULT->CALLBLOCKS.size() - 1;
}

void eqop()
{
	Token right = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();
	Token left = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();
	Symbol *s = RESULT->SEARCH(currentBlock, left.GETID());
	if(s == nullptr)
	{
		std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
		return;
	}
	if(right.GETID() > 18)
	{
		Symbol *rightsymbol = RESULT->SEARCH(currentBlock, right.GETID());
		if(rightsymbol == nullptr)
		{
			std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
			return;
		}
		std::pair<TOKENTYPE, int> rightsymbolvalue;
		rightsymbolvalue = symbolValues->at(rightsymbol->VALUEPTR);
		s->VALUEPTR = symbolValues->size();
		symbolValues->push_back(std::make_pair(rightsymbolvalue.first, rightsymbolvalue.second));
	}
	else if(right.GETINT() != -1)
	{
		s->VALUEPTR = symbolValues->size();
		symbolValues->push_back(std::make_pair(TOKENTYPE::TOKENTYPE_INTEGER, right.GETINT()));
	}
	else if(right.GETREAL() != -1)
	{
		s->VALUEPTR = symbolValues->size();
		symbolValues->push_back(std::make_pair(TOKENTYPE::TOKENTYPE_REAL, right.GETREAL()));
	}
	std::cout << left.STRING << " := " << right.STRING << std::endl;
}

void muladdop(int op)
{
	int rightvalue = 0, leftvalue = 0;
	Token right = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();
	Token left = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();

	Token temp;
	temp.TYPE = TOKENTYPE::TOKENTYPE_INTEGER;

	if(right.GETID() > 18)
	{
		Symbol *rightsymbol = RESULT->SEARCH(currentBlock, right.GETID());
		if(rightsymbol == nullptr)
		{
			std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
			return;
		}
		if(rightsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNC)
		{
			rightvalue = symbolValues->at(RESULT->SEARCH(rightsymbol->INFO2, rightsymbol->ID)->VALUEPTR).second;
		}
		else if(rightsymbol->KIND == SYMBOLKIND::SYMBOLKIND_SIMPLE)
		{
			rightvalue = symbolValues->at(rightsymbol->VALUEPTR).second;
		}
	}
	else if(right.GETINT() != -1)
	{
		rightvalue = right.GETINT();
	}
	else if(right.GETREAL() != -1)
	{
		temp.TYPE = TOKENTYPE::TOKENTYPE_REAL;
		rightvalue = right.GETREAL();
	}

	if(left.GETID() > 18)
	{
		Symbol *leftsymbol = RESULT->SEARCH(currentBlock, left.GETID());
		if(leftsymbol == nullptr)
		{
			std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
			return;
		}
		if(leftsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNC)
		{
			leftvalue = symbolValues->at(RESULT->SEARCH(leftsymbol->INFO2, leftsymbol->ID)->VALUEPTR).second;
		}
		else if(leftsymbol->KIND == SYMBOLKIND::SYMBOLKIND_SIMPLE)
		{
			leftvalue = symbolValues->at(leftsymbol->VALUEPTR).second;
		}
	}
	else if(left.GETINT() != -1)
	{
		leftvalue = left.GETINT();
	}
	else if(left.GETREAL() != -1)
	{
		temp.TYPE = TOKENTYPE::TOKENTYPE_REAL;
		leftvalue = left.GETREAL();
	}
	std::string opsign = "UNDEF";
	switch(op)
	{
	case 13: // +
		temp.CODE = leftvalue + rightvalue;
		temp.STRING = std::to_string(leftvalue + rightvalue);
		opsign = " + ";
		break;
	case 14: // -
		temp.CODE = leftvalue - rightvalue;
		temp.STRING = std::to_string(leftvalue - rightvalue);
		opsign = " - ";
		break;
	case 15: // x
		temp.CODE = leftvalue * rightvalue;
		temp.STRING = std::to_string(leftvalue * rightvalue);
		opsign = " x ";
		break;
	case 16: // ÷
		temp.CODE = leftvalue / rightvalue;
		temp.STRING = std::to_string(leftvalue / rightvalue);
		opsign = " ÷ ";
		break;
	}
	std::cout << leftvalue <<  opsign << rightvalue << " = " << temp.CODE << std::endl;
	RESULT->CALLBLOCKS[currentBlock].stack.push_back(temp);
}

void compareop()
{
	int rightvalue = 0, leftvalue = 0;
	Token right = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();
	Token left = RESULT->CALLBLOCKS[currentBlock].stack.back();
	RESULT->CALLBLOCKS[currentBlock].stack.pop_back();

	Token temp;
	temp.TYPE = TOKENTYPE::TOKENTYPE_INTEGER;

	if(right.GETID() > 18)
	{
		Symbol *rightsymbol = RESULT->SEARCH(currentBlock, right.GETID());
		if(rightsymbol == nullptr)
		{
			std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
			return;
		}
		if(rightsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNC)
		{
			rightvalue = symbolValues->at(RESULT->SEARCH(rightsymbol->INFO2, rightsymbol->ID)->VALUEPTR).second;
		}
		else if(rightsymbol->KIND == SYMBOLKIND::SYMBOLKIND_SIMPLE ||rightsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNCVAL)
		{
			rightvalue = symbolValues->at(rightsymbol->VALUEPTR).second;
		}
	}
	else if(right.GETINT() != -1)
	{
		rightvalue = right.GETINT();
	}
	else if(right.GETREAL() != -1)
	{
		temp.TYPE = TOKENTYPE::TOKENTYPE_REAL;
		rightvalue = right.GETREAL();
	}

	if(left.GETID() > 18)
	{
		Symbol *leftsymbol = RESULT->SEARCH(currentBlock, left.GETID());
		if(leftsymbol == nullptr)
		{
			std::cout << "ERROR: SYMBOL NOT FOUND" << std::endl;
			return;
		}
		if(leftsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNC)
		{
			leftvalue = symbolValues->at(RESULT->SEARCH(leftsymbol->INFO2, leftsymbol->ID)->VALUEPTR).second;
		}
		else if(leftsymbol->KIND == SYMBOLKIND::SYMBOLKIND_SIMPLE ||leftsymbol->KIND == SYMBOLKIND::SYMBOLKIND_FUNCVAL)
		{
			leftvalue = symbolValues->at(leftsymbol->VALUEPTR).second;
		}
	}
	else if(left.GETINT() != -1)
	{
		leftvalue = left.GETINT();
	}
	else if(left.GETREAL() != -1)
	{
		temp.TYPE = TOKENTYPE::TOKENTYPE_REAL;
		leftvalue = left.GETREAL();
	}
	Token op = RESULT->CALLBLOCKS[currentBlock].operatorstack.back();
	RESULT->CALLBLOCKS[currentBlock].operatorstack.pop_back();
	switch(op.GETCODE())
	{
	case 11:
		{
			ifstatementresults.push_back(leftvalue > rightvalue);

			if(!ifstatementresults.back())
			{
				while (RESULT->GETCODE(currentBlock).GETID() != 12)
				{

				}
				ifstatementresults.pop_back();
			}
			break;
		}
	}
}


bool loadFile(std::string filepath)
{
	int numblocks = 0;

	std::string line;
	myfile.open(filepath);
	if(!myfile.is_open())
	{
		std::cout << "Error: File could not be opened" << std::endl;
		return false;
	}
	//load file
	std::getline(myfile, line); // ###PROGRAM###
	std::getline(myfile, line); // num blocks

	numblocks = std::atoi(line.c_str());
	
	RESULT = new Res(numblocks);

	for(int _blockID = 0; _blockID  < numblocks; _blockID ++)
	{
		std::getline(myfile, line); // ##BLOCK##
		
		std::getline(myfile, line); // BLOCKSTRUKTUR
		loadBlock(line, RESULT->BLOCKS[_blockID]);
		
		std::getline(myfile, line); // #DEKLARATIONER#
		for(int _numsym = 0; _numsym < RESULT->BLOCKS[_blockID].SYMAMOUNT; _numsym++)
		{
			std::getline(myfile, line); // SYMBOLSTRUKTUR
			loadSymbol(line, RESULT->BLOCKS[_blockID].SYMBOLS[_numsym]);
		}
		
		std::getline(myfile, line); // #KOD#
		for(int _numtok = 0; _numtok < RESULT->BLOCKS[_blockID].TOKENAMOUNT; _numtok++)
		{
			std::getline(myfile, line); // TOKENSTRUKTUR
			loadToken(line, RESULT->BLOCKS[_blockID].TOKENS[_numtok]);
		}
		
		std::getline(myfile, line); // ##BLOCKSLUT##
	}
	line.clear();
	myfile.close();

	return true;
}

void printoutsymbols()
{
	std::pair<TOKENTYPE, int> tempvalue;
	for (int i = 0; i < RESULT->NUMBLOCKS; i++)
	{
		std::cout << "---BLOCK " << i << "---" << std::endl;
		for (int j = 0; j < RESULT->BLOCKS[i].SYMAMOUNT; j++)
		{
			if(RESULT->BLOCKS[i].SYMBOLS[j].KIND != SYMBOLKIND_FUNC)
				tempvalue = symbolValues->at(RESULT->BLOCKS[i].SYMBOLS[j].VALUEPTR);
			if(tempvalue.first == TOKENTYPE::TOKENTYPE_INTEGER)
			{
				std::cout << RESULT->BLOCKS[i].SYMBOLS[j].NAME << " = " << tempvalue.second << std::endl;
			}
			else if(tempvalue.first == TOKENTYPE::TOKENTYPE_REAL)
			{
				std::cout << RESULT->BLOCKS[i].SYMBOLS[j].NAME << " = " << tempvalue.second << std::endl;
			}
		}
	}
}

void loadBlock(std::string line, Block& _block)
{
	_block.BLKNR = std::atoi(getStringElement(line, 0).c_str());

	_block.SF = std::atoi(getStringElement(line, 1).c_str());

	_block.SIZE = std::atoi(getStringElement(line, 2).c_str());

	_block.SYMAMOUNT = std::atoi(getStringElement(line, 3).c_str());

	_block.TOKENAMOUNT = std::atoi(getStringElement(line, 4).c_str());

	_block.CURRENTTOKEN = 0;

	_block.SYMBOLS.resize(_block.SYMAMOUNT);

	_block.TOKENS.resize(_block.TOKENAMOUNT);
}

void loadSymbol(std::string line, Symbol& _symbol)
{
	_symbol.ID = std::atoi(getStringElement(line, 0).c_str());
	
	_symbol.TYPE = std::atoi(getStringElement(line, 1).c_str());
	
	_symbol.KIND = (SYMBOLKIND)std::atoi(getStringElement(line, 2).c_str());
	
	_symbol.INFO1 = std::atoi(getStringElement(line, 3).c_str());
	
	_symbol.INFO2 = std::atoi(getStringElement(line, 4).c_str());
	
	_symbol.INFO3 = std::atoi(getStringElement(line, 5).c_str());
	
	_symbol.RELADR = std::atoi(getStringElement(line, 6).c_str());

	_symbol.NAME = getStringElement(line, -1);

	_symbol.VALUEPTR = -1;
}

void loadToken(std::string line, Token& _token)
{
	_token.CODE = std::atoi(getStringElement(line, 0).c_str());

	_token.TYPE = (TOKENTYPE)std::atoi(getStringElement(line, 1).c_str());

	_token.STRING = getStringElement(line, 2).c_str();
}

std::string getStringElement(std::string line, int index)
{
	while(line[0] == ' ')
	{
		line = line.substr(line.find_first_of(' ') + 1, line.size());
	} 
	if(index == -1) // last element
	{
		line = line.substr(line.find_last_of(' '), line.size());
	}
	else
	{
		for(int i = 0; i < index; i++)
		{
			line = line.substr(line.find_first_of(' ') + 1, line.size());
			while(line[0] == ' ')
			{
				line = line.substr(line.find_first_of(' ') + 1, line.size());
			} 
		}
		line = line.substr(0, line.find_first_of(' '));
	}

	return line;
}

void printLoadedStuff()
{
	std::cout << "###PROGRAM###" << std::endl;
	std::cout << "\t" << RESULT->NUMBLOCKS << std::endl;
	
	for(int i = 0; i < RESULT->NUMBLOCKS; i++)
	{
		std::cout << "##BLOCK##" << std::endl;
		Block block = RESULT->BLOCKS[i];
		std::cout << "  " << block.BLKNR << " " << block.SF << "  " << block.SIZE 
			<< "  " << block.SYMAMOUNT << "\t" << block.TOKENAMOUNT << std::endl;

		std::cout << "#DEKLARATIONER#" << std::endl;
		for(int j = 0; j < RESULT->BLOCKS[i].SYMAMOUNT; j++)
		{
			Symbol sym = RESULT->BLOCKS[i].SYMBOLS[j];
			std::cout << " " << sym.ID << " " << sym.TYPE << " " << sym.KIND << " " 
				<< sym.VALUEPTR << " " << sym.INFO2 << " " << sym.INFO3 << " " 
				<< sym.RELADR << std::endl;
		}
		std::cout << "#KOD#" << std::endl;
		for(int k = 0; k < RESULT->BLOCKS[i].TOKENAMOUNT; k++)
		{
			Token tok = RESULT->BLOCKS[i].TOKENS[k];
			std::cout << "\t" << tok.CODE << "\t" << tok.TYPE << "\t" << tok.STRING.c_str() << std::endl;
		}
		std::cout << "##BLOCKSLUT##" << std::endl;
	}

	std::cout << "###PROGRAMSLUT###" << std::endl;
}