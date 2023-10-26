// #include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Shader.h"
#include "Renderer.h"
#include "Mesh.h"
#include "MassSpringSolver.h"
#include "UserInteraction.h"

// GLOBALS

// Window
GLFWwindow *window;
static int g_windowWidth = 640;
static int g_windowHeight = 640;
static bool g_mouseClickDown = false;
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static float g_mouseClickX;
static float g_mouseClickY;

bool firstMouse = true;

// User Interaction
static UserInteraction *UI;
static Renderer *g_pickRenderer;

// Constants
static const float PI = glm::pi<float>();

// Shader Handles
static PhongShader *g_phongShader; // linked phong shader
static PickShader *g_pickShader;   // linked pick shader

// Shader parameters
static const glm::vec3 g_albedo(0.0f, 0.3f, 0.7f);
static const glm::vec3 g_ambient(0.01f, 0.01f, 0.01f);
static const glm::vec3 g_light(1.0f, 1.0f, -1.0f);

// Mesh
static Mesh *g_clothMesh; // halfedge data structure

// Render Target
static Renderer renderer;
static ProgramInput *g_render_target; // vertex, normal, texture, index

// Animation
static const int g_fps = 60;        // frames per second  | 60
static const int g_iter = 5;        // iterations per time step | 10
static const int g_frame_time = 15; // approximate time for frame calculations | 15
static const int g_animation_timer = (int)((1.0f / g_fps) * 1000 - g_frame_time);

// Mass Spring System
static mass_spring_system *g_system;
static MassSpringSolver *g_solver;

// System parameters
namespace SystemParam
{
    static const int n = 33;                    // must be odd, n * n = n_vertices | 61
    static const float w = 2.0f;                // width | 2.0f
    static const float h = 0.008f;              // time step, smaller for better results | 0.008f = 0.016f/2
    static const float r = w / (n - 1) * 1.05f; // spring rest length
    static const float k = 1.0f;                // spring stiffness | 1.0f
    static const float m = 0.25f / (n * n);     // pint mass | 0.25f
    static const float a = 0.993f;              // damping, clost to 1.0 | 0.993f
    static const float g = 9.8f * m;            // gravitational force | 9.8f
}

// Constraint Graph
static CgRootNode *g_cgRootNode;

// Scene parameters
static const float g_camera_distance = 4.2f;

// Scene matrices
static glm::mat4 g_ModelViewMatrix;
static glm::mat4 g_ProjectionMatrix;

// Functions
// state initialization
static void initGlfwState();
static void initGLState();
static void display();

static void initShaders(); // Read, compile and link shaders
static void initCloth();   // Generate cloth mesh
static void initScene();   // Generate scene matrices
static void initRenderer();
// demos
static void demo_hang(); // curtain hanging from top corners
static void demo_drop(); // curtain dropping on sphere
// static void (*g_demo)() = demo_drop;
static void (*g_demo)() = demo_hang;

// draw cloth function
static void drawCloth();
static void animateCloth();

// scene update
static void updateProjection();
static void updateRenderTarget();

// glfw callbacks
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
static void processInput(GLFWwindow *window);

// error checks
void checkGlErrors();

int main()
{
    try
    {
        initGlfwState();
        initGLState();
        initShaders();
        initCloth();
        initScene();
        initRenderer();
        display();

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
}

static void initGlfwState()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // create window
    window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Cloth Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    return;
}

static void initGLState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadBuffer(GL_BACK);
    glEnable(GL_FRAMEBUFFER_SRGB);

    checkGlErrors();
}

static void initShaders()
{
    GLShader basic_vert(GL_VERTEX_SHADER);
    GLShader phong_frag(GL_FRAGMENT_SHADER);
    GLShader pick_frag(GL_FRAGMENT_SHADER);
    auto ibasic = std::ifstream("./shaders/basic.vshader");
    auto iphong = std::ifstream("./shaders/phong.fshader");
    auto ifrag = std::ifstream("./shaders/pick.fshader");

    basic_vert.compile(ibasic);
    phong_frag.compile(iphong);
    pick_frag.compile(ifrag);

    g_phongShader = new PhongShader;
    g_pickShader = new PickShader;
    g_phongShader->link(basic_vert, phong_frag);
    g_pickShader->link(basic_vert, pick_frag);

    checkGlErrors();
}

