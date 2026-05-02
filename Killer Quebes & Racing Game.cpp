
// Name : Sajana pamuditha
// Uclan ID : G21328035


#include <TL-Engine.h>	// TL-Engine include file and namespace


using namespace tle;
using namespace std;

// Constants
const float kPi = 3.14159f;
const float kMaxSpeed = 55.56f;
const float kMaxReverseSpeed = 27.78f;
const float kAcceleration = 50.0f;
const float kDeceleration = 30.0f;
const float kTurnSpeed = 75.0f;
const float kHealthDecrease = 0.1f;
const float kLowHealthThreshold = 25.0f;
const float kCameraSpeed = 50.0f;
const float kScale = 1.0f;
const float kCheckpointRadius = 10.0f;
const float kCarRadius = 1.0f;
const float kBoxWidth = 10.0f;
const float kBoxDepth = 10.0f;
const float kInitialCarX = 0.0f;
const float kInitialCarY = 0.0f;
const float kInitialCarZ = -20.0f;
const float kSkyboxY = -840.0f;
const float kCameraInitialX = 0.0f;
const float kCameraInitialY = 10.0f;
const float kCameraInitialZ = -30.0f;
const float kLegCollisionRadius = 1.5f;
const float kCheckpointLegWidth = 1.0f;
const float kCheckpointLegHeight = 5.0f;

const int kNumCheckpoints = 4;
const int kNumIsles = 6;
const int kNumWalls = 4;
const int kNumTanks = 16;
const int kNumTribune = 2;
const int kNumCheckpointLegs = 8;

const float kBounceEffect = -0.5f;
const float kCollisionCooldownTime = 1.0f;
const float kCollisionMoveOffset = 0.1f;
const float kCollisionMoveMultiplier = 5.0f;
const float kCameraRotationSpeed = 50.0f;
const float kCameraRotationLimit = 80.0f;
const float kCheckpointSpeedThreshold = 0.16f;
const float kMouseRotationSpeed = 50.0f;
const float kFirstPersonCameraX = 0.0f;
const float kFirstPersonCameraY = 2.0f;
const float kFirstPersonCameraZ = 0.0f;
const float kFirstPersonCameraRotation = 180.0f;

// Game states enumeration
enum GameState
{
    StartMenu,
    Countdown,
    Racing,
    Finished
};

// 2D Vector structure
struct Vector2D
{
    float x;
    float z;

    // Constructor to initialize the vector
    Vector2D(float x = 0.0f, float z = 0.0f) : x(x), z(z) {}

    // Overload the + operator for vector addition
    Vector2D operator+(const Vector2D& other) const
    {
        return Vector2D(x + other.x, z + other.z);
    }

    // Overload the * operator for scalar multiplication
    Vector2D operator*(float scalar) const
    {
        return Vector2D(x * scalar, z * scalar);
    }
};

// Checkpoint structure
struct Checkpoint
{
    IModel* model;
    bool passed;
    int index; // Index to track the order of checkpoints
};

// Tank structure
struct Tank
{
    IModel* model;
    bool hit;
};

// Checkpoint leg structure
struct CheckpointLeg
{
    IModel* checkpoint;
    float offsetX;
    float offsetZ;
};

