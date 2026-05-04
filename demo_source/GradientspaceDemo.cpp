// GradientspaceDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <chrono>

#include "Core/TextIO.h"
#include "Core/BinaryIO.h"
#include "Mesh/DenseMesh.h"
#include "MeshIO/OBJReader.h"
#include "MeshIO/OBJWriter.h"
#include "MeshIO/STLReader.h"
#include "MeshIO/STLWriter.h"
#include "MeshIO/GLTFReader.h"
#include "MeshIO/GLTFWriter.h"
#include "MeshIO/GLBReader.h"
#include "MeshIO/GLBWriter.h"


#define ENABLE_GRADIENTSPACE_GRID
#ifdef ENABLE_GRADIENTSPACE_GRID
#include "ModelGrid/ModelGrid.h"
#include "ModelGrid/ModelGridCell.h"
#include "ModelGrid/ModelGridEditor.h"
#include "ModelGrid/ModelGridEditMachine.h"
#include "ModelGrid/ModelGridSerializer.h"
#endif

static void WriteDenseMeshOBJ(GS::DenseMesh& Mesh, const std::string& filename)
{
	using clock = std::chrono::steady_clock;
	auto ms_since = [](clock::time_point t0) {
		return std::chrono::duration<double, std::milli>(clock::now() - t0).count();
	};

	std::cout << "  WriteDenseMeshOBJ: " << filename
		<< "  (verts=" << Mesh.GetVertexCount()
		<< ", tris=" << Mesh.GetTriangleCount() << ")" << std::endl;

	auto t0 = clock::now();
	GS::OBJFormatData WriteOBJData;
	GS::DenseMeshToOBJFormatData(Mesh, WriteOBJData);
	double convertMs = ms_since(t0);

	auto t1 = clock::now();
	auto Writer = GS::FileTextWriter::OpenFile(filename);
	GS::OBJWriter::WriteOBJ(Writer, WriteOBJData);
	Writer.CloseFile();
	double writeMs = ms_since(t1);

	std::cout << "    convert=" << convertMs << " ms"
		<< ", write=" << writeMs << " ms"
		<< ", total=" << (convertMs + writeMs) << " ms" << std::endl;
}

