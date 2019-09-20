#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>
#include "Texture.h"
#include <iostream>
#include <map>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#include <vector>
#include <list>
using namespace std;
//////////////////////////////////------------------------------------------------------------

#define SEARCH_RANGE 50 //开始点指定误差
#define PIC_CHANNEL 4
#define PIX_OK(_DATA, _POS) (_DATA[_POS] > 200 && _DATA[_POS+1] < 100 && _DATA[_POS+2] < 100)
#define ACCESS_OK(_X, _Y) (!access[_Y * m_mapWidth + _X])

#define PIX_OK2(_X, _Y) (PIX_OK(m_mapData,((_Y * m_mapWidth + _X) * PIC_CHANNEL)))

#define BUILD_POINT(_X, _Y) (new Point(_X, _Y, 0, CalcH(m_end, Point(_X, _Y))))


struct Point
{
	int x, y;
	Point() {};
	Point(int tx, int ty, int g=0,int h = 0)
	{
		x = tx;
		y = ty;
		G = g;
		H = h;
		ParentPoint = NULL;
		CalcF();
	}

	void CalcF()
	{
		F = G + H;
	}
	bool operator < (const Point& p)
	{
		return this->F < p.F;
	}

	bool approximate(const Point& p)
	{
		return abs(x - p.x) <= SEARCH_RANGE && abs(y - p.y	) <= SEARCH_RANGE;
	}

	bool operator == (const Point& p)
	{
		return this->x == p.x && this->y == p.y;
	}

	Point* ParentPoint = NULL;
	int F;
	int G;
	int H;
};



//////////////////////////////////------------------------------------------------------------

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

class CityInfo
{
public:
	CityInfo(int id):city_id(id){}

	int city_id;
	vector<int> m_wpId;//路点id
};

//vector<vector<bool>> access;
bool* access;
Point** openListMap;

class WayPoint
{

public:
	WayPoint(Point start, Point end, unsigned char * mapData, int mapWidth, int mapHeight)
	{
		m_mapData = mapData;
		m_mapWidth = mapWidth;
		m_mapHeight = mapHeight;
		m_start = start;
		m_end = end;
		//Init(start);
		Point* r = InitWay(start);
		/*Point* t1 = r->ParentPoint;
		Point* t2 = r->ParentPoint->ParentPoint;
		while (t1 != t2)
		{
			t1 = t1->ParentPoint;
			t2 = t2->ParentPoint->ParentPoint;

		}
		int count = 1;
		t2 = t2->ParentPoint;
		while (t1 != t2)
		{
			t2 = t2->ParentPoint;
			count++;
		}

		t1 = r;
		t2 = r;
		while (count > 0)
		{
			t2 = t2->ParentPoint;
			count--;
		}
		while (t2 != t1)
		{
			t1 = t1->ParentPoint;
			t2 = t2->ParentPoint;
		}*/

		while (r)
		{
			m_way.push_front(r);
			r = r->ParentPoint;
		}
		int i = 0;
	}


	Point* InitWay(Point start)
	{
		ClearAccess();
		statistic = 0;
		closetPoint = NULL;
		for (int offset = 0; offset < SEARCH_RANGE; ++offset)
		{
			int top_left_x = start.x - offset;
			int top_left_y = start.y - offset;

			int top_right_x = start.x + offset;
			int top_right_y = start.y - offset;

			int bottom_left_x = start.x - offset;
			int bottom_left_y = start.y + offset;

			int bottom_right_x = start.x + offset;
			int bottom_right_y = start.y + offset;

			////////////     -------|
			///////////      |      |
			///////////      |      |
			///////////      |      |
			///////////      |-------
			for (int i = 0; i < offset * 2; ++i)
			{
				AddOpenList(top_left_x + i, top_left_y);
				AddOpenList(top_right_x, top_right_y + i);
				AddOpenList(bottom_left_x, bottom_left_y - i);
				AddOpenList(bottom_right_x - i, bottom_right_y);
			}
		}
		while (!openList.empty())
		{
			statistic++;
			Point* p = GetOpenPoint();
			if (!closetPoint || dis(*p, m_end) < dis(*closetPoint, m_end))
			{
				closetPoint = p;
			}
			if (m_end.approximate(*p))
			{
				return p;
			}
			AddOpenList(p->x - 1, p->y - 1, p);
			AddOpenList(p->x - 1, p->y, p);
			AddOpenList(p->x - 1, p->y + 1, p);

			AddOpenList(p->x, p->y - 1, p);
			AddOpenList(p->x, p->y + 1, p);

			AddOpenList(p->x + 1, p->y - 1, p);
			AddOpenList(p->x + 1, p->y, p);
			AddOpenList(p->x + 1, p->y + 1, p);
		}
		return NULL;
	}

