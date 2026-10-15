@echo off
echo ========================================================
echo   Compiling SkillSwap - FODS (C Language Engine)
echo ========================================================

gcc -O2 -Wall ^
  backend\student.c ^
  backend\skill.c ^
  backend\linked_list.c ^
  backend\hash_table.c ^
  backend\queue.c ^
  backend\stack.c ^
  backend\graph.c ^
  backend\bst.c ^
  backend\heap.c ^
  backend\trie.c ^
  backend\dynarray.c ^
  backend\sorting.c ^
  backend\matching.c ^
  backend\storage.c ^
  backend\server.c ^
  backend\main.c ^
  -lws2_32 ^
  -o skillswap_server.exe

if %errorlevel% neq 0 (
    echo [ERROR] Server compilation failed.
    exit /b %errorlevel%
)

gcc -O2 -Wall ^
  backend\student.c ^
  backend\skill.c ^
  backend\linked_list.c ^
  backend\hash_table.c ^
  backend\queue.c ^
  backend\stack.c ^
  backend\graph.c ^
  backend\bst.c ^
  backend\heap.c ^
  backend\trie.c ^
  backend\dynarray.c ^
  backend\sorting.c ^
  backend\matching.c ^
  backend\storage.c ^
  cli\skillswap_cli.c ^
  -o cli\skillswap_cli.exe

gcc -O2 -Wall ^
  backend\student.c ^
  backend\skill.c ^
  backend\linked_list.c ^
  backend\hash_table.c ^
  backend\queue.c ^
  backend\stack.c ^
  backend\graph.c ^
  backend\bst.c ^
  backend\heap.c ^
  backend\trie.c ^
  backend\dynarray.c ^
  backend\sorting.c ^
  backend\matching.c ^
  backend\storage.c ^
  tests\test_data_structures.c ^
  -o tests\test_data_structures.exe

echo.
echo [SUCCESS] All executables built successfully:
echo   1. skillswap_server.exe (C Native HTTP Server on port 8080)
echo   2. cli\skillswap_cli.exe (Interactive Academic Viva CLI)
echo   3. tests\test_data_structures.exe (Automated Unit Test Suite)
echo.