static void initCloth()
{
    // short hand
    const int n = SystemParam::n;
    const float w = SystemParam::w;

    // generate mesh
    MeshBuilder meshBuilder;
    meshBuilder.uniformGrid(w, n);
    g_clothMesh = meshBuilder.getResult();

    // fill program input
    g_render_target = new ProgramInput;
    g_render_target->setPositionData(g_clothMesh->vbuff(), g_clothMesh->vbuffLen());
    g_render_target->setNormalData(g_clothMesh->nbuff(), g_clothMesh->nbuffLen());
    g_render_target->setTextureDate(g_clothMesh->tbuff(), g_clothMesh->tbuffLen());
    g_render_target->setIndexData(g_clothMesh->ibuff(), g_clothMesh->ibuffLen());

    checkGlErrors();

    g_demo();
}

static void initScene()
{
    g_ModelViewMatrix = glm::lookAt(
                            glm::vec3(0.618, -0.786, 0.3f) * g_camera_distance,
                            glm::vec3(0.0f, 0.0f, -1.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)) *
                        glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, SystemParam::w / 4));
    updateProjection();
}

static void initRenderer()
{
    renderer.setProgram(g_phongShader);
    renderer.setModelview(g_ModelViewMatrix);
    renderer.setProjection(g_ProjectionMatrix);
    g_phongShader->setAlbedo(g_albedo);
    g_phongShader->setAmbient(g_ambient);
    g_phongShader->setLight(g_light);
    renderer.setProgramInput(g_render_target);
    renderer.setElementCount(g_clothMesh->ibuffLen());
}

static void demo_hang()
{
    // short hand
    const int n = SystemParam::n;

    // initializa mass spring system
    MassSpringBuilder massSpringBuilder;
    massSpringBuilder.uniformGrid(
        SystemParam::n,
        SystemParam::h,
        SystemParam::r,
        SystemParam::k,
        SystemParam::m,
        SystemParam::a,
        SystemParam::g);
    g_system = massSpringBuilder.getResult();

    // initialize mass spring solver
    g_solver = new MassSpringSolver(g_system, g_clothMesh->vbuff());

    // deformation constraint parameters
    const float tauc = 0.4f;            // critical spring deformation | 0.4f
    const unsigned int deformIter = 15; // number of iterations | 15

    // initialize constraints
    // spring deformation constraint
    CgSpringDeformationNode *deformationNode =
        new CgSpringDeformationNode(g_system, g_clothMesh->vbuff(), tauc, deformIter);
    deformationNode->addSprings(massSpringBuilder.getShearIndex());
    deformationNode->addSprings(massSpringBuilder.getStructIndex());

    // fix top corners
    CgPointFixNode *cornerFixer = new CgPointFixNode(g_system, g_clothMesh->vbuff());
    cornerFixer->fixPoint(0);
    cornerFixer->fixPoint(n - 1);

    // initialize user interaction
    g_pickRenderer = new Renderer();
    g_pickRenderer->setProgram(g_pickShader);
    g_pickRenderer->setProgramInput(g_render_target);
    g_pickRenderer->setElementCount(g_clothMesh->ibuffLen());
    g_pickShader->setTessFact(SystemParam::n);
    CgPointFixNode *mouseFixer = new CgPointFixNode(g_system, g_clothMesh->vbuff());
    UI = new GridMeshUI(g_pickRenderer, mouseFixer, g_clothMesh->vbuff(), n);

    // build constraint graph
    g_cgRootNode = new CgRootNode(g_system, g_clothMesh->vbuff());

    // first layer
    g_cgRootNode->addChild(deformationNode);

    // second layer
    deformationNode->addChild(cornerFixer);
    deformationNode->addChild(mouseFixer);
}