	Point* GetOpenPoint()
	{
		auto iter = openList.begin();
		for (auto i = openList.begin(); i != openList.end(); ++i)
		{
			if (*i < *iter)
			{
				iter = i;
			}
		}
		Point* p = *iter;
		openList.erase(iter);
		closeList.push_back(p);
		access[p->y * m_mapWidth + p->x] = true;
		return p;
	}

	void AddOpenList(int x, int y, Point* parent)
	{
		Point* p = openListMap[y*m_mapWidth + x];
		if (!p)
		{
			AddOpenList(x,y);
			p = openListMap[y*m_mapWidth + x];
			if (p)
			{
				p->ParentPoint = parent;
				p->G = parent->G + CalcH(*p, *parent);
				p->ParentPoint = parent;
				p->CalcF();
			}
		}
		p = openListMap[y*m_mapWidth + x];
		if (!p)
		{
			return;
		}
		if (p->ParentPoint && parent->G < p->ParentPoint->G)
		{
			p->ParentPoint = parent;
			p->G = parent->G + CalcH(*p, *parent);
			p->ParentPoint = parent;
			p->CalcF();
		}
	}

	void AddOpenList(int x, int y)
	{
		if (x < 0 || x >= m_mapWidth || y < 0 || y >= m_mapHeight || !PIX_OK2(x, y))
		{
			return;
		}
		Point* p = BUILD_POINT(x, y);
		openListMap[y*m_mapWidth + x] = p;
		openList.push_back(p);
	}

	int CalcH(Point start, Point end)
	{
		return abs(start.x - end.x) + abs(start.y - end.y);
	}

	int dis(Point start, Point end)
	{
		return abs(start.x - end.x) * abs(start.x - end.x) + abs(start.y - end.y) * abs(start.y - end.y);
	}

	list<Point*> openList;
	list<Point*> closeList;
	Point* closetPoint;


	void ClearAccess()
	{
		memset(access, 0, sizeof(bool) * m_mapWidth * m_mapHeight);
		memset(openListMap, NULL, sizeof(int) * m_mapWidth * m_mapHeight);
	}

	unsigned char* m_mapData;
	int m_mapWidth;
	int m_mapHeight;
	int statistic;
	int statisticOff;
	Point m_start;
	Point m_end;
	list<Point*> m_way;
private:
};

class MapParser
{
#define ROAD_NAME(_CITY1, _CITY2) (((_CITY1) * 1000) + (_CITY2))
public:
	MapParser(unsigned char * mapData, int mapWidth, int mapHeight):m_mapData(mapData),m_mapWidth(mapWidth),
		m_mapHeight(mapHeight)
	{
		m_offsetX = 0;
		m_offsetY = 0;

		access = new bool[mapWidth * mapHeight];
		openListMap = new Point*[mapWidth * mapHeight];
		initCityPos();
		initNeighborMap();
		initCity();
	}

