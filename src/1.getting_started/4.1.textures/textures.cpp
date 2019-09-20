#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#include <vector>
using namespace std;

const float arrow_size = 0.02;

struct Point
{
	float x, y;
	Point() {};
	Point(float tx, float ty)
	{
		x = tx;
		y = ty;
	}

};

vector<vector<float>> p;
vector<vector<float>> bp;


double getRatio(double t, double a, double b, double c, double d)
{
	return a * pow(t, 3) + b * pow(t, 2) + c * t + d;
}

vector<vector<float>> Bezier2(vector<vector<float>> poss, int precision)
{
	int dimersion = 2;
	int number = poss.size();
	if (number < 2 || dimersion < 2)
		return vector<vector<float>>();
	vector<vector<float>> result(precision + 1);
	for (auto& r : result)
	{
		r.resize(2);
	}

	vector<float> mi(number);
	mi[0] = mi[1] = 1;
	for (int i = 3; i <= number; i++) {

		vector<float> t(i - 1);
		for (int j = 0; j < t.size(); j++) {
			t[j] = mi[j];
		}

		mi[0] = mi[i - 1] = 1;
		for (int j = 0; j < i - 2; j++) {
			mi[j + 1] = t[j] + t[j + 1];
		}
	}

	for (int i = 0; i < precision; i++) {
		float t = (float)i / precision;
		for (int j = 0; j < dimersion; j++) {
			float temp = 0.0f;
			for (int k = 0; k < number; k++) {
				float a1 = pow(1 - t, number - k - 1);
				float a2 = poss[k][j];
				float a3 = pow(t, k);
				temp += a1 * a2 * a3 * mi[k];
			}
			result[i][j] = temp;
		}
	}
	result[precision][0] = poss[poss.size() - 1][0];
	result[precision][1] = poss[poss.size() - 1][1];
	return result;
}

void Bezier()
{
	bp = Bezier2(p, 10);
}

void AddArrow(vector<float>& src, vector<float>& src2,  vector<unsigned int>& srcIndices, int _x, int _y)
{
	float x = _x / 1000.0f;
	float y = _y / 1000.0f;
	vector<float> vertices = {
		x + 0.5f * arrow_size,y + 0.5f * arrow_size, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		x + 0.5f * arrow_size,y + -0.5f * arrow_size, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		x + -0.5f * arrow_size,y + -0.5f * arrow_size, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		x + -0.5f * arrow_size,y + 0.5f * arrow_size, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};

	src.insert(src.end(), vertices.begin(), vertices.end());

	int sz = src.size() / 8;
	unsigned int indices[] = {
	   sz - 4, sz - 3, sz - 1, // first triangle
	   sz - 3, sz - 2, sz - 1  // second triangle
	};

	//srcIndices.insert(srcIndices.end(), indices, indices+6);

	src2.push_back(x);
	src2.push_back(y);
	src2.push_back(0);

	int sz2 = src2.size() / 3;
	if (sz2 == 0)
	{
	}
	else
	{
		srcIndices.push_back(sz2 - 1);
		srcIndices.push_back(sz2);
	}

}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	glEnable(GL_PROGRAM_POINT_SIZE);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("4.1.texture.vs", "4.1.texture.fs"); 
    Shader pointShader("point.vs", "point.fs");

     //set up vertex data (and buffer(s)) and configure vertex attributes
     //------------------------------------------------------------------
 //   float vertices[] = {
 //       // positions          // colors           // texture coords
 //        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
 //        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
 //       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
 //       -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
 //   };

	//unsigned int indices[] = {
	//	 0,1,3,
	//	 1,2,3
	// };

	/*p = { Point(-0.5,-0.5),
		Point(-1,-0.25),
		Point(1,0.25),
		Point(0.5,0.5)
	};*/
	p = { {200,200},
		{-400,100},
		{400,-100},
		{-200,-200},
	}; 

	Bezier();

	vector<float> vertices, vertices2;
	vector<unsigned int>  indices;


	//AddArrow(vertices, indices,0, 0);

	for (auto p : bp)
	{
		AddArrow(vertices, vertices2, indices, p[0],p[1]);
	}

	//AddArrow(vertices, indices,0.2, 0.2);
	//AddArrow(vertices, indices, 0.4, 0.4);
	//AddArrow(vertices, indices, 0.6, 0.6);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(),GL_STATIC_DRAW);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	unsigned int VBO2, VAO2, EBO2;
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	float vertices3[] = {
		//0.9f,0.9f,0,
		0.0f,0.5f,0,
		//0.9f,0.9f,0,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices2.size(), vertices2.data(), GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3), vertices3, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), indices.data(), GL_STATIC_DRAW);


    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    //unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
    unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/ptnbs_3002_2.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

	glLineWidth(1);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // render container
        //ourShader.use();
        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        //glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
		pointShader.use();

		glBindVertexArray(VAO2);
		glDrawArrays(GL_POINTS, 0, vertices2.size() / 3);
		glDrawElements(GL_LINES, indices.size() - 1, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}