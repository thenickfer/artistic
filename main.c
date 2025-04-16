#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // Para usar strings
#include <stdbool.h>
#include <time.h>

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

void addNode(int x, int y, EdgeArray *edgeArray)
{
    if (edgeArray->size >= 500000)
    {
        printf("Error: EdgeArray capacity exceeded.\n");
        return;
    }
    for (int i = 0; i < edgeArray->size; i++)
    {
        if (edgeArray->coord[i].x == x && edgeArray->coord[i].y == y)
        {
            return;
        }
    }
    EdgeCoord aux = (EdgeCoord) {x, y};
    edgeArray->coord[edgeArray->size] = aux;
    edgeArray->size += 1;
}

// Usando EdgeNode como LinkedList
/* EdgeCoord getNearestEdge(int x, int y, EdgeNode *head)
{
    EdgeNode *aux = head;      
    EdgeCoord nearest = aux->coord;
    float nearestDist = sqrt(pow(nearest.x, 2) + pow(nearest.y, 2));
    while (aux != NULL)       
    {
        aux = aux->next;
        EdgeCoord curr = aux->coord;
        float currDist = sqrt(pow(curr.x, 2) + pow(curr.y, 2));
        if (currDist < nearestDist)
        {
            nearest = curr;
            nearestDist = currDist;
        }
    }
    return nearest;
}

EdgeNode *addNode(int x, int y, EdgeNode *head)
{
    EdgeNode *aux = head;
    EdgeCoord curr;
    while (aux != NULL)
    {
        curr = aux->coord;
        if (curr.x == x && curr.y == y)
            return head;
        aux = aux->next;
    }
    aux = malloc(sizeof(EdgeNode));
    aux->coord = (EdgeCoord){x, y};
    aux->next = head;
    head = aux;
    return head;
} */

int main(int argc, char **argv)
{
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
    double media= 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            media += (in[y][x].r*0.299 + 0.587*in[y][x].g + 0.114*in[y][x].b);
        }
    }
    media /= (height * width);


    double var= 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            var += pow((in[y][x].r*0.299 + 0.587*in[y][x].g + 0.114*in[y][x].b) - media, 2);
        }
    }

    var /= (height * width);
    
    int threshold = sqrt(var); //calcular com a threshold por cor ao inves de media
    // Isso foi tudo pra definir um threshold com base na variancia de cor, provavelmente seja melhor trocar esse metodo por algo mais eficiente

    EdgeArray arr;
    arr.size = 0;

    //Obs, isso tudo se tornou extremamente ineficiente, talvez fosse melhor comparar cada pixel com uma regiao vizinha ao inves 
    //de so comparar com o da direita e o de baixo, pra assim nao precisar calcular a variancia e evitar ruido na imagem

    for (int y = 0; y < height - 1; y++)
    {
        for (int x = 0; x < width -1; x++)
        {
            bool diffX = sqrt(pow(((in[y][x].r*0.299 + 0.587*in[y][x].g + 0.114*in[y][x].b) - (in[y][x+1].r*0.299 + 0.587*in[y][x+1].g + 0.114*in[y][x+1].b)), 2)) > threshold;
            bool diffY = sqrt(pow(((in[y][x].r*0.299 + 0.587*in[y][x].g + 0.114*in[y][x].b)- (in[y+1][x].r*0.299 + 0.587*in[y+1][x].g + 0.114*in[y+1][x].b)), 2)) > threshold;

            if (diffX || diffY)
            {
                
                addNode(x, y, &arr);
                if (diffX)
                    addNode(x + 1, y, &arr);
                if (diffY)
                    addNode(x, y + 1, &arr);
            }
        }
    } 


    srand(time(NULL));

    for (int i = 0; i < arr.size - 1; i++)
    {   
        EdgeCoord *teste = &(arr.coord[i]);

        int offsetX = width - (width - teste->x);
        int offsetY = height - (height - teste->y); 

        teste->x += ((rand()%width)-offsetX)/4;
        teste->y += ((rand()%height)-offsetY)/4;
        
    }

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

    for(int i=0;i<arr.size;i++){
        EdgeCoord coord = arr.coord[i];
        in[coord.y][coord.x].r = 255;
        in[coord.y][coord.x].g = 0;
        in[coord.y][coord.x].b = 0;
    }

    /* for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            out[y][x].r = in[y][x].r;
            out[y][x].g = in[y][x].g;
            out[y][x].b = in[y][x].b;
        }
    } */
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
