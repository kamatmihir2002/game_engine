all:
	cp ./lib/glfw3.dll ./bin
	cp ./lib/lua51.dll ./bin
	g++ -g -Iinclude ./src/*.cpp ./src/*.c ./lib/libglfw3dll.a ./lib/liblua.a -Llib -lglfw3 -lopengl32  -o ./bin/app_2D.exe