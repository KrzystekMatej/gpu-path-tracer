#include "TestApp.hpp"

#include <iostream>
#include "TestCore.hpp"

void TestApp::Run()
{
	std::cout << "Hello from App!" << std::endl;
	TestCore core;
	core.Run();
}