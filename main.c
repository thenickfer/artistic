#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // Para usar strings
#include <stdbool.h>
#include <time.h>
#include <omp.h>

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel Pixel (24 bits)
typedef struct
{
    unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct
{
    int width, height;
    Pixel *img;
} Img;

typedef struct
{
    int x, y;
} EdgeCoord;

/* typedef struct EdgeNode
{
    EdgeCoord coord;
    struct EdgeNode *next;
} EdgeNode; */

typedef struct
{
    int size;
    EdgeCoord coord[500000];
} EdgeArray;

// Protótipos
void load(char *name, Img *pic);
void valida();

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[2];

// As 2 imagens
Img pic[2];

// Imagem selecionada (0,1)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (Pixel *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

EdgeCoord getNearestEdge(int x, int y, EdgeArray *edgeArray)
{
    if (edgeArray->size == 0)
    {
        printf("Error: EdgeArray is empty.\n");
        return (EdgeCoord){-1, -1}; // Return an invalid coordinate
    }
    EdgeCoord nearest = edgeArray->coord[0];
    float nearestDist = sqrt(pow(nearest.x - x, 2) + pow(nearest.y - y, 2));
    for (int i = 0; i < edgeArray->size; i++)
    {
        EdgeCoord curr = edgeArray->coord[i];
        float currDist = sqrt(pow(curr.x - x, 2) + pow(curr.y - y, 2));
        if (currDist < nearestDist)
        {
            nearestDist = currDist;
            nearest = curr;
        }
    }
    return nearest;
}

void addNode(int x, int y, EdgeArray *edgeArray, bool (*visited)[width])
{
    if (visited[y][x])
        return;

    visited[y][x] = true;

    if (edgeArray->size >= 500000)
    {
        printf("Error: EdgeArray capacity exceeded.\n");
        return;
    }

    edgeArray->coord[edgeArray->size++] = (EdgeCoord){x, y};
    /*
    for (int i = 0; i < edgeArray->size; i++)
    {
        if (edgeArray->coord[i].x == x && edgeArray->coord[i].y == y)
        {
            return;
        }
    }*/

    // Aqui era o principal problema de performance, foi melhor implementar um hash pra testar se foi visitado, mas mesmo assim vai ser lento sem multithreading
}

int main(int argc, char **argv)
{

    clock_t tstart = clock();

    if (argc < 1)
    {
        printf("artistic [im. entrada]\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem de entrada
    // pic[1] -> imagem de saida

    // Carrega a imagem
    load(argv[1], &pic[0]);

    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem de entrada (0)
    pic[1].width = pic[0].width;
    pic[1].height = pic[0].height;
    pic[1].img = calloc(pic[1].width * pic[1].height, 3); // W x H x 3 bytes (Pixel)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Image Estilizeitor+");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    sel = 0; // entrada

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Converte para interpretar como matriz
    Pixel(*in)[width] = (Pixel(*)[width])pic[0].img;
    Pixel(*out)[width] = (Pixel(*)[width])pic[1].img;

    // Aplica o algoritmo e gera a saida em out (pic[1].img)
    // ...
    // ...
    // Exemplo: copia apenas o componente vermelho para a saida

    int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    int threads = omp_get_max_threads();
    omp_set_num_threads(threads);

    EdgeArray arr;
    arr.size = 0;

    int medias[3][3];

    #pragma omp parallel for collapse(2)
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gradienteX = 0;
            int gradienteY = 0;
            for (int i = -1; i < 2; i++)
            {
                for (int j = -1; j < 2; j++)
                {
                    int grayscalePix = (in[y + i][x + j].r * 0.299 + 0.587 * in[y + i][x + j].g + 0.114 * in[y + i][x + j].b);
                    gradienteX += grayscalePix * sobelX[i + 1][j + 1];
                    gradienteY += grayscalePix * sobelY[i + 1][j + 1];
                }
            }

            int mag = (int)sqrt(gradienteX * gradienteX + gradienteY * gradienteY);

            int sectionY = (y < height / 3) ? 0 : (y < 2 * height / 3) ? 1
                                                                       : 2;
            int sectionX = (x < width / 3) ? 0 : (x < 2 * width / 3) ? 1
                                                                     : 2;
            #pragma omp atomic
            medias[sectionY][sectionX] += mag;
        }
    }

    int sectionHeight = (height - 1) / 3;
    int sectionWidth = (width - 1) / 3;

    int topMidNorm = sectionHeight * sectionWidth;
    int botNorm = (height - 2 * sectionHeight) * (width - 2 * sectionHeight);

    for (int i = 0; i < 3; i++)
    {
        medias[0][i] /= topMidNorm;
        medias[1][i] /= topMidNorm;
        medias[2][i] /= botNorm;
    }

    bool (*visited)[width] = calloc(height, sizeof(bool[width]));

    #pragma omp parallel for collapse(2)
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gradienteX = 0;
            int gradienteY = 0;
            for (int i = -1; i < 2; i++)
            {
                for (int j = -1; j < 2; j++)
                {
                    int grayscalePix = (in[y + i][x + j].r * 0.299 + 0.587 * in[y + i][x + j].g + 0.114 * in[y + i][x + j].b);
                    gradienteX += grayscalePix * sobelX[i + 1][j + 1];
                    gradienteY += grayscalePix * sobelY[i + 1][j + 1];
                }
            }

            int mag = (int)sqrt(gradienteX * gradienteX + gradienteY * gradienteY);

            int sectionY = (y < height / 3) ? 0 : (y < 2 * height / 3) ? 1
                                                                       : 2;
            int sectionX = (x < width / 3) ? 0 : (x < 2 * width / 3) ? 1
                                                                     : 2;
            // meu vscode usa indentacao automatica e forca essa parte a ficar assim ^

            if (mag > medias[sectionY][sectionX])
            {
                #pragma omp critical
                addNode(x, y, &arr, visited);
            }
        }
    }

    free(visited);

    EdgeArray arrCopy;
    arrCopy.size = arr.size;

    srand(time(NULL));

    for (int i = 0; i < arr.size - 1; i++)
    {
        EdgeCoord *teste = &(arr.coord[i]);

        arrCopy.coord[i] = *teste;    

        int offsetX = width - (width - teste->x);
        int offsetY = height - (height - teste->y);

        teste->x += ((rand() % width) - offsetX) / 4;
        teste->y += ((rand() % height) - offsetY) / 4;
    }

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            EdgeCoord aux = getNearestEdge(x, y, &arr);
            out[y][x].r = in[aux.y][aux.x].r;
            out[y][x].g = in[aux.y][aux.x].g;
            out[y][x].b = in[aux.y][aux.x].b;
        }
    }

    for(int i=0; i<arrCopy.size;i++){
        EdgeCoord curr = arrCopy.coord[i];
        in[curr.y][curr.x].r = 0x85;
        in[curr.y][curr.x].g = 0x00;
        in[curr.y][curr.x].b = 0xFF; 
    }

    clock_t tend = clock();

    double tempo = ((double)(tend - tstart)) / CLOCKS_PER_SEC;


    printf("Tempo para carregar: %.6f segundos\nUsando %d threads\n", tempo/omp_get_max_threads(), omp_get_max_threads());

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza　
        free(pic[0].img);
        free(pic[1].img);
        exit(1);
    }
    if (key >= '1' && key <= '2')
        // 1-2: seleciona a imagem correspondente (origem ou destino)
        sel = key - '1';
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/Pixel.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
