// #include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// GLOBALS

// Window
static int g_windowWidth = 640;
static int g_windowHeight = 640;
static bool g_mouseClickDown = false;
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static float g_mouseClickX;
static float g_mouseClickY;

bool firstMouse = true;

// Functions
// state initialization
static void initGlfwState();
static void initGLState();

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
        // glewInit();
        initGLState();
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
    GLFWwindow *window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Cloth Simulation", NULL, NULL);
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

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.25f, 0.25f, 0.25f, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // todo -> cloth simulation

        // glfw: swap buffers and poll IO events (keys pressed / released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
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
// GLFW Callbacks
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

    if (g_mouseClickDown)
    {
        // todo more user interaction
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