// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Linear Algebra Library
#include <Eigen/Core>

// Timer
#include <chrono>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
Eigen::MatrixXf V(2,3);

// Contains the per-vertex color
Eigen::MatrixXf C(3,3);


struct Game{
    int tiles[20][10] = {0};
    void new_tile() {
        tile << 5, 0, random()%7, random()%4;
    }

    void write_tile() {
        if(!running || !started) return ;
        for(int i = 0 ; i < 4; i++) {
            tiles[tile[1]+tileMap[tile[2]][tile[3]][i][1]][tile[0]+tileMap[tile[2]][tile[3]][i][0]] = tile[2] + 1;
        }
    }

    void wipe_tile() {
        if(!running || !started) return ;
        for(int i = 0 ; i < 4; i++) {
            tiles[tile[1]+tileMap[tile[2]][tile[3]][i][1]][tile[0]+tileMap[tile[2]][tile[3]][i][0]]  = 0;
        }
    }

    void rotation() {
        if(!running || !started) return ;
        tile[3] = (tile[3]+1)%4;
        for(int i = 0 ; i < 4; i++) {
            std::pair<int,int> pos= {
                    tile[0]+tileMap[tile[2]][tile[3]][i][0],
                    tile[1]+tileMap[tile[2]][tile[3]][i][1],
            };
            if(pos.first < 0 || pos.first >= 10 || pos.second >= 20 || tiles[pos.second][pos.first]) {
                tile[3] = (tile[3]+3)%4;
                break;
            }
        }
    }

    bool move(int x, int y, bool passive = false) {
        if(!running || !started) return true;
        tile[0] += x;
        tile[1] += y;
        for(int i = 0 ; i < 4; i++) {
            std::pair<int,int> pos= {
                    tile[0]+tileMap[tile[2]][tile[3]][i][0],
                    tile[1]+tileMap[tile[2]][tile[3]][i][1],
            };
            if(pos.first < 0 || pos.first >= 10 || pos.second >= 20 || tiles[pos.second][pos.first]) {
                tile[0] -= x; tile[1] -= y;
                if(passive) {
                    write_tile();
                    if(tile[1] != 0) new_tile();
                    else {started = false; write_end(8);}
                }
                return false;
            }
        }
        check_row();
        return true;
    }

    void update() {
        if(!running || !started) return ;
        if(frame_cnt++ == 96) {
            wipe_tile();
            move(0,1,true);
            write_tile();
            frame_cnt = 0;
        }
    }

    void restart() {
        memset(tiles, 0, sizeof(int)*200);
        new_tile();
        started = true;
        running = true;
    }

    void resume_or_pause() {
        running = !running;
    }


    void write_end(int i = 8) {
        memset(tiles[std::max(i-1,0)], 0, 4*10*7);
        tiles[i+1][0] = tiles[i+3][0] = 1;
        tiles[i+0][0] = tiles[i+0][1] = tiles[i+0][2] = 1;
        tiles[i+2][0] = tiles[i+2][1] = tiles[i+2][2] = 1;
        tiles[i+4][0] = tiles[i+4][1] = tiles[i+4][2] = 1;
        tiles[i+0][3] = tiles[i+1][3] = tiles[i+2][3] = tiles[i+3][3] = tiles[i+4][3] = 2;
        tiles[i+0][6] = tiles[i+1][6] = tiles[i+2][6] = tiles[i+3][6] = tiles[i+4][6] = 2;
        tiles[i+1][4] = tiles[i+2][4] = tiles[i+2][5] = tiles[i+3][5] = 2;
        tiles[i+0][7] = tiles[i+1][7] = tiles[i+2][7] = tiles[i+3][7] = tiles[i+4][7] = 3;
        tiles[i+0][8] = tiles[i+4][8] = tiles[i+1][9] = tiles[i+2][9] = tiles[i+3][9] = 3;
    }

    Eigen::Vector4i tile; // (x, y, type, rotation)

private:
    typedef Eigen::Vector2i vec2;
    static vec2 tileMap[7][4][4];

    void check_row() {
        for(int i = 19, ii = 19; i >= 0; i--) {
            int tile_cnt = 0;
            for(int j = 0; j < 10; j++) tile_cnt += tiles[i][j] > 0;
            for(int j = 0; j < 10; j++) tiles[ii][j] = tiles[i][j] ;
            if(tile_cnt != 10) ii--;
        }
    }

    int frame_cnt = 0;
    bool running = false, started = false;
} game;


