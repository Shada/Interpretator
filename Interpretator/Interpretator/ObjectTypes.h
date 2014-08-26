#include <string>

enum TOKENTYPE
{
	TOKENTYPE_NULL = -1,
	TOKENTYPE_ID = 1,
	TOKENTYPE_INTEGER,
	TOKENTYPE_REAL,
	TOKENTYPE_STRING,
	TOKENTYPE_OP,
	TOKENTYPE_ERROR,
	TOKENTYPE_LINE,
	TOKENTYPE_CALL = 10,
};

enum SYMBOLKIND
{
	SYMBOLKIND_UNDEF = -1,
	SYMBOLKIND_SIMPLE,
	SYMBOLKIND_ARRAY,
	SYMBOLKIND_FUNC,
	SYMBOLKIND_FUNCVAL,
};

struct Symbol
{
	int ID;
	int TYPE;
	SYMBOLKIND KIND;
	int INFO1;
	int INFO2;
	int INFO3;
	int RELADR;
	std::string NAME;
	int VALUEPTR;

	Symbol()
	{
		ID = TYPE = INFO1 = INFO2 = INFO3 = RELADR = VALUEPTR = -1;
		KIND = SYMBOLKIND_UNDEF;
	}
};

struct Token
{
	TOKENTYPE TYPE;
	int CODE;
	std::string STRING;

	Token()
	{
		CODE = -1;
		TYPE = TOKENTYPE::TOKENTYPE_NULL;
		STRING = -1;
	}
	TOKENTYPE GETTYPE()
	{
		return TYPE;
	}
	int GETID()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_ID)
		{
			return CODE;
		}
		return -1;
	}
	int GETCODE()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_OP)
		{
			return CODE;
		}
		return -1;
	}
	int GETLINE()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_LINE)
		{
			return CODE;
		}
		return -1;
	}
	int GETBLKNR()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_CALL)
		{
			return CODE;
		}
		return -1;
	}
	std::string GETNAME()
	{
		return STRING;
	}
	int GETINT()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_INTEGER)
		{
			return std::atoi(STRING.c_str());
		}
		return -1;
	}
	float GETREAL()
	{
		if(TYPE == TOKENTYPE::TOKENTYPE_REAL)
		{
			return std::atof(STRING.c_str());
		}
		return -1;
	}
};

struct Block
{
	int BLKNR;
	int SF;
	int SIZE;
	int SYMAMOUNT;
	int TOKENAMOUNT;
	int CURRENTTOKEN;
	std::vector<Symbol> SYMBOLS;
	std::vector<Token> TOKENS;

	std::vector<Token> stack;
	std::vector<Token> operatorstack;
};

struct Res
{
	int NUMBLOCKS;
	int NUMCALLBLOCKS;
	std::vector<Block> BLOCKS;
	std::vector<Block> CALLBLOCKS; // blocks that have been called

	Res(int _NUMBLOCKS)
	{
		NUMBLOCKS = _NUMBLOCKS;
		NUMCALLBLOCKS = 1;
		BLOCKS.resize(NUMBLOCKS);
	}

	Token GETCODE(int BLKID)
	{
		Token t;
		while(true)
		{
			if(CALLBLOCKS[BLKID].CURRENTTOKEN >= CALLBLOCKS[BLKID].TOKENAMOUNT) // no more tokens
			{
				break;
			}
			t = CALLBLOCKS[BLKID].TOKENS[CALLBLOCKS[BLKID].CURRENTTOKEN];
			CALLBLOCKS[BLKID].CURRENTTOKEN++;
			if(t.GETLINE() == -1)
			{
				break;
			}
		}
		return t;
	}

	Symbol *SEARCH(int BLKID, int SYMID)
	{
		Symbol *s = nullptr;

		while (BLKID != -1)
		{
			for (int i = 0; i < CALLBLOCKS[BLKID].SYMAMOUNT; i++)
			{
				if(CALLBLOCKS[BLKID].SYMBOLS[i].ID == SYMID)
				{
					s = &CALLBLOCKS[BLKID].SYMBOLS[i];
					BLKID = -1;
					break;
				}
			}
			if(s == nullptr)
			{
				BLKID = CALLBLOCKS[BLKID].SF;
			}
		}
		return s;
	}
};