// Read a .gltf at <SourceDir>/<GLTFFilename>, run mesh-level and data-level
// read/write tests, and dump the outputs into a fresh subfolder of OutputBaseDir
// named "gltf_<stem>" (e.g. "gltf_DamagedHelmet"). Any pre-existing subfolder
// of that name is deleted first.
//
// If bHasEmbedded is true, also runs a glTF-Embedded round-trip on
// <SourceDir>-Embedded/<GLTFFilename> (single self-contained .gltf with
// base64-inlined buffer) and writes it as "<stem>.embed.gltf" in the same
// output folder.
static void TestGLTFReadWrite(
    const std::filesystem::path& SourceDir,
    const std::string& GLTFFilename,
    const std::filesystem::path& OutputBaseDir,
    bool bHasEmbedded)
{
    namespace fs = std::filesystem;

    fs::path SourcePath = SourceDir / GLTFFilename;
    std::string Stem = fs::path(GLTFFilename).stem().string();

    if (!fs::exists(SourcePath))
    {
        std::cout << "=== TestGLTFReadWrite: SKIPPED, source file not found: "
            << SourcePath.string() << std::endl;
        return;
    }

    fs::path OutDir = OutputBaseDir / ("gltf_" + Stem);
    if (fs::exists(OutDir))
        fs::remove_all(OutDir);
    fs::create_directories(OutDir);

    std::string OutPrefix = OutDir.string() + std::string(1, fs::path::preferred_separator);
    std::string SourceStr = SourcePath.string();

    std::cout << "=== TestGLTFReadWrite: " << SourceStr << " ===" << std::endl;

    // mesh-level round-trip: collapse scene to a single DenseMesh
    {
        GS::DenseMesh GLTFMesh;
        bool bGLTFReadOK = GS::GLTFReader::ReadGLTFToDenseMesh(SourceStr, GLTFMesh);
        std::cout << "GLTF Mesh Read ok: " << bGLTFReadOK
            << "  (verts=" << GLTFMesh.GetVertexCount()
            << ", tris=" << GLTFMesh.GetTriangleCount() << ")" << std::endl;

        WriteDenseMeshOBJ(GLTFMesh, OutPrefix + Stem + "_mesh.obj");

        bool bGLTFWriteOK = GS::GLTFWriter::WriteGLTF(OutPrefix + Stem + "_mesh.gltf", GLTFMesh);
        std::cout << "GLTF Mesh Write ok: " << bGLTFWriteOK << std::endl;

        GS::DenseMesh GLTFRoundtripMesh;
        bool bGLTFReadbackOK = GS::GLTFReader::ReadGLTFToDenseMesh(OutPrefix + Stem + "_mesh.gltf", GLTFRoundtripMesh);
        std::cout << "GLTF Mesh Readback ok: " << bGLTFReadbackOK
            << "  (verts=" << GLTFRoundtripMesh.GetVertexCount()
            << ", tris=" << GLTFRoundtripMesh.GetTriangleCount() << ")" << std::endl;
    }

    // data-level round-trip: preserve full GLTFData (materials, textures, scene)
    {
        GS::GLTFFormatData::Root GLTFRoot;
        std::vector<std::vector<uint8_t>> GLTFBuffers;
        bool bGLTFDataReadOK = GS::GLTFReader::ReadGLTF(SourceStr, GLTFRoot, GLTFBuffers);
        std::cout << "GLTF Data Read ok: " << bGLTFDataReadOK
            << "  (meshes=" << GLTFRoot.meshes.size()
            << ", materials=" << GLTFRoot.materials.size()
            << ", textures=" << GLTFRoot.textures.size()
            << ", images=" << GLTFRoot.images.size() << ")" << std::endl;

        // Copy externally-referenced texture image files alongside the output
        // .gltf so they resolve when opened in a viewer. Subdirectories in the
        // URI are preserved; bufferView-embedded and data: URIs are skipped.
        for (const auto& Image : GLTFRoot.images)
        {
            if (!Image.uri.has_value())
                continue;
            const std::string& UriStr = *Image.uri;
            if (UriStr.rfind("data:", 0) == 0)
                continue;
            fs::path SrcImg = SourceDir / UriStr;
            fs::path DstImg = OutDir / UriStr;
            std::error_code EC;
            fs::create_directories(DstImg.parent_path(), EC);
            fs::copy_file(SrcImg, DstImg, fs::copy_options::overwrite_existing, EC);
            if (EC)
                std::cout << "  WARNING: failed to copy texture '" << UriStr << "': " << EC.message() << std::endl;
            else
                std::cout << "  Copied texture: " << UriStr << std::endl;
        }

        bool bGLTFDataWriteOK = GS::GLTFWriter::WriteGLTF(OutPrefix + Stem + ".gltf", GLTFRoot, GLTFBuffers[0]);
        std::cout << "GLTF Data Write ok: " << bGLTFDataWriteOK << std::endl;
    }

    // glTF-Embedded round-trip: single self-contained .gltf with the buffer
    // payload inlined into buffers[0].uri as a base64 data: URI. Image URIs are
    // passed through unchanged. Source dir is <SourceDir>-Embedded.
    if (bHasEmbedded)
    {
        fs::path EmbeddedSourcePath = fs::path(SourceDir.string() + "-Embedded") / GLTFFilename;
        if (!fs::exists(EmbeddedSourcePath))
        {
            std::cout << "  GLTF Embedded: SKIPPED, source file not found: "
                << EmbeddedSourcePath.string() << std::endl;
            return;
        }
        std::string EmbeddedSourceStr = EmbeddedSourcePath.string();

        GS::GLTFFormatData::Root GLTFRoot;
        std::vector<std::vector<uint8_t>> GLTFBuffers;
        bool bGLTFDataReadOK = GS::GLTFReader::ReadGLTF(EmbeddedSourceStr, GLTFRoot, GLTFBuffers);
        std::cout << "GLTF Embedded Data Read ok: " << bGLTFDataReadOK
            << "  (meshes=" << GLTFRoot.meshes.size()
            << ", materials=" << GLTFRoot.materials.size()
            << ", textures=" << GLTFRoot.textures.size()
            << ", images=" << GLTFRoot.images.size()
            << ", buffer0 bytes=" << (GLTFBuffers.empty() ? 0 : (int)GLTFBuffers[0].size())
            << ")" << std::endl;

        std::string OutPath = OutPrefix + Stem + ".embed.gltf";
        bool bGLTFDataWriteOK = GS::GLTFWriter::WriteGLTF(
            OutPath, GLTFRoot,
            GLTFBuffers.empty() ? std::vector<uint8_t>() : GLTFBuffers[0],
            GS::GLTFWriter::BufferLayout::Embedded);
        std::cout << "GLTF Embedded Data Write ok: " << bGLTFDataWriteOK << std::endl;

        GS::GLTFFormatData::Root ReadbackRoot;
        std::vector<std::vector<uint8_t>> ReadbackBuffers;
        bool bReadbackOK = GS::GLTFReader::ReadGLTF(OutPath, ReadbackRoot, ReadbackBuffers);
        std::cout << "GLTF Embedded Data Readback ok: " << bReadbackOK
            << "  (buffer0 bytes=" << (ReadbackBuffers.empty() ? 0 : (int)ReadbackBuffers[0].size())
            << ")" << std::endl;
    }
}

