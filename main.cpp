#include "Pipes3DDriver.h"
#include <ctime>
#include <cstdlib>

int main(int argc, char* argv[])
{
    srand(time(nullptr));
	Pipes3DDriver driver(400, 300, 3);
	driver.run();

	return 0;
}