Eigen::Vector2i Game::tileMap[7][4][4] =  {
        { { vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1) },	// "L"
                  { vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1) },
                  { vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
                  { vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1) } },

          { { vec2(0, 0), vec2(-1, -1), vec2(-1, 0), vec2(0, -1) },	//"O"
                  { vec2(0, 0), vec2(-1, -1), vec2(-1, 0), vec2(0, -1) },
                  { vec2(0, 0), vec2(-1, -1), vec2(-1, 0), vec2(0, -1) },
                  { vec2(0, 0), vec2(-1, -1), vec2(-1, 0), vec2(0, -1) } },

          { { vec2(0, 0), vec2(1, 0), vec2(-1, 0), vec2(-2, 0) },	//"I"
                  { vec2(0, 0), vec2(0, -2), vec2(0, -1), vec2(0, 1) },
                  { vec2(0, 0), vec2(1, 0), vec2(-1, 0), vec2(-2, 0) },
                  { vec2(0, 0), vec2(0, -2), vec2(0, -1), vec2(0, 1) } },

          { { vec2(0, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1) },	//"S"
                  { vec2(0, 0), vec2(0, 1), vec2(1,0), vec2(1, -1) },
                  { vec2(0, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1) },
                  { vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, -1) } },

          { { vec2(0, 0), vec2(-1, 0), vec2(1, 0), vec2(1, -1) },	//"J"
                  { vec2(0, 0), vec2(0, 1), vec2(0, -1), vec2(1, 1) },
                  { vec2(0, 0), vec2(1,0), vec2(-1, 0), vec2(-1, 1) },
                  { vec2(0, 0), vec2(0, 1), vec2(0, -1), vec2(-1, -1) } },

          { { vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },	//"Z"
                  { vec2(0, -1), vec2(0, 0), vec2(1, 0), vec2(1, 1) },
                  { vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },
                  { vec2(0, -1), vec2(0, 0), vec2(1, 0), vec2(1, 1) } },

          { { vec2(0, 0), vec2(1, 0), vec2(-1, 0), vec2(0, -1) },	//"T"
                  { vec2(0, 0), vec2(0, 1), vec2(0, -1), vec2(1, 0) },
                  { vec2(0, 0), vec2(0, 1), vec2(-1, 0), vec2(1, 0) },
                  { vec2(-1, 0), vec2(0, 1), vec2(0, -1), vec2(0, 0) } }

        };



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        V.col(0) << xworld, yworld;

    // Upload the change to the GPU
    VBO.update(V);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    if(action != GLFW_RELEASE) return ;
    switch (key)
    {
        case GLFW_KEY_DOWN:
            game.wipe_tile();
            game.move(0,1);
            game.write_tile();
            break;
        case GLFW_KEY_LEFT:
            game.wipe_tile();
            game.move(-1,0);
            game.write_tile();
            break;
        case GLFW_KEY_RIGHT:
            game.wipe_tile();
            game.move(1,0);
            game.write_tile();
            break;
        case GLFW_KEY_UP:
        case GLFW_KEY_SPACE:
            game.wipe_tile();
            game.rotation();
            game.write_tile();
            break;
        case GLFW_KEY_R:
            game.restart();
            break;
        case GLFW_KEY_TAB:
            game.resume_or_pause();
            break;
        default:
            break;
    }

    // Upload the change to the GPU
    VBO.update(V);
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(340, 680, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

    V.resize(2,4);
    V <<
    1.0,  1.0, -1.0, -1.0,
    1.0, -1.0, -1.0, 1.0;
    VBO.update(V);

    // Second VBO for colors
    VBO_C.init();

    C.resize(3,4);
    C <<
    1,  0, 0, 1,
    0,  1, 0, 1,
    0,  0, 1, 1;

    VBO_C.update(C);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
// set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../texture/res.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 330 core\n"
            "in vec2 position;"
            "out vec2 pos;"
            "void main()"
            "{"
            "    pos = vec2(position.x*1.14,position.y*-1.09);"
            "    gl_Position = vec4(position, 0.0, 1.0);"
            "}";
    const GLchar* fragment_shader =
            "#version 330 core\n"
            "out vec4 outColor;"
            "in vec2 pos;"
            "uniform int tile[200];"
            "uniform sampler2D tileTex;"
            "void main()"
            "{\n"
            "    vec2 border = smoothstep(-0.1, 0.0, -abs(sin(3.1415926*pos*vec2(5.0,10.0))));\n"
            "    if(max(abs(pos.x),abs(pos.y))>1.002) {outColor.xyz = texture(tileTex,vec2(pos.x/2.28 + 0.5, 360.0/393.0*(pos.y/2.18 + 0.5 )) ).xyz;return;}int tile_id = int(floor(10.0*pos.y+10.0)*10)+int(floor(5.0*pos.x+5.0));"
            "    tile_id     = tile[tile_id<200&&tile_id>=0?tile_id:0];"
            "    vec3 tile_color = texture(tileTex,vec2( (tile_id-1+mod(abs(pos.x*5),1.0))/7.0,360.0/393.0+33.0/393.0*mod(abs(pos.y*10.0), 1.0))).xyz; "
            "    tile_color = tile_id == 0 ? vec3(0.55)*length(tile_color): tile_color;"
            "    outColor.xyz = mix(tile_color ,vec3(0.0),max(border.x,border.y));\n"
            "}";

    //
//    length(texture(tileTex,mod(vec2(abs(pos.x*5.0),abs(pos.y*10.0)),vec2(1.0))))
//    length(texture(tileTex,pos))
            
    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",VBO_C);

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
//    glfwSetMouseButtonCallback(window, mouse_button_callback);

    game.write_end(8);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glUniform1iv(program.uniform("tile"), 200,(const GLint*)game.tiles);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        game.update();

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