	void initCity()
	{
		for (int cityId = 1; cityId <= m_cityPos.size(); ++cityId)
		{
			cout << "export city" << cityId << endl;
			for (auto neighborCity : m_neighborMap[cityId - 1])
			{
				if (m_roadName2WPID.find(ROAD_NAME(cityId, neighborCity)) == m_roadName2WPID.end())
				{
					auto& posBegin = m_cityPos[min(cityId - 1, neighborCity - 1)];
					auto& posEnd = m_cityPos[max(cityId - 1, neighborCity - 1)];
					m_wp.push_back(WayPoint(Point(posBegin[0] + m_offsetX, posBegin[1] + m_offsetY),
						Point(posEnd[0] + m_offsetX, posEnd[1] + m_offsetY),
						m_mapData, m_mapWidth, m_mapHeight
					));
					m_roadName2WPID[ROAD_NAME(cityId, neighborCity)] = m_wp.size() - 1;
					m_roadName2WPID[ROAD_NAME(neighborCity, cityId)] = m_wp.size() - 1;
				}
			}
			
		}
	}

	void initCityPos()
	{
		m_cityPos = {
			{3922, 928},
{3336, 1122},
{2720, 851},
{2813, 1113},
{2334, 1101},
{1938, 977},
{2996, 885},
{3291, 815},
{3513, 609},
{3350, 491},
{3697, 507},
{3356, 1414},
{3661, 1177},
{3743, 1467},
{4127, 1256},
{3487, 1723},
{3102, 1772},
{3185, 2188},
{4222, 1665},
{4040, 1497},
{2673, 2145},
{2970, 1941},
{2709, 1660},
{2404, 1590},
{2894, 1519},
{3017, 1231},
{2544, 1352},
{2160, 1451},
{2138, 1687},
{2548, 1938},
{1760, 1570},
{1702, 1407},
{1430, 1433},
{1062, 1549},
{1201, 1321},
{1150, 1138},
{1426, 1167},
{1970, 1303},
{1624, 1006},
{1282, 838},
{1496, 726},
{1297, 613},
{1457, 411},
{917, 713},
{1028, 448},
{2283, 2038},
{2036, 1961},
{1718, 2083},
{1473, 1795},
{1367, 2075},
{1151, 1849},
{1122, 2179},
{1901, 735},
{1966, 561},
{2156, 766},
{2281, 613},
{2486, 785},
{2691, 598},
{2591, 417},
{2929, 449},
{664, 474},
{836, 926},
{574, 906},
{595, 1164},
{897, 1317},
{780, 1636},
{830, 1909},
{776, 2125},
{1181, 2430},
{1618, 2316},
{1972, 2232},
{2176, 2298},
{2384, 2449},
{2842, 2477},
{3248, 2380},
{3402, 1999},
{3755, 1986},
{4154, 1923},
{3543, 2277},
{3939, 2205},

		};
	}

	void initNeighborMap()
	{
		m_neighborMap = { {2, 9, 15},
{1, 4, 8, 13},
{4, 7},
{2, 3, 5, 7},
{4, 6, 57},
{5, 39, 55},
{3, 4},
{2, 9},
{1, 8, 10, 11},
{9, 11, 60},
{9, 10},
{13, 14, 25, 26},
{2, 12},
{12, 15, 16, 20},
{1, 14},
{14, 17},
{16, 22, 23, 30},
{21, 22, 75, 76},
{20, 77},
{14, 19},
{18, 46},
{17, 18},
{24, 25, 17, 30},
{23, 28},
{12, 23},
{4, 12, 27},
{26, 38},
{24, 32},
{30, 47},
{17, 23, 29, 46},
{32, 49, 51, 66},
{31, 28, 38, 33},
{32, 34},
{33, 35, 65, 66},
{34, 37},
{37, 62, 65},
{35, 36, 39},
{27, 32, 39},
{6, 37, 38, 53, 40, 41},
{39, 41},
{39, 40, 42},
{41, 43, 44},
{42, 45},
{42, 45, 62, 63},
{43, 44, 61},
{30, 47, 71},
{46, 48, 29},
{47, 50},
{31, 48, 50},
{48, 49, 51, 52, 70},
{31, 66, 50},
{50, 68},
{54, 39},
{53, 56},
{6, 56},
{54, 55, 57},
{5, 56, 58},
{57, 59, 60},
{58, 60},
{58, 59, 10},
{45, 63},
{44, 63, 36, 64},
{61, 62, 44},
{62, 65},
{34, 36, 64},
{31, 34, 51, 67},
{66, 68},
{67, 52},
{70},
{50, 69, 71},
{70, 72, 46},
{71, 73},
{72, 74},
{73, 75},
{74, 79, 76},
{18, 75, 77},
{76, 78, 79, 19},
{77, 80},
{75, 77},
{78},

		};
	}
	
