function vs ($arch)
{
        push-location 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build'
        cmd /c "set VSCMD_START_DIR=%CD% && vcvarsall.bat $arch&set" |
        foreach-object {
                if ($_ -match "=") {
                        $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
                }
        }
        pop-location
}

new-item -path build >$null 2>&1 ; set-location -path build

vs amd64
$env:Path += ";C:/Qt/Qt5.9.2/5.9.2/msvc2017_64/bin"

cmake  -GNinja `
       -DCMAKE_BUILD_TYPE=RelWithDebInfo `
       -DCMAKE_C_COMPILER=clang-cl `
       -DCMAKE_CXX_COMPILER=clang-cl `
       -DVCPKG_TARGET_TRIPLET=x64-windows-static `
       -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake `
       -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
        ..
ninja
