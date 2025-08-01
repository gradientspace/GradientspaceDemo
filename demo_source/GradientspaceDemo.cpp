// GradientspaceDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Mesh/DenseMesh.h"
#include "MeshIO/OBJReader.h"
#include "ModelGrid/ModelGrid.h"
#include "ModelGrid/ModelGridCell.h"
#include "ModelGrid/ModelGridEditor.h"
#include "ModelGrid/ModelGridEditMachine.h"
#include "ModelGrid/ModelGridSerializer.h"

int main()
{
    std::cout << "Hello World!\n";

    // read OBJ file into a DenseMesh
    GS::DenseMesh DenseMesh;
    GS::OBJFormatData OBJData;
    bool bOBJReadOK = GS::OBJReader::ReadOBJ("C:\\scratch\\bunny_open_200.obj", OBJData);
    std::cout << "OBJ Mesh Read ok: " << bOBJReadOK << std::endl;

    // create a ModelGrid
    GS::ModelGrid Grid;
    Grid.Initialize(GS::Vector3d::One());
    // high-level ModelGrid edits via ModelGridEditMachine
    GS::ModelGridEditMachine EditMachine;
    EditMachine.Initialize(Grid);
    EditMachine.SetCurrentDrawCellType(GS::EModelGridCellType::Ramp_Parametric);
    EditMachine.SetActiveDrawPlaneNormal(GS::Vector3d::UnitZ());
    EditMachine.BeginSculptCells_Rect2D();
    EditMachine.SetInitialCellCursor(GS::Vector3i(-5, -5, 0), GS::Vector3d(-5,-5,0), GS::Vector3d::UnitZ());
    EditMachine.UpdateCellCursor(GS::Vector3i(5, 5, 0));    // fills 1x1 rectangle (-5 to +5, inclusive)
    EditMachine.EndCurrentInteraction();

    // low-level ModelGrid edits via ModelGridEditor
    GS::ModelGridEditor Editor(Grid);

    int NumFilledCells = 0;
    Grid.EnumerateFilledCells([&](GS::Vector3i Key, const GS::ModelGridCell& CellInfo, GS::AxisBox3d LocalBounds) {
        NumFilledCells++;
        Editor.PaintCell(GS::Vector3i::Zero(), GS::Color3b::Red());
    });
    gs_debug_assert(NumFilledCells == 11*11);

    // ModelGrid binary serialization
    GS::MemorySerializer Serializer;
    Serializer.BeginWrite();
    bool bStoreOK = GS::ModelGridSerializer::Serialize(Grid, Serializer);
    std::cout << "Grid write ok: " << bStoreOK << std::endl;
    GS::ModelGrid RestoredGrid;
    Serializer.BeginRead();
    bool bRestoreOK = GS::ModelGridSerializer::Restore(RestoredGrid, Serializer);
    std::cout << "Grid read ok: " << bStoreOK << std::endl;

}

