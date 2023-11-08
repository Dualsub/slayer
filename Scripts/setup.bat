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
python -m virtualenv venv
call venv\Scripts\activate.bat
pip install -r requirements.txt
call buildpack.bat
deactivate
cd ..