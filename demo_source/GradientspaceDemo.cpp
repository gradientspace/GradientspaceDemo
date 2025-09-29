// GradientspaceDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Core/TextIO.h"
#include "Core/BinaryIO.h"
#include "Mesh/DenseMesh.h"
#include "MeshIO/OBJReader.h"
#include "MeshIO/OBJWriter.h"
#include "MeshIO/STLReader.h"
#include "MeshIO/STLWriter.h"


#define ENABLE_GRADIENTSPACE_GRID
#ifdef ENABLE_GRADIENTSPACE_GRID
#include "ModelGrid/ModelGrid.h"
#include "ModelGrid/ModelGridCell.h"
#include "ModelGrid/ModelGridEditor.h"
#include "ModelGrid/ModelGridEditMachine.h"
#include "ModelGrid/ModelGridSerializer.h"
#endif

int main()
{
    std::cout << "Hello World!\n";

	auto WriteDenseMeshOBJ = [](GS::DenseMesh& Mesh, std::string filename)
	{
		GS::OBJFormatData WriteOBJData;
		GS::DenseMeshToOBJFormatData(Mesh, WriteOBJData);
		auto Writer = GS::FileTextWriter::OpenFile(filename);
		GS::OBJWriter::WriteOBJ(Writer, WriteOBJData);
		Writer.CloseFile();
	};

    GS::DenseMesh TmpMesh;

    std::string testFilesPath = "c:\\scratch\\";
    std::string writeFilesPath = "c:\\scratch\\";

    // read OBJ file into a DenseMesh
    GS::OBJFormatData OBJData;
    bool bOBJReadOK = GS::OBJReader::ReadOBJ(testFilesPath+"bunny_open_200.obj", OBJData);
    std::cout << "OBJ Mesh Read ok: " << bOBJReadOK << std::endl;

    GS::STLReader::STLMeshData STLAsciiMesh;
	bool bSTLAsciiReadOK = GS::STLReader::ReadSTL(testFilesPath + "bunny_ascii.stl", STLAsciiMesh);
    TmpMesh.Clear();
    GS::STLReader::STLMeshToDenseMesh(STLAsciiMesh, TmpMesh);
    WriteDenseMeshOBJ(TmpMesh, writeFilesPath+"bunny_ascii_stl_out.obj");
    std::cout << "STL Ascii Mesh Read ok: " << bSTLAsciiReadOK << std::endl;
    bool bSTLAsciiWriteOK = GS::STLWriter::WriteSTL(writeFilesPath + "bunny_ascii_stl_out.stl", TmpMesh, "bunny_ascii", false);
    std::cout << "STL Ascii Mesh Write ok: " << bSTLAsciiWriteOK << std::endl;
    GS::STLReader::STLMeshData STLAsciiReadbackMesh;
    bool bSTLAsciiReadbackOK = GS::STLReader::ReadSTL(writeFilesPath + "bunny_ascii_stl_out.stl", STLAsciiReadbackMesh);
	std::cout << "STL Ascii Mesh Readback ok: " << bSTLAsciiReadbackOK << std::endl;

    GS::STLReader::STLMeshData STLBinaryMesh;
    bool bSTLBinaryReadOK = GS::STLReader::ReadSTL(testFilesPath + "bunny_binary.stl", STLBinaryMesh);
    TmpMesh.Clear();
    GS::STLReader::STLMeshToDenseMesh(STLBinaryMesh, TmpMesh);
    WriteDenseMeshOBJ(TmpMesh, writeFilesPath+"bunny_binary_stl_out.obj");
    std::cout << "STL Binary Mesh Read ok: " << bSTLBinaryReadOK << std::endl;
    bool bSTLBinaryWriteOK = GS::STLWriter::WriteSTL(writeFilesPath + "bunny_binary_stl_out.stl", TmpMesh, "bunny_ascii", true);
    std::cout << "STL Binary Mesh Write ok: " << bSTLBinaryWriteOK << std::endl;
    GS::STLReader::STLMeshData STLBinaryReadbackMesh;
    bool bSTLBinaryReadbackOK = GS::STLReader::ReadSTL(writeFilesPath + "bunny_binary_stl_out.stl", STLBinaryReadbackMesh);
    std::cout << "STL Ascii Mesh Readback ok: " << bSTLBinaryReadbackOK << std::endl;

#ifdef ENABLE_GRADIENTSPACE_GRID

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
#endif
}

