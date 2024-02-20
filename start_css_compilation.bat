@echo off
cd src/web_core
npx tailwindcss -i ./src/input.css -o ./build/css/style.css --watch