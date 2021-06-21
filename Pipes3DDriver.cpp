#include "Pipes3DDriver.h"

Pipes3DDriver::Pipes3DDriver()
    : Pipes3DDriver{ 100, 100, 1 }
{
}

Pipes3DDriver::Pipes3DDriver(int width, int height, int scale)
    : width{ width }, height{ height }, scale{ scale },
      windowWidth{ scale * width }, windowHeight{ scale * height },
      raster{ width, height }, renderer{ &raster },blankTexture1{ 1, 1, Color{ 0, 184, 169, 255 } },
      blankTexture2{ 1, 1, Color{ 248, 243, 212, 255 } },
      blankTexture3{ 1, 1, Color{ 246, 65, 108, 255 } },
      blankTexture4{ 1, 1, Color{ 255, 222, 125, 255 } },
      camera{ radians(90.0), width / (double) height, 0.1, Vector3{ 0.0, 0.0, 45.0 }, 0.0, 0.0 },
      pipesWidth{ 25 }, pipesHeight{ 25 }, pipesLength{ 40 }
{
    forwardPressed = false;
    backwardPressed = false;
    rightPressed = false;
    leftPressed = false;
    upPressed = false;
    downPressed = false;
    xMotion = 0.0;
    yMotion = 0.0;

    lights.push_back(LightSource{ AmbientLight{ Vector3{ 0.2, 0.2, 0.2 } } });
    lights.push_back(LightSource{ DirectionalLight{ Vector3{ 1.0, 1.0, 1.0 }, Vector3{ -1.0, -1.0, -1.0 } } });

    straightPipeMesh = Mesh::loadFromFile("pipe_straight.obj", Mesh::Shading::KEEP_NORMALS);
    elbowPipeMesh = Mesh::loadFromFile("pipe_elbow.obj", Mesh::Shading::KEEP_NORMALS);

    pipes.resize(pipesWidth * pipesHeight * pipesLength);
    for (int i = 0; i < pipes.size(); i++)
    {
        Pipe p;
        p.type = PipeType::NONE;
        pipes.at(i) = p;
    }
    currentCoords = randomCoordinates();
    currentDir = randomDirection();
    currentCol = PipeColor::COLOR_1;
    pipeCount = 0;
    maxPipeCount = 500;
    done = false;
    timer = 0.0;
}

void Pipes3DDriver::input(SDL_Event* e)
{
    if (e->type == SDL_KEYDOWN)
    {
        if (e->key.keysym.sym == SDLK_w)
            forwardPressed = true;
        if (e->key.keysym.sym == SDLK_d)
            rightPressed = true;
        if (e->key.keysym.sym == SDLK_e)
            upPressed = true;
        if (e->key.keysym.sym == SDLK_s)
            backwardPressed = true;
        if (e->key.keysym.sym == SDLK_a)
            leftPressed = true;
        if (e->key.keysym.sym == SDLK_q)
            downPressed = true;
    }
    else if (e->type == SDL_KEYUP)
    {
        if (e->key.keysym.sym == SDLK_w)
            forwardPressed = false;
        if (e->key.keysym.sym == SDLK_d)
            rightPressed = false;
        if (e->key.keysym.sym == SDLK_e)
            upPressed = false;
        if (e->key.keysym.sym == SDLK_s)
            backwardPressed = false;
        if (e->key.keysym.sym == SDLK_a)
            leftPressed = false;
        if (e->key.keysym.sym == SDLK_q)
            downPressed = false;
    }
    else if (e->type == SDL_MOUSEMOTION)
    {
        xMotion = e->motion.xrel;
        yMotion = e->motion.yrel;
    }
}

void Pipes3DDriver::update()
{
    double speed = 1.0146;
    Vector3 frontVec = camera.getFrontVec();
    frontVec.scl(speed);
    Vector3 rightVec = camera.getRightVec();
    rightVec.scl(speed);
    Vector3 upVec = Vector3{ 0.0, 1.0, 0.0 };
    upVec.scl(speed);
    Vector3 backVec{ frontVec };
    backVec.scl(-1.0);
    Vector3 leftVec{ rightVec };
    leftVec.scl(-1.0);
    Vector3 downVec{ upVec };
    downVec.scl(-1.0);

    if (forwardPressed)
        camera.translate(frontVec);
    if (rightPressed)
        camera.translate(rightVec);
    if (upPressed)
        camera.translate(upVec);
    if (backwardPressed)
        camera.translate(backVec);
    if (leftPressed)
        camera.translate(leftVec);
    if (downPressed)
        camera.translate(downVec);

    double rotSpeed = 0.015;
    camera.rotateYaw(-xMotion * rotSpeed);
    camera.rotatePitch(-yMotion * rotSpeed);
    xMotion = 0.0;
    yMotion = 0.0;

    int times = 0;
    while (timer >= 1 / 30.0)
    {
        times++;
        timer -= 1 / 30.0;
    }
    while (!done && times > 0)
    {
        times--;
        Pipe pipe;
        pipe.color = currentCol;
        int weights[6] =
        {
            1, 1, 1, 1, 1, 1
        };
        weights[static_cast<int>(currentDir)] = 20;
        for (int i = 0; i < 6; i++)
        {
            Direction dir = static_cast<Direction>(i);
            Coordinates next = nextCoordinates(currentCoords, dir);
            if (getPipe(next).type != PipeType::NONE || areDirectionsOpposite(dir, currentDir))
            {
                weights[i] = 0;
            }
        }
        int totalWeight = 0;
        for (int i = 0; i < 6; i++)
            totalWeight += weights[i];
        for (int i = 0; i < 6; i++)
        {
            Direction dir = static_cast<Direction>(i);
            Coordinates next = nextCoordinates(currentCoords, dir);
            int weight = weights[i];
            if (totalWeight == 0)
            {
                nextColor();
                break;
            }
            int random = rand() % totalWeight;
            if (random < weight)
            {
                Pipe pipe;
                pipe.type = from2Directions(currentDir, dir);
                pipe.color = currentCol;
                setPipe(pipe, currentCoords);
                pipeCount++;
                if (pipeCount > maxPipeCount)
                {
                    nextColor();
                    break;
                }
                currentCoords = next;
                currentDir = dir;
                break;
            }
            else
            {
                totalWeight -= weight;
            }
        }
    }
    timer += 1 / 60.0;
}

void Pipes3DDriver::render()
{
    int offsetX = pipesWidth / 2;
    int offsetY = pipesHeight / 2;
    int offsetZ = pipesLength / 2;
    Coordinates coords;
    for (coords.z = 0; coords.z < pipesLength; coords.z++)
    {
        for (coords.y = 0; coords.y < pipesHeight; coords.y++)
        {
            for (coords.x = 0; coords.x < pipesWidth; coords.x++)
            {
                Pipe pipe = getPipe(coords);
                renderPipe(pipe, coords.x - offsetX, coords.y - offsetY, coords.z - offsetZ);
            }
        }
    }
}

Pipes3DDriver::PipeType Pipes3DDriver::from2Directions(Direction dir0, Direction dir1)
{
    if (dir0 == Direction::Z_PLUS && dir1 == Direction::Y_MINUS || dir0 == Direction::Y_PLUS && dir1 == Direction::Z_MINUS)
        return PipeType::ELBOW_X_0;
    else if (dir0 == Direction::Z_PLUS && dir1 == Direction::Y_PLUS || dir0 == Direction::Y_MINUS && dir1 == Direction::Z_MINUS)
        return PipeType::ELBOW_X_1;
    else if (dir1 == Direction::Z_PLUS && dir0 == Direction::Y_MINUS || dir1 == Direction::Y_PLUS && dir0 == Direction::Z_MINUS)
        return PipeType::ELBOW_X_2;
    else if (dir1 == Direction::Z_PLUS && dir0 == Direction::Y_PLUS || dir1 == Direction::Y_MINUS && dir0 == Direction::Z_MINUS)
        return PipeType::ELBOW_X_3;
    else if (dir0 == Direction::X_PLUS && dir1 == Direction::Z_MINUS || dir0 == Direction::Z_PLUS && dir1 == Direction::X_MINUS)
        return PipeType::ELBOW_Y_0;
    else if (dir0 == Direction::X_PLUS && dir1 == Direction::Z_PLUS || dir0 == Direction::Z_MINUS && dir1 == Direction::X_MINUS)
        return PipeType::ELBOW_Y_1;
    else if (dir1 == Direction::X_PLUS && dir0 == Direction::Z_MINUS || dir1 == Direction::Z_PLUS && dir0 == Direction::X_MINUS)
        return PipeType::ELBOW_Y_2;
    else if (dir1 == Direction::X_PLUS && dir0 == Direction::Z_PLUS || dir1 == Direction::Z_MINUS && dir0 == Direction::X_MINUS)
        return PipeType::ELBOW_Y_3;
    else if (dir0 == Direction::Y_PLUS && dir1 == Direction::X_MINUS || dir0 == Direction::X_PLUS && dir1 == Direction::Y_MINUS)
        return PipeType::ELBOW_Z_0;
    else if (dir0 == Direction::Y_PLUS && dir1 == Direction::X_PLUS || dir0 == Direction::X_MINUS && dir1 == Direction::Y_MINUS)
        return PipeType::ELBOW_Z_1;
    else if (dir1 == Direction::Y_PLUS && dir0 == Direction::X_MINUS || dir1 == Direction::X_PLUS && dir0 == Direction::Y_MINUS)
        return PipeType::ELBOW_Z_2;
    else if (dir1 == Direction::Y_PLUS && dir0 == Direction::X_PLUS || dir1 == Direction::X_MINUS && dir0 == Direction::Y_MINUS)
        return PipeType::ELBOW_Z_3;
    else if (dir0 == dir1)
    {
        if (dir0 == Direction::X_MINUS || dir0 == Direction::X_PLUS)
            return PipeType::STRAIGHT_X;
        else  if (dir0 == Direction::Y_MINUS || dir0 == Direction::Y_PLUS)
            return PipeType::STRAIGHT_Y;
        else if (dir0 == Direction::Z_MINUS || dir0 == Direction::Z_PLUS)
            return PipeType::STRAIGHT_Z;
    }
}

Pipes3DDriver::Coordinates Pipes3DDriver::nextCoordinates(Coordinates coords, Direction dir)
{
    switch (dir)
    {
    case Direction::X_MINUS:
        coords.x--;
        break;
    case Direction::X_PLUS:
        coords.x++;
        break;
    case Direction::Y_MINUS:
        coords.y--;
        break;
    case Direction::Y_PLUS:
        coords.y++;
        break;
    case Direction::Z_MINUS:
        coords.z--;
        break;
    case Direction::Z_PLUS:
        coords.z++;
        break;
    }
    return coords;
}

Pipes3DDriver::Coordinates Pipes3DDriver::randomCoordinates()
{
    Coordinates coords;
    coords.x = rand() % pipesWidth;
    coords.y = rand() % pipesHeight;
    coords.z = rand() % pipesLength;
    return coords;
}

void Pipes3DDriver::setPipe(Pipe pipe, Coordinates coords)
{
    int x = coords.x;
    int y = coords.y;
    int z = coords.z;
    if (x >= 0 && x < pipesWidth &&
        y >= 0 && y < pipesHeight &&
        z >= 0 && z < pipesLength)
    {
        pipes.at(x + y * pipesWidth + z * pipesWidth * pipesHeight) = pipe;
    }
}

Pipes3DDriver::Pipe Pipes3DDriver::getPipe(Coordinates coords)
{
    if (coords.x >= 0 && coords.x < pipesWidth &&
        coords.y >= 0 && coords.y < pipesHeight &&
        coords.z >= 0 && coords.z < pipesLength)
    {
        return pipes.at(coords.x + coords.y * pipesWidth + coords.z * pipesWidth * pipesHeight);
    }
    Pipe pipe;
    pipe.type = PipeType::STRAIGHT_X;
    return pipe;
}

Pipes3DDriver::Direction Pipes3DDriver::randomDirection()
{
    Direction dir;
    dir = static_cast<Direction>(rand() % 6);
    return dir;
}

bool Pipes3DDriver::areDirectionsOpposite(Direction dir0, Direction dir1)
{
    return dir0 == Direction::X_MINUS && dir1 == Direction::X_PLUS ||
           dir0 == Direction::X_PLUS && dir1 == Direction::X_MINUS ||
           dir0 == Direction::Y_MINUS && dir1 == Direction::Y_PLUS ||
           dir0 == Direction::Y_PLUS && dir1 == Direction::Y_MINUS ||
           dir0 == Direction::Z_MINUS && dir1 == Direction::Z_PLUS ||
           dir0 == Direction::Z_PLUS && dir1 == Direction::Z_MINUS;
}

void Pipes3DDriver::nextColor()
{
    currentCol = static_cast<PipeColor>((static_cast<int>(currentCol) + 1));
    if (currentCol == PipeColor::END)
        done = true;
    pipeCount = 0;
    Coordinates coords = randomCoordinates();
    while (getPipe(coords).type != PipeType::NONE)
        coords = randomCoordinates();
    currentCoords = coords;
    currentDir = randomDirection();
}

void Pipes3DDriver::renderPipe(Pipe pipe, int x, int y, int z)
{
    if (pipe.type == PipeType::NONE)
        return;

    Rotate r0;
    Rotate r1;
    Translate t{ Vector3{ x * 2.0, y * 2.0, z * 2.0 } };
    Combined c{ &r0, &r1, &t };
    bool straight = false;

    switch (pipe.type)
    {
    case PipeType::STRAIGHT_X:
        r1 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        straight = true;
        break;
    case PipeType::STRAIGHT_Y:
        straight = true;
        break;
    case PipeType::STRAIGHT_Z:
        r1 = Rotate{ Rotate::Axis::X, radians(90.0) };
        straight = true;
        break;
    case PipeType::ELBOW_X_0:
        r1 = Rotate{ Rotate::Axis::X, radians(90.0) };
        break;
    case PipeType::ELBOW_X_1:
        r1 = Rotate{ Rotate::Axis::X, radians(180.0) };
        break;
    case PipeType::ELBOW_X_2:
        r1 = Rotate{ Rotate::Axis::X, radians(270.0) };
        break;
    case PipeType::ELBOW_X_3:
        r1 = Rotate{ Rotate::Axis::X, radians(360.0) };
        break;
    case PipeType::ELBOW_Y_0:
        r0 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        r1 = Rotate{ Rotate::Axis::Y, radians(180.0) };
        break;
    case PipeType::ELBOW_Y_1:
        r0 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        r1 = Rotate{ Rotate::Axis::Y, radians(270.0) };
        break;
    case PipeType::ELBOW_Y_2:
        r0 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        r1 = Rotate{ Rotate::Axis::Y, radians(360.0) };
        break;
    case PipeType::ELBOW_Y_3:
        r0 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        r1 = Rotate{ Rotate::Axis::Y, radians(450.0) };
        break;
    case PipeType::ELBOW_Z_0:
        r0 = Rotate{ Rotate::Axis::Y, radians(270.0) };
        r1 = Rotate{ Rotate::Axis::Z, radians(0.0) };
        break;
    case PipeType::ELBOW_Z_1:
        r0 = Rotate{ Rotate::Axis::Y, radians(270.0) };
        r1 = Rotate{ Rotate::Axis::Z, radians(90.0) };
        break;
    case PipeType::ELBOW_Z_2:
        r0 = Rotate{ Rotate::Axis::Y, radians(270.0) };
        r1 = Rotate{ Rotate::Axis::Z, radians(180.0) };
        break;
    case PipeType::ELBOW_Z_3:
        r0 = Rotate{ Rotate::Axis::Y, radians(270.0) };
        r1 = Rotate{ Rotate::Axis::Z, radians(270.0) };
        break;
    }

    Raster* theRaster = &blankTexture1;
    if (pipe.color == PipeColor::COLOR_2)
        theRaster = &blankTexture2;
    else if (pipe.color == PipeColor::COLOR_3)
        theRaster = &blankTexture3;
    else if (pipe.color == PipeColor::COLOR_4)
        theRaster = &blankTexture4;

    if (straight)
        renderer.renderMesh(straightPipeMesh, *theRaster, c, camera, lights, Renderer::Lighting::DIFFUSE);
    else
        renderer.renderMesh(elbowPipeMesh, *theRaster, c, camera, lights, Renderer::Lighting::DIFFUSE);
}

void Pipes3DDriver::run()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow("Pipes Screensaver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture* screenTexture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    bool running = true;
    Uint32 oldTime = SDL_GetTicks();
    Uint32 delta = 0;
    while (running)
    {
        Uint32 now = SDL_GetTicks();
        delta += now - oldTime;
        oldTime = now;

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                    running = false;
            }
            input(&e);
        }

        while (delta >= 16)
        {
            update();
            delta -= 16;
        }

        renderer.clearColorDepth(Color{ 0, 0, 0, 255 });
        render();

        uint8_t* pixels = nullptr;
        int pitch;
        SDL_LockTexture(screenTexture, nullptr, (void**)&pixels, &pitch);
        int indexPixels = 0;
        int indexRaster = 0;
        for (int y = 0; y < height; y++)
        {
            int ip = indexPixels;
            for (int x = 0; x < width; x++)
            {
                Color c = raster.getPixel(indexRaster);
                pixels[ip] = c.r;
                pixels[ip + 1] = c.g;
                pixels[ip + 2] = c.b;
                pixels[ip + 3] = c.a;
                ip += 4;
                indexRaster += 4;
            }
            indexPixels += pitch;
        }
        SDL_UnlockTexture(screenTexture);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, screenTexture, nullptr, nullptr);
        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    SDL_Quit();
}