// Read a .glb at <SourceDir>/<GLBFilename>, run mesh-level and data-level
// read/write tests, and dump the outputs into a fresh subfolder of OutputBaseDir
// named "glb_<stem>" (e.g. "glb_DamagedHelmet"). Any pre-existing subfolder
// of that name is deleted first.
static void TestGLBReadWrite(
    const std::filesystem::path& SourceDir,
    const std::string& GLBFilename,
    const std::filesystem::path& OutputBaseDir)
{
    namespace fs = std::filesystem;

    fs::path SourcePath = SourceDir / GLBFilename;
    std::string Stem = fs::path(GLBFilename).stem().string();

    if (!fs::exists(SourcePath))
    {
        std::cout << "=== TestGLBReadWrite: SKIPPED, source file not found: "
            << SourcePath.string() << std::endl;
        return;
    }

    fs::path OutDir = OutputBaseDir / ("glb_" + Stem);
    if (fs::exists(OutDir))
        fs::remove_all(OutDir);
    fs::create_directories(OutDir);

    std::string OutPrefix = OutDir.string() + std::string(1, fs::path::preferred_separator);
    std::string SourceStr = SourcePath.string();

    std::cout << "=== TestGLBReadWrite: " << SourceStr << " ===" << std::endl;

    // mesh-level round-trip: collapse scene to a single DenseMesh
    {
        GS::DenseMesh GLBMesh;
        bool bGLBReadOK = GS::GLBReader::ReadGLBToDenseMesh(SourceStr, GLBMesh);
        std::cout << "GLB Mesh Read ok: " << bGLBReadOK
            << "  (verts=" << GLBMesh.GetVertexCount()
            << ", tris=" << GLBMesh.GetTriangleCount() << ")" << std::endl;

        WriteDenseMeshOBJ(GLBMesh, OutPrefix + Stem + "_mesh.obj");

        bool bGLBWriteOK = GS::GLBWriter::WriteGLB(OutPrefix + Stem + "_mesh.glb", GLBMesh);
        std::cout << "GLB Mesh Write ok: " << bGLBWriteOK << std::endl;

        GS::DenseMesh GLBRoundtripMesh;
        bool bGLBReadbackOK = GS::GLBReader::ReadGLBToDenseMesh(OutPrefix + Stem + "_mesh.glb", GLBRoundtripMesh);
        std::cout << "GLB Mesh Readback ok: " << bGLBReadbackOK
            << "  (verts=" << GLBRoundtripMesh.GetVertexCount()
            << ", tris=" << GLBRoundtripMesh.GetTriangleCount() << ")" << std::endl;
    }

    // data-level round-trip: preserve full GLTFData (materials, textures, scene)
    {
        GS::GLTFFormatData::Root GLBRoot;
        std::vector<std::vector<uint8_t>> GLBBuffers;
        bool bGLBDataReadOK = GS::GLBReader::ReadGLB(SourceStr, GLBRoot, GLBBuffers);
        std::cout << "GLB Data Read ok: " << bGLBDataReadOK
            << "  (meshes=" << GLBRoot.meshes.size()
            << ", materials=" << GLBRoot.materials.size()
            << ", textures=" << GLBRoot.textures.size()
            << ", images=" << GLBRoot.images.size() << ")" << std::endl;

        bool bGLBDataWriteOK = GS::GLBWriter::WriteGLB(OutPrefix + Stem + ".glb", GLBRoot,
            GLBBuffers.empty() ? std::vector<uint8_t>() : GLBBuffers[0]);
        std::cout << "GLB Data Write ok: " << bGLBDataWriteOK << std::endl;
    }
}

