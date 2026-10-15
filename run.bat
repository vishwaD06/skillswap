@echo off
echo Starting SkillSwap Native C Server on port 8080...
start "" "http://localhost:8080"
skillswap_server.exe --port 8080 --dir frontend
