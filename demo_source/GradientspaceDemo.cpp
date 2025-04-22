// GradientspaceDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Mesh/DenseMesh.h"
#include "MeshIO/OBJReader.h"

int main()
{
    std::cout << "Hello World!\n";

    GS::DenseMesh DenseMesh;
    GS::OBJFormatData OBJData;
    bool bReadOK = GS::OBJReader::ReadOBJ("C:\\scratch\\bunny_open_200.obj", OBJData);
}