	unsigned char* m_mapData;
	int m_mapWidth;
	int m_mapHeight;
	vector<vector<int>> m_cityPos;
	vector<vector<int>> m_neighborMap;
	vector<WayPoint> m_wp;
	map<int, int> m_roadName2WPID;
	int m_offsetX, m_offsetY;
};

class LuaExporter
{
public:
	LuaExporter(string fileName, MapParser& data)
	{
		ofstream vShaderFile;
		vShaderFile.open(fileName, ios_base::out | ios::trunc);
		
		cout << "start export head" << endl;
		vShaderFile << "Name2WPID={ \n";

		for (auto& in : data.m_roadName2WPID)
		{
			vShaderFile << "[" << in.first << "] = " << in.second << "\n";
		}
		vShaderFile << "} \n";
		cout << "start export WayPoint" << endl;
		vShaderFile << "WayPoint = { \n";
		for (auto& v : data.m_wp)
		{
			vShaderFile << "{ ";
			for (const auto& i : v.m_way)
			{
				vShaderFile << "{" << i->x << "," << i->y << "}, ";
			}
			vShaderFile << "}, \n";
		}
		vShaderFile << "} \n";
		vShaderFile.close();
	}
};


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
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

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
	unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/road.png").c_str(), &width, &height, &nrChannels, 0);
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

	unsigned char *data2 = stbi_load(FileSystem::getPath("resources/textures/road_way.png").c_str(), &width, &height, &nrChannels, 0);
	if (data2)
	{
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	int r = data2[0];
	int g = data2[1];
	int b = data2[2];
	int r2 = data2[0 + 8];
	int g2 = data2[1 + 8];
	int b2 = data2[2 + 8];

	int r3 = data2[0 + (width-1) * 4];
	int g3 = data2[1 + (width - 1) * 4];
	int b3 = data2[2 + (width - 1) * 4];

	int r1 = data2[0 + width*4];
	int g1 = data2[1 + width*4];
	int b1 = data2[2 + width*4];
	//WayPoint way({154,34}, {589,93}, data2, width, height);
	MapParser parser(data2, width, height);
	LuaExporter exporter(FileSystem::getPath("resources/textures/CityWayPoint.lua").c_str(), parser);
	stbi_image_free(data2);
	vector<float> vertices;
	vector<unsigned int> indices;
	for (auto& way : parser.m_wp)
	{
		int offset = vertices.size() / 3;
		int i = 0;
		for (auto p : way.m_way)
		{
			float x = (float(p->x) / width * 2.0 - 1);
			float y = (float(p->y) / height * 2.0 - 1);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(0);
			if (i > 0)
			{
				indices.push_back(i - 1 + offset);
				indices.push_back(i + offset);
			}
			i++;
		}
	}

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	Shader postShader("post.vs", "post.fs");
	Shader lineShader("line.vs", "line.fs");

	glLineWidth(4);
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
		postShader.use();
		glBindTexture(GL_TEXTURE_2D, texture);
		renderQuad();
		lineShader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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