// Read an OBJ at <SourceDir>/<OBJFilename>, write it back out, and read the
// result. Outputs land in a fresh subfolder of OutputBaseDir named
// "obj_<stem>" (e.g. "obj_bunny"). Any pre-existing subfolder is deleted first.
static void TestOBJReadWrite(
    const std::filesystem::path& SourceDir,
    const std::string& OBJFilename,
    const std::filesystem::path& OutputBaseDir)
{
    namespace fs = std::filesystem;

    fs::path SourcePath = SourceDir / OBJFilename;
    std::string Stem = fs::path(OBJFilename).stem().string();

    if (!fs::exists(SourcePath))
    {
        std::cout << "=== TestOBJReadWrite: SKIPPED, source file not found: "
            << SourcePath.string() << std::endl;
        return;
    }

    fs::path OutDir = OutputBaseDir / ("obj_" + Stem);
    if (fs::exists(OutDir))
        fs::remove_all(OutDir);
    fs::create_directories(OutDir);

    std::string OutPrefix = OutDir.string() + std::string(1, fs::path::preferred_separator);
    std::string SourceStr = SourcePath.string();

    std::cout << "=== TestOBJReadWrite: " << SourceStr << " ===" << std::endl;

    GS::OBJFormatData OBJData;
    bool bOBJReadOK = GS::OBJReader::ReadOBJ(SourceStr, OBJData);
    std::cout << "OBJ Read ok: " << bOBJReadOK << std::endl;

    std::string OutPath = OutPrefix + Stem + ".obj";
    {
        auto Writer = GS::FileTextWriter::OpenFile(OutPath);
        bool bOBJWriteOK = GS::OBJWriter::WriteOBJ(Writer, OBJData);
        Writer.CloseFile();
        std::cout << "OBJ Write ok: " << bOBJWriteOK << std::endl;
    }

    GS::OBJFormatData OBJReadback;
    bool bOBJReadbackOK = GS::OBJReader::ReadOBJ(OutPath, OBJReadback);
    std::cout << "OBJ Readback ok: " << bOBJReadbackOK << std::endl;
}

