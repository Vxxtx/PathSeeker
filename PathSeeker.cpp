// PathSeeker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Application.h"

int main()
{
start:
    Application* App = new Application();

#if ASK_USER_FOR_INPUT
    std::cout << "Generate new map? Y/N: ";
    char yn;
    std::cin >> yn;

    if (yn == 'y' || yn == 'Y')
#endif
        goto start;
}
