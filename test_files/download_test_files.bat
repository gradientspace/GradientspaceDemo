@echo off
setlocal enabledelayedexpansion

REM Downloads the glTF / GLB sample models referenced by the demo into this folder.
REM Output path is anchored to the script's own location (%~dp0), so it works
REM regardless of the caller's current directory.
REM
REM Most models come from the current Khronos sample repo (glTF-Sample-Assets);
REM 2CylinderEngine lives only in the legacy repo (glTF-Sample-Models).
REM Requires curl.exe (shipped with Windows 10 1803+).

set "ASSETS=https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models"
set "MODELS=https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/main/2.0"

REM %~dp0 = directory containing this .bat (always ends with a backslash)
set "OUT=%~dp0"
REM strip the trailing backslash so %OUT%\... is well-formed
if "%OUT:~-1%"=="\" set "OUT=%OUT:~0,-1%"

REM 2CylinderEngine -- legacy repo only
call :get %MODELS% 2CylinderEngine glTF-Binary 2CylinderEngine.glb
call :get %MODELS% 2CylinderEngine glTF-Embedded 2CylinderEngine.gltf

REM Box
call :get %ASSETS% Box glTF Box.gltf
call :get %ASSETS% Box glTF Box0.bin
call :get %ASSETS% Box glTF-Binary Box.glb
call :get %ASSETS% Box glTF-Embedded Box.gltf

REM CesiumMilkTruck
call :get %ASSETS% CesiumMilkTruck glTF CesiumMilkTruck.gltf
call :get %ASSETS% CesiumMilkTruck glTF CesiumMilkTruck_data.bin
call :get %ASSETS% CesiumMilkTruck glTF CesiumMilkTruck.jpg
call :get %ASSETS% CesiumMilkTruck glTF-Binary CesiumMilkTruck.glb
call :get %ASSETS% CesiumMilkTruck glTF-Embedded CesiumMilkTruck.gltf

REM ChronographWatch
call :get %ASSETS% ChronographWatch glTF ChronographWatch.gltf
call :get %ASSETS% ChronographWatch glTF ChronographWatch.data.bin
call :get %ASSETS% ChronographWatch glTF band_occlusion.png
call :get %ASSETS% ChronographWatch glTF carbonfiber_normal.png
call :get %ASSETS% ChronographWatch glTF dgg_basecolor.png
call :get %ASSETS% ChronographWatch glTF dgg_rm.png
call :get %ASSETS% ChronographWatch glTF khronos_basecolor.png
call :get %ASSETS% ChronographWatch glTF khronos_rm.png
call :get %ASSETS% ChronographWatch glTF watchface_basecolor.png
call :get %ASSETS% ChronographWatch glTF watchface_orm.png
call :get %ASSETS% ChronographWatch glTF-Binary ChronographWatch.glb

REM DamagedHelmet
call :get %ASSETS% DamagedHelmet glTF DamagedHelmet.gltf
call :get %ASSETS% DamagedHelmet glTF DamagedHelmet.bin
call :get %ASSETS% DamagedHelmet glTF Default_AO.jpg
call :get %ASSETS% DamagedHelmet glTF Default_albedo.jpg
call :get %ASSETS% DamagedHelmet glTF Default_emissive.jpg
call :get %ASSETS% DamagedHelmet glTF Default_metalRoughness.jpg
call :get %ASSETS% DamagedHelmet glTF Default_normal.jpg
call :get %ASSETS% DamagedHelmet glTF-Binary DamagedHelmet.glb
call :get %ASSETS% DamagedHelmet glTF-Embedded DamagedHelmet.gltf

REM Lantern
call :get %ASSETS% Lantern glTF Lantern.gltf
call :get %ASSETS% Lantern glTF Lantern.bin
call :get %ASSETS% Lantern glTF Lantern_baseColor.png
call :get %ASSETS% Lantern glTF Lantern_emissive.png
call :get %ASSETS% Lantern glTF Lantern_normal.png
call :get %ASSETS% Lantern glTF Lantern_roughnessMetallic.png
call :get %ASSETS% Lantern glTF-Binary Lantern.glb

REM SciFiHelmet (no glTF-Binary variant)
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet.gltf
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet.bin
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet_AmbientOcclusion.png
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet_BaseColor.png
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet_MetallicRoughness.png
call :get %ASSETS% SciFiHelmet glTF SciFiHelmet_Normal.png

echo.
echo Done.
exit /b 0

:get
REM %1 = base URL, %2 = model, %3 = variant subfolder, %4 = filename
set "BASE=%~1"
set "MODEL=%~2"
set "VARIANT=%~3"
set "FILE=%~4"
set "DEST_DIR=%OUT%\%MODEL%\%VARIANT%"
set "DEST=%DEST_DIR%\%FILE%"
if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"
if exist "%DEST%" (
    echo [skip] %MODEL%/%VARIANT%/%FILE%
    exit /b 0
)
echo [get ] %MODEL%/%VARIANT%/%FILE%
curl -fsSL -o "%DEST%" "%BASE%/%MODEL%/%VARIANT%/%FILE%"
if errorlevel 1 echo   FAILED: %BASE%/%MODEL%/%VARIANT%/%FILE%
exit /b 0
