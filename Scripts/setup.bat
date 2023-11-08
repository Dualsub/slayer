@REM Initial tools setup
echo Initial tools setup
cd Tools
python -m virtualenv .venv
call .venv\Scripts\activate.bat
call pip install -r requirements.txt
call deactivate
cd ..

@REM Building the project
echo Building the project
mkdir build
cd build
call cmake ..
call cmake --build . --config RelWithDebInfo
cd ..

@REM Building asset pack
echo Building asset pack
cd Tools
call .venv\Scripts\activate.bat
call buildpack.bat
call deactivate
cd ..