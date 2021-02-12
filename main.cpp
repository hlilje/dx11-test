#include "window.hpp"


int main() {
	Window window;

	if (!window.Create())
		return -1;

	window.Run();

	return 0;
}