static void demo_drop()
{
    // short hand
    const int n = SystemParam::n;

    // initializa mass spring systems
    MassSpringBuilder massSpringBuilder;
    massSpringBuilder.uniformGrid(
        SystemParam::n,
        SystemParam::h,
        SystemParam::r,
        SystemParam::k,
        SystemParam::m,
        SystemParam::a,
        SystemParam::g);
    g_system = massSpringBuilder.getResult();

    // initialize mass spring solver
    g_solver = new MassSpringSolver(g_system, g_clothMesh->vbuff());

    // sphere collision constraint parameters
    const float radius = 0.64f;             // sphere radius | 0.64f
    const Eigen::Vector3f center(0, 0, -1); // sphere center | (0, 0, -1)

    // deformation constraint parameters
    const float tauc = 0.12f;           // critical spring deformation | 0.12f
    const unsigned int deformIter = 15; // number of iterations | 15

    // initialize constraints
    // sphere collision constraint
    CgSphereCollisionNode *sphereCollisionNode =
        new CgSphereCollisionNode(g_system, g_clothMesh->vbuff(), radius, center);

    // spring deformation constraint
    CgSpringDeformationNode *deformationNode =
        new CgSpringDeformationNode(g_system, g_clothMesh->vbuff(), tauc, deformIter);
    deformationNode->addSprings(massSpringBuilder.getShearIndex());
    deformationNode->addSprings(massSpringBuilder.getStructIndex());

    // initialize user interaction
    g_pickRenderer = new Renderer();
    g_pickRenderer->setProgram(g_pickShader);
    g_pickRenderer->setProgramInput(g_render_target);
    g_pickRenderer->setElementCount(g_clothMesh->ibuffLen());
    g_pickShader->setTessFact(SystemParam::n);
    CgPointFixNode *mouseFixer = new CgPointFixNode(g_system, g_clothMesh->vbuff());
    UI = new GridMeshUI(g_pickRenderer, mouseFixer, g_clothMesh->vbuff(), n);

    // build constraint graph
    g_cgRootNode = new CgRootNode(g_system, g_clothMesh->vbuff());

    // first layer
    g_cgRootNode->addChild(deformationNode);
    g_cgRootNode->addChild(sphereCollisionNode);

    // second layer
    deformationNode->addChild(mouseFixer);
}

// GLFW Callbacks
static void display()
{
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.25f, 0.25f, 0.25f, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // cloth simulation
        animateCloth();
        drawCloth();

        // glfw: swap buffers and poll IO events (keys pressed / released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return;
}
// button click && mouse move event callback
static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    g_mouseLClickButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    g_mouseRClickButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    g_mouseMClickButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;

    g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;

    int xpos = static_cast<float>(xposIn);
    int ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        g_mouseClickX = xpos;
        g_mouseClickY = ypos;
        firstMouse = false;
    }

    const float xoffset = xpos - g_mouseClickX;
    const float yoffset = g_mouseClickY - ypos; // reversed since y-coordinates go from bottom to top

    std::cout << g_mouseClickDown << std::endl;
    if (g_mouseClickDown)
    {
        // todo more user interaction
        UI->setModelview(g_ModelViewMatrix);
        UI->setProjection(g_ProjectionMatrix);
        UI->grabPoint(g_mouseClickX, g_mouseClickY);
        glm::vec3 ux(0, 1, 0);
        glm::vec3 uy(0, 0, -1);
        UI->movePoint(0.01f * (xoffset * ux + yoffset * uy));
        std::cout << "test" << std::endl;
    }
    else
    {
        UI->releasePoint();
    }

    g_mouseClickX = xpos;
    g_mouseClickY = ypos;
}

static void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}

// CLOTH
static void drawCloth()
{
    // render
    renderer.draw();
}

static void animateCloth()
{
    // solve two time-steps
    g_solver->solve(g_iter);
    g_solver->solve(g_iter);

    // fix points
    CgSatisfyVisitor visitor;
    visitor.satisfy(*g_cgRootNode);

    // update normals
    g_clothMesh->request_face_normals();
    g_clothMesh->update_normals();
    g_clothMesh->release_face_normals();

    // update target
    updateRenderTarget();
}

// SCENE UPDATE
static void updateProjection()
{
    g_ProjectionMatrix = glm::perspective(PI / 4.0f,
                                          g_windowWidth * 1.0f / g_windowHeight, 0.01f, 1000.0f);
}

static void updateRenderTarget()
{
    // update vertex positions
    g_render_target->setPositionData(g_clothMesh->vbuff(), g_clothMesh->vbuffLen());

    // update vertex normals
    g_render_target->setNormalData(g_clothMesh->nbuff(), g_clothMesh->vbuffLen());
}

// ERRORS
void checkGlErrors()
{
    const char *errorDesc;
    int code = glfwGetError(&errorDesc);

    if (errorDesc)
    {
        std::string error("GL Error: ");
        error += errorDesc;
        std::cerr << error << std::endl;
        throw std::runtime_error(error);
    }
}