// Function prototypes
bool SphereToSphereCollision(IModel* car, Checkpoint& checkpoint, float speed, int currentCheckpoint); // Check collision with checkpoint
bool SphereBoxCollision(IModel* car, IModel* box); // Check collision between sphere and box
Vector2D GetThrustVector(IModel* car, float thrust); // Get thrust vector for the car
Vector2D GetDragVector(const Vector2D& momentum, float frameTime); // Get drag vector for the car
float Clamp(float value, float min, float max); // Clamp a value between min and max
float Distance(float x1, float z1, float x2, float z2); // Calculate distance between two points
bool CircleCollision(float x1, float z1, float r1, float x2, float z2, float r2); // Check collision between two circles
bool CheckLegCollision(IModel* car, CheckpointLeg legs[], int numLegs); // Check collision with checkpoint legs

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
    myEngine->AddMediaFolder("./media");
    myEngine->AddMediaFolder("./media1");


	/**** Set up your scene here ****/
   // Create UI backdrop
    ISprite* backdrop = myEngine->CreateSprite("ui_backdrop.jpg", 0, 650);

    // Load meshes
    IMesh* carMesh = myEngine->LoadMesh("race2.x");
    IMesh* checkpointMesh = myEngine->LoadMesh("Checkpoint.x");
    IMesh* isleMesh = myEngine->LoadMesh("IsleStraight.x");
    IMesh* wallMesh = myEngine->LoadMesh("Wall.x");
    IMesh* skyboxMesh = myEngine->LoadMesh("Skybox 07.x");
    IMesh* floorMesh = myEngine->LoadMesh("ground.x");
    IMesh* tankMesh = myEngine->LoadMesh("TankSmall1.x");
    IMesh* tribuneMesh = myEngine->LoadMesh("Tribune1.x");

    // Create models
    IModel* car = carMesh->CreateModel(kInitialCarX, kInitialCarY, kInitialCarZ);
    IModel* skybox = skyboxMesh->CreateModel(0, kSkyboxY, 0);
    IModel* floor = floorMesh->CreateModel(0.0f, 0.0f, 0.0f);
    IModel* tribune[kNumTribune];

    // Initialize tribunes
    tribune[0] = tribuneMesh->CreateModel(60.0f, 0.0f, 50.0f);
    tribune[1] = tribuneMesh->CreateModel(-60.0f, 0.0f, 60.0f);

    // Initialize checkpoints with indices
    Checkpoint checkpoints[kNumCheckpoints] = {
        { checkpointMesh->CreateModel(0, 0, 0), false, 0 },
        { checkpointMesh->CreateModel(10, 0, 120), false, 1 },
        { checkpointMesh->CreateModel(40, 0, 90), false, 2 },
        { checkpointMesh->CreateModel(25, 0, 56), false, 3 }
    };

    // Rotate the second checkpoint's model
    checkpoints[1].model->RotateY(90);

    // Initialize isles
    IModel* isles[kNumIsles] = {
        isleMesh->CreateModel(-10, 0, 40),
        isleMesh->CreateModel(10, 0, 40),
        isleMesh->CreateModel(-10, 0, 56),
        isleMesh->CreateModel(10, 0, 56),
        isleMesh->CreateModel(-10, 0, 72),
        isleMesh->CreateModel(10, 0, 72)
    };

    // Initialize walls
    IModel* walls[kNumWalls] = {
        wallMesh->CreateModel(-10, 0, 48),
        wallMesh->CreateModel(10, 0, 48),
        wallMesh->CreateModel(-10, 0, 64),
        wallMesh->CreateModel(10, 0, 64)
    };

    // Initialize tanks
    Tank cornerTanks[kNumTanks] = {
        { tankMesh->CreateModel(5, 0, 30), false },
        { tankMesh->CreateModel(15, 0, 70), false },
        { tankMesh->CreateModel(20, 0, 110), false },
        { tankMesh->CreateModel(15, 0, 60), false },
        { tankMesh->CreateModel(-15, 0, 30), false },
        { tankMesh->CreateModel(0, -2, 100), false },
        { tankMesh->CreateModel(10, -14, 24), false },
        { tankMesh->CreateModel(-12, 0, 20), false },
        { tankMesh->CreateModel(-6, -0, 130), false },
        { tankMesh->CreateModel(-20, 0, 120), false },
        { tankMesh->CreateModel(30, -0, 130), false },
        { tankMesh->CreateModel(44, 0, 120), false },
        { tankMesh->CreateModel(53, 0, 100), false },
        { tankMesh->CreateModel(60, 0, 90), false },
        { tankMesh->CreateModel(58, -0, 80), false },
        { tankMesh->CreateModel(50, -0, 70), true },

    };

    // Initialize checkpoint legs
    CheckpointLeg checkpointLegs[kNumCheckpointLegs] = {
        { checkpoints[0].model, -8.5f, 0.0f },
        { checkpoints[0].model, 8.5f, 0.0f },
        { checkpoints[1].model, 0.0f, -8.5f },
        { checkpoints[1].model, 0.0f, 8.5f },
        { checkpoints[2].model, -8.5f, 0.0f },
        { checkpoints[2].model, 8.5f, 0.0f },
        { checkpoints[3].model, -8.5f, 0.0f },
        { checkpoints[3].model, 8.5f, 0.0f }
    };

  

    // Initialize game variables
    Vector2D momentum(0.0f, 0.0f);
    float carHealth = 100.0f; 
    int currentCheckpoint = 0; 
    GameState gameState = StartMenu; 
    float countdownTimer = 0.0f;
    float collisionCooldown = 0.0f;

    // Load font
    IFont* font = myEngine->LoadFont("Comic Sans MS", 36);
    string gameStateMessage = "Hit Space to Start";

    // Initialize camera
    ICamera* camera = myEngine->CreateCamera(kManual, kCameraInitialX, kCameraInitialY, kCameraInitialZ);
    camera->AttachToParent(car); 
    bool isFirstPersonCam = false; 

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
        float frameTime = myEngine->Timer(); 

		/**** Update your scene each frame here ****/

        // Exit the game if Escape key is pressed
        if (myEngine->KeyHit(Key_Escape))
        {
            myEngine->Stop();
        }

        switch (gameState)
        {
        case StartMenu:
            // Start the countdown if Space key is pressed
            if (myEngine->KeyHit(Key_Space))
            {
                gameState = Countdown;
                countdownTimer = 3.0f;
            }
            break;

        case Countdown:
            // Update countdown timer
            countdownTimer -= frameTime;
            if (countdownTimer <= 0)
            {
                gameState = Racing;
                gameStateMessage = "Go!";
            }
            else if (countdownTimer <= 1)
            {
                gameStateMessage = "1";
            }
            else if (countdownTimer <= 2)
            {
                gameStateMessage = "2";
            }
            else
            {
                gameStateMessage = "3";
            }
            break;

        case Racing:
        {
            // Handle car movement
            if (myEngine->KeyHeld(Key_W))
            {
                Vector2D thrust = GetThrustVector(car, kAcceleration * frameTime);
                momentum = momentum + thrust;
            }
            else if (myEngine->KeyHeld(Key_S))
            {
                Vector2D reverseThrust = GetThrustVector(car, -kAcceleration * frameTime);
                momentum = momentum + reverseThrust;
            }

            // Apply drag to momentum
            Vector2D drag = GetDragVector(momentum, frameTime);
            momentum = momentum + drag;

            // Clamp speed to maximum values
            float speed = sqrt(momentum.x * momentum.x + momentum.z * momentum.z);
            if (speed > kMaxSpeed)
            {
                momentum = momentum * (kMaxSpeed / speed);
            }
            if (speed > kMaxReverseSpeed && momentum.z < 0)
            {
                momentum = momentum * (kMaxReverseSpeed / speed);
            }

            // Handle car turning
            if (myEngine->KeyHeld(Key_A))
            {
                car->RotateY(-kTurnSpeed * frameTime);
            }
            if (myEngine->KeyHeld(Key_D))
            {
                car->RotateY(kTurnSpeed * frameTime);
            }

            // Move the car based on momentum
            if (speed > 0.01f || speed < -0.01f)
            {
                car->MoveLocalX(momentum.x * frameTime);
                car->MoveLocalZ(momentum.z * frameTime);
            }

            // Check for checkpoint collisions
            if (!checkpoints[currentCheckpoint].passed && SphereToSphereCollision(car, checkpoints[currentCheckpoint], speed, currentCheckpoint))
            {
                checkpoints[currentCheckpoint].passed = true;
                currentCheckpoint++;
                if (currentCheckpoint < kNumCheckpoints)
                {
                    gameStateMessage = "Stage " + to_string(currentCheckpoint) + " complete";
                }
                else
                {
                    gameState = Finished;
                    gameStateMessage = "Race complete";
                }
            }

        

            // Check for isle collisions
            for (int i = 0; i < kNumIsles; i++)
            {
                if (SphereBoxCollision(car, isles[i]))
                {
                    momentum = momentum * kBounceEffect;
                    carHealth -= 1.0f;
                    if (carHealth < 0.0f) carHealth = 0.0f;

                    car->MoveLocalX(momentum.x * frameTime * kCollisionMoveMultiplier);
                    car->MoveLocalZ(momentum.z * frameTime * kCollisionMoveMultiplier);
                }
            }

            // Check for leg collisions with cooldown
            if (collisionCooldown <= 0.0f && CheckLegCollision(car, checkpointLegs, kNumCheckpointLegs))
            {
                momentum = momentum * kBounceEffect; // Reverse momentum (bounce effect)
                carHealth -= 1.0f;
                if (carHealth < 0.0f) carHealth = 0.0f;
                collisionCooldown = kCollisionCooldownTime; // Set cooldown to 1 second

                // Move the car slightly away from the collision point
                car->MoveLocalX(kCollisionMoveOffset);
                car->MoveLocalZ(kCollisionMoveOffset);
            }
            else
            {
                collisionCooldown -= frameTime; // Decrease cooldown over time
            }

            // Check for tank collisions
            for (int i = 0; i < kNumTanks; i++)
            {
                if (SphereBoxCollision(car, cornerTanks[i].model)) // Removed the hit flag check. now every collision will cause damage.
                {
                    momentum = momentum * kBounceEffect; // Bounce effect
                    carHealth -= 1.0f;
                    if (carHealth < 0.0f) carHealth = 0.0f;

                    // Move the car slightly away from the collision point
                    car->MoveLocalX(momentum.x * frameTime * kCollisionMoveMultiplier);
                    car->MoveLocalZ(momentum.z * frameTime * kCollisionMoveMultiplier);
                }
            }

         
            // Check if car health is depleted
            if (carHealth <= 0.0f)
            {
                momentum = Vector2D(0.0f, 0.0f);
                gameStateMessage = "Race Over";
            }
            break;
        }

        case Finished:
            // Display finish message and reset game if R key is pressed
            gameStateMessage = "Race Complete! .";
            if (myEngine->KeyHit(Key_R))
            {
                gameState = StartMenu;
                carHealth = 100.0f;
                currentCheckpoint = 0;
                momentum = Vector2D(0.0f, 0.0f);
                car->SetPosition(kInitialCarX, kInitialCarY, kInitialCarZ);
                for (int i = 0; i < kNumCheckpoints; i++)
                {
                    checkpoints[i].passed = false;
                }
            }
            break;
        }

        // Handle camera movement
        if (myEngine->KeyHeld(Key_Up))
        {
            camera->MoveLocalZ(kCameraSpeed * frameTime);
        }
        if (myEngine->KeyHeld(Key_Down))
        {
            camera->MoveLocalZ(-kCameraSpeed * frameTime);
        }
        if (myEngine->KeyHeld(Key_Right))
        {
            camera->MoveLocalX(kCameraSpeed * frameTime);
        }
        if (myEngine->KeyHeld(Key_Left))
        {
            camera->MoveLocalX(-kCameraSpeed * frameTime);
        }
        if (myEngine->KeyHit(Key_1))
        {
            camera->ResetOrientation();
            camera->SetLocalPosition(kCameraInitialX, kCameraInitialY, kCameraInitialZ);
        }
        if (myEngine->KeyHit(Key_2)) {
            isFirstPersonCam = true;

            camera->DetachFromParent();
            camera->SetLocalPosition(kFirstPersonCameraX, kFirstPersonCameraY, kFirstPersonCameraZ);
            camera->AttachToParent(car);
        }

        // Handle mouse movement for camera rotation
        int mouseX = myEngine->GetMouseMovementX();
        int mouseY = myEngine->GetMouseMovementY();

        if (mouseX != 0)
        {
            camera->RotateY(mouseX * frameTime * kMouseRotationSpeed);
        }
        if (mouseY != 0)
        {
            float currentRotationX = camera->GetLocalX();
            float newRotationX = currentRotationX + (mouseY * frameTime * kMouseRotationSpeed);
            if (newRotationX > -kCameraRotationLimit && newRotationX < kCameraRotationLimit)
            {
                camera->RotateLocalX(mouseY * frameTime * kMouseRotationSpeed);
            }
        }

        // Draw game state message and speed/health info
        font->Draw(gameStateMessage, 0, 650, kBlack);
        string speedHealth = "Speed: " + to_string(int(sqrt(momentum.x * momentum.x + momentum.z * momentum.z) * kScale * 3.6f)) +
            " km/h  Health: " + to_string(int(carHealth));
        font->Draw(speedHealth, 300, 650, kBlue);
    }

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

// Calculate thrust vector based on car orientation and thrust value
Vector2D GetThrustVector(IModel* car, float thrust)
{
    float angle = -car->GetY() * kPi / 180.0f; // Calculate angle in radians
    return Vector2D(sin(angle) * thrust, cos(angle) * thrust); // Return thrust vector
}

// Calculate drag vector based on momentum and frame time
Vector2D GetDragVector(const Vector2D& momentum, float frameTime)
{
    float speed = sqrt(momentum.x * momentum.x + momentum.z * momentum.z); // Calculate speed
    if (speed > 0.0f)
    {
        return momentum * (-kDeceleration * frameTime / speed); // Return drag vector
    }
    return Vector2D(0.0f, 0.0f); // Return zero vector if speed is zero
}

// Check collision between car and checkpoint
bool SphereToSphereCollision(IModel* car, Checkpoint& checkpoint, float speed, int currentCheckpoint)
{
    float carX = car->GetX(); // Get car's X position
    float carZ = car->GetZ();

    float checkX = checkpoint.model->GetX(); // Get checkpoint's X position
    float checkZ = checkpoint.model->GetZ();

    float dx = carX - checkX; // Calculate X distance
    float dz = carZ - checkZ;

    bool collision = (dx * dx + dz * dz) < (kCheckpointRadius * kCheckpointRadius); // Check collision

    if (collision && speed > kMaxSpeed * kCheckpointSpeedThreshold && !checkpoint.passed && currentCheckpoint == checkpoint.index)
    {
        return true;
    }

    return false;
}

// Clamp a value between min and max
float Clamp(float value, float min, float max)
{
    if (value < min) return min; // Return min if value is less than min
    if (value > max) return max; // Return max if value is greater than max
    return value;
}

// Calculate distance between two points
float Distance(float x1, float z1, float x2, float z2)
{
    float dx = x2 - x1; // Calculate X distance
    float dz = z2 - z1;
    return sqrt(dx * dx + dz * dz);
}

// Check collision between sphere (car) and box (obstacle)
bool SphereBoxCollision(IModel* car, IModel* box)
{
    float sphereX = car->GetX(); // Get car's X position
    float sphereZ = car->GetZ();
    float sphereRadius = kCarRadius; // Get car's radius

    float boxX = box->GetX(); // Get box's X position
    float boxZ = box->GetZ();

    float boxMinX = boxX - kBoxWidth / 2; // Calculate box's minimum X
    float boxMaxX = boxX + kBoxWidth / 2; // Calculate box's maximum X
    float boxMinZ = boxZ - kBoxDepth / 2;
    float boxMaxZ = boxZ + kBoxDepth / 2;

    float closestX = Clamp(sphereX, boxMinX, boxMaxX); // Clamp car's X position to box's range
    float closestZ = Clamp(sphereZ, boxMinZ, boxMaxZ);

    float distance = Distance(sphereX, sphereZ, closestX, closestZ); // Calculate distance

    return distance <= sphereRadius;
}

