FreeType — статическая сборка (всё в product.dll, инжект без лишних DLL)

1. Один раз выполни скрипт сборки статической библиотеки:
   Открой папку   product\backend\imgui\freetype\
   Запусти        build_static.bat

   Нужны: CMake, Visual Studio 2019/2022 (x64).
   Если CMake выдаст ошибку про генератор — в build_static.bat замени
   "Visual Studio 17 2022" на "Visual Studio 16 2019".

2. После успешного выполнения появятся:
   out_static\Release\freetype.lib
   out_static\Debug\freetype.lib

3. Собери проект product (Release x64). Проект уже настроен на эти .lib.

4. В папке x64\Release должна быть только product.dll (без freetype.dll, zlib1.dll и т.д.).
   Если раньше использовался vcpkg и он копировал DLL — отключи интеграцию vcpkg
   для этого проекта или удали лишние DLL из x64\Release вручную перед инжектом.

5. Инжектируй только product.dll — инжект должен проходить.
