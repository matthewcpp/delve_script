#include "console.h"

int main(int argc, char** argv) 
{
	Delve::Script::Console console;
	console.runInteractive();

	return 0;
}