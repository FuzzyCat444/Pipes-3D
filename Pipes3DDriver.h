#ifndef PIPES3DDRIVER_H
#define PIPES3DDRIVER_H

#include <vector>
#include <cstdlib>

#include <SDL2/SDL.h>

#include <FuzzyRender3D/Renderer.hpp>

class Pipes3DDriver
{
public:
	Pipes3DDriver();
	Pipes3DDriver(int width, int height, int scale);

	void run();
private:
    bool forwardPressed;
    bool backwardPressed;
    bool rightPressed;
    bool leftPressed;
    bool upPressed;
    bool downPressed;
    double xMotion;
    double yMotion;

    void input(SDL_Event* e);
    void update();
    void render();

	int width;
	int height;
	int scale;
	int windowWidth;
	int windowHeight;

	Raster raster;
	Renderer renderer;

	Mesh straightPipeMesh;
	Mesh elbowPipeMesh;
	Raster blankTexture1;
	Raster blankTexture2;
	Raster blankTexture3;
	Raster blankTexture4;
	Camera camera;
	std::vector<LightSource> lights;

	enum class PipeType
	{
	    NONE,

	    STRAIGHT_X, STRAIGHT_Y, STRAIGHT_Z,

	    ELBOW_X_0, ELBOW_X_1, ELBOW_X_2, ELBOW_X_3,
	    ELBOW_Y_0, ELBOW_Y_1, ELBOW_Y_2, ELBOW_Y_3,
	    ELBOW_Z_0, ELBOW_Z_1, ELBOW_Z_2, ELBOW_Z_3
	};

	enum class PipeColor
	{
        COLOR_1, COLOR_2, COLOR_3, COLOR_4, END
	};

	struct Pipe
	{
	    PipeType type;
	    PipeColor color;
	};

	enum class Direction
	{
	    X_MINUS, X_PLUS,
	    Y_MINUS, Y_PLUS,
	    Z_MINUS, Z_PLUS
	};

	struct Coordinates
	{
	    int x, y, z;
	};

	PipeType from2Directions(Direction dir0, Direction dir1);
	Coordinates nextCoordinates(Coordinates coords, Direction dir);
	Coordinates randomCoordinates();
	Direction randomDirection();
	bool areDirectionsOpposite(Direction dir0, Direction dir1);
	void setPipe(Pipe pipe, Coordinates coords);
	Pipe getPipe(Coordinates coords);
	void nextColor();

	Coordinates currentCoords;
	Direction currentDir;
	PipeColor currentCol;
	int pipeCount;
	int maxPipeCount;
	bool done;
	double timer;

    void renderPipe(Pipe pipe, int x, int y, int z);

	std::vector<Pipe> pipes;
	int pipesWidth;
	int pipesHeight;
	int pipesLength;
};

#endif