// Read a mesh file at <SourceDir>/<MeshFilename> (currently OBJ), write it
// back out as both ASCII and Binary STL, and read each result. Outputs land
// in a fresh subfolder of OutputBaseDir named "stl_<stem>" (e.g. "stl_bunny");
// any pre-existing subfolder is deleted first.
static void TestSTLReadWrite(
    const std::filesystem::path& SourceDir,
    const std::string& MeshFilename,
    const std::filesystem::path& OutputBaseDir)
{
    namespace fs = std::filesystem;

    fs::path SourcePath = SourceDir / MeshFilename;
    std::string Stem = fs::path(MeshFilename).stem().string();

    if (!fs::exists(SourcePath))
    {
        std::cout << "=== TestSTLReadWrite: SKIPPED, source file not found: "
            << SourcePath.string() << std::endl;
        return;
    }

    fs::path OutDir = OutputBaseDir / ("stl_" + Stem);
    if (fs::exists(OutDir))
        fs::remove_all(OutDir);
    fs::create_directories(OutDir);

    std::string OutPrefix = OutDir.string() + std::string(1, fs::path::preferred_separator);
    std::string SourceStr = SourcePath.string();

    std::cout << "=== TestSTLReadWrite: " << SourceStr << " ===" << std::endl;

    GS::OBJFormatData SourceOBJ;
    bool bSourceReadOK = GS::OBJReader::ReadOBJ(SourceStr, SourceOBJ);
    std::cout << "STL Source Read ok: " << bSourceReadOK << std::endl;

    GS::DenseMesh SourceMesh;
    GS::OBJFormatDataToDenseMesh(SourceOBJ, SourceMesh);

    // ASCII STL
    {
        std::string AsciiPath = OutPrefix + Stem + "_ascii.stl";
        bool bWriteOK = GS::STLWriter::WriteSTL(AsciiPath, SourceMesh, Stem, /*bWriteBinary*/ false);
        std::cout << "STL Ascii Write ok: " << bWriteOK << std::endl;

        GS::STLReader::STLMeshData ReadbackData;
        bool bReadbackOK = GS::STLReader::ReadSTL(AsciiPath, ReadbackData);
        std::cout << "STL Ascii Readback ok: " << bReadbackOK
            << "  (tris=" << ReadbackData.Triangles.size() << ")" << std::endl;
    }

    // Binary STL
    {
        std::string BinaryPath = OutPrefix + Stem + "_binary.stl";
        bool bWriteOK = GS::STLWriter::WriteSTL(BinaryPath, SourceMesh, Stem, /*bWriteBinary*/ true);
        std::cout << "STL Binary Write ok: " << bWriteOK << std::endl;

        GS::STLReader::STLMeshData ReadbackData;
        bool bReadbackOK = GS::STLReader::ReadSTL(BinaryPath, ReadbackData);
        std::cout << "STL Binary Readback ok: " << bReadbackOK
            << "  (tris=" << ReadbackData.Triangles.size() << ")" << std::endl;
    }
}

#ifdef ENABLE_GRADIENTSPACE_GRID
static void TestModelGrid()
{
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
#endif

int main()
{
    namespace fs = std::filesystem;

    fs::path outputDir = fs::current_path() / "test_output";
    fs::create_directories(outputDir);

    fs::path testFilesDir("test_files");

    struct GLTFTestCase { const char* SubDir; const char* Filename; bool bHasEmbedded; };
    struct GLBTestCase { const char* SubDir; const char* Filename; };

    // bHasEmbedded: there is an upstream glTF-Embedded sibling at
    // <SubDir>-Embedded with the same Filename (single self-contained .gltf
    // with base64-inlined buffers).
    const GLTFTestCase GLTFCases[] = {
        { "Box/glTF",               "Box.gltf",              true  },
        { "CesiumMilkTruck/glTF",   "CesiumMilkTruck.gltf",  true  },
        { "ChronographWatch/glTF",  "ChronographWatch.gltf", false },
        { "DamagedHelmet/glTF",     "DamagedHelmet.gltf",    true  },
        { "Lantern/glTF",           "Lantern.gltf",          false },
        { "SciFiHelmet/glTF",       "SciFiHelmet.gltf",      false },
    };

    const GLBTestCase GLBCases[] = {
        { "2CylinderEngine/glTF-Binary",  "2CylinderEngine.glb" },
        { "Box/glTF-Binary",              "Box.glb" },
        { "CesiumMilkTruck/glTF-Binary",  "CesiumMilkTruck.glb" },
        { "ChronographWatch/glTF-Binary", "ChronographWatch.glb" },
        { "DamagedHelmet/glTF-Binary",    "DamagedHelmet.glb" },
        { "Lantern/glTF-Binary",          "Lantern.glb" },
    };

    for (const GLTFTestCase& Case : GLTFCases)
        TestGLTFReadWrite(testFilesDir / Case.SubDir, Case.Filename, outputDir, Case.bHasEmbedded);

    for (const GLBTestCase& Case : GLBCases)
        TestGLBReadWrite(testFilesDir / Case.SubDir, Case.Filename, outputDir);

    TestOBJReadWrite(testFilesDir, "bunny.obj", outputDir);
    TestSTLReadWrite(testFilesDir, "bunny.obj", outputDir);

#ifdef ENABLE_GRADIENTSPACE_GRID
    TestModelGrid();
#endif
}