// Check collision between car and checkpoint legs
bool CheckLegCollision(IModel* car, CheckpointLeg legs[], int numLegs)
{
    float carX = car->GetX(); // Get car's X position
    float carZ = car->GetZ();

    for (int i = 0; i < numLegs; ++i)
    {
        CheckpointLeg leg = legs[i]; // Copy the leg object instead of using a reference
        float checkpointAngle = leg.checkpoint->GetY() * kPi / 180.0f; // Calculate angle in radians
        float legX = leg.checkpoint->GetX(); // Get leg's X position
        float legZ = leg.checkpoint->GetZ();

        if (abs(checkpointAngle) < 0.1f)
        {
            legX += leg.offsetX; // Add X offset
            legZ += leg.offsetZ;
        }
        else
        {
            legX += leg.offsetZ * sin(checkpointAngle); // Add X offset with angle
            legZ += leg.offsetX * cos(checkpointAngle);
        }

        if (CircleCollision(carX, carZ, kCarRadius, legX, legZ, kLegCollisionRadius))
        {
            return true;
        }
    }
    return false;
}

// Check collision between two circles
bool CircleCollision(float x1, float z1, float r1, float x2, float z2, float r2)
{
    float distance = Distance(x1, z1, x2, z2); // Calculate distance
    return distance < (r1 + r2);
}
