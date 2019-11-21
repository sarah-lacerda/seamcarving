#include <stdio.h>
#include <stdlib.h>
#include <string.h>        // Para usar strings

#ifdef WIN32
#include <windows.h>    // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>     // Funções da OpenGL
#include <GL/glu.h>    // Funções da GLU
#include <GL/glut.h>   // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel RGB (24 bits)
typedef struct {
    unsigned char r, g, b;
} RGB;

// Uma imagem RGB
typedef struct {
    int width, height;
    RGB* img;
} Img;

// Protótipos
void load(char* name, Img* pic);
void uploadTexture();
int energyCalculator(RGB previousPixel, RGB nextPixel, RGB topPixel, RGB lowerPixel);
int accumulatedEnergy(int* energy, int maxWidth, int actualWidth, int i);

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height, cont = 0, cont1 = 0;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char* name, Img* pic)
{
    int chan;
    pic->img = (RGB*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if(!pic->img)
    {
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

int main(int argc, char** argv)
{

    if(argc < 2) {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc,argv);




    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if(pic[0].width != pic[1].width || pic[0].height != pic[1].height) {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width  = pic[1].width;
    pic[2].height = pic[1].height;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc (keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char*) pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char*) pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Destino : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0,width,height,0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width*height*3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}


// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{

    if(key==27) {
      // ESC: libera memória e finaliza
      free(pic[0].img);
      free(pic[1].img);
      free(pic[2].img);
      exit(1);
    }
    if(key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if(key == 's') {
        // Aplica o algoritmo e gera a saida em pic[2].img...
        // ...
        // ... (crie uma função para isso!)

        // Exemplo: pintando tudo de amarelo

        int auxWidth = pic[2].width;
        int desiredWidth = 400, lowestEnergyPixel;

        //Criado para evitar modificar a máscara fornecida.
        RGB* maskAux = malloc(pic[1].width * pic[1].height * 3);

        for(int i = 0; i < pic[2].height * pic[2].width; i++)
            pic[2].img[i] = pic[0].img[i];



        /* for(int i = 0; i < pic[1].height * pic[1].width; i++) {
            maskAux[i].r = pic[1].img[i].r;
            maskAux[i].g = pic[1].img[i].g;
            maskAux[i].b = pic[1].img[i].b;


            if (maskAux[i].b != 255)
                printf("\n%d\t%d", maskAux[i].b, pic[1].img[i].b);
        } */

        while (auxWidth > desiredWidth) {

            int* energy = malloc(sizeof(int) * pic[2].height * pic[2].width);

            for (int i = 0; i < pic[2].height; i++) {

                for (int j = 0; j < pic[2].width; j++) {

                    //Define a energia inicial das células com base na máscara.
                    if (pic[1].img[(i * pic[2].width) + j].r > 200 && pic[1].img[(i * pic[2].width) + j].g < 100 && pic[1].img[(i * pic[2].width) + j].b < 100)
                        //printf("\nHello, World 1");
                        energy[(i * pic[2].width) + j] = -10000000;

                    else if (pic[1].img[(i * pic[2].width) + j].r < 100 && pic[1].img[(i * pic[2].width) + j].g > 200 && pic[1].img[(i * pic[2].width) + j].b < 100)
                        //printf("\nHello, World 2");
                        energy[(i * pic[2].width) + j] = 10000000;

                    else
                        energy[(i * pic[2].width) + j] = 0;

                    // Condição especial para a primeira linha da imagem, na qual não é aplicada a função accumulatedEnergy(), que soma a energia das células acima,
                    // e a "célula acima", para fins de cálculo da energia, é considerada a célula mais abaixo da mesma coluna.
                    if (i == 0) {

                        if (j == 0)
                            energy[0] += energyCalculator(pic[2].img[1], pic[2].img[auxWidth - 1], pic[2].img[pic[2].width * (pic[2].height - 1)], pic[2].img[pic[2].width]);

                        else if (j == auxWidth - 1)
                            energy[j] += energyCalculator(pic[2].img[auxWidth - 2], pic[2].img[0], pic[2].img[(pic[2].width * pic[2].height - 1) + auxWidth - 1], pic[2].img[(pic[2].width + auxWidth) - 1]);

                        else
                            energy[j] += energyCalculator(pic[2].img[j - 1], pic[2].img[j + 1], pic[2].img[j + (pic[2].width * (pic[2].height -1))], pic[2].img[j + pic[2].width]);

                    }

                    // Condição especial para a última linha da imagem, onde a "célula abaixo", para fins de cálculo da energia, é considerada a célula mais acima da mesma coluna.
                    else if ((i * pic[2].width) + j >= pic[2].width * (pic[2].height - 1)) {

                        if (j == 0) {

                            energy[i * pic[2].width] += energyCalculator(pic[2].img[(pic[2].width * i) + auxWidth - 1], pic[2].img[(i * pic[2].width) + j + 1], pic[2].img[pic[2].width * (i - 1)], pic[2].img[0]);
                            energy[i * pic[2].width] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);

                        }

                        else if (j == auxWidth - 1) {

                            energy[(i * pic[2].width) + j] += energyCalculator(pic[2].img[(pic[2].width * i) + auxWidth - 2], pic[2].img[pic[2].width * i], pic[2].img[(pic[2].width * (i - 1)) + auxWidth - 1], pic[2].img[j]);
                            energy[(i * pic[2].width) + j] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);

                        } else {

                            energy[(i * pic[2].width) + j] += energyCalculator(pic[2].img[(i * pic[2].width) + j - 1], pic[2].img[(i * pic[2].width) + j + 1], pic[2].img[((i - 1) * pic[2].width) + j], pic[2].img[j]);
                            energy[(i * pic[2].width) + j] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);

                        }


                    }

                    // Condição especial para a primeira coluna da imagem, onde a "célula à esquerda", para fins de cálculo da energia, é considerada a célula mais à direita da mesma linha.
                    else if (j == 0) {

                        energy[i * pic[2].width] += energyCalculator(pic[2].img[(i * pic[2].width) + auxWidth - 1], pic[2].img[(i * pic[2].width) + 1], pic[2].img[(i - 1) * pic[2].width], pic[2].img[(i + 1) * pic[2].width]);
                        energy[i * pic[2].width] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);;

                    }

                    // Condição especial para a última coluna da imagem, onde a "célula à direita", para fins de cálculo da energia, é considerada a célula mais à esquerda da mesma linha.
                    else if (j == auxWidth - 1) {

                        energy[(i * pic[2].width) + j] += energyCalculator(pic[2].img[(i * pic[2].width) + j - 1], pic[2].img[i * pic[2].width], pic[2].img[(i - 1) * pic[2].width + j], pic[2].img[(i + 1) * pic[2].width + j]);
                        energy[(i * pic[2].width) + j] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);;

                    }

                    // Condição padrão para cálculo da energia.
                    else {

                        energy[(i * pic[2].width) + j] += energyCalculator(pic[2].img[(i * pic[2].width) + j - 1], pic[2].img[(i * pic[2].width) + j + 1], pic[2].img[(i - 1) * pic[2].width + j], pic[2].img[(i + 1) * pic[2].width + j]);
                        energy[(i * pic[2].width) + j] += accumulatedEnergy(energy, pic[2].width, auxWidth, (i * pic[2].width) + j);;

                    }

                }


            }


            lowestEnergyPixel = pic[2].width * (pic[2].height - 1);

            // Determina o pixel de menor energia da última linha da imagem.
            for (int i = lowestEnergyPixel; i <  pic[2].width * (pic[2].height - 1) + auxWidth; i++) {

                if (energy[i] < energy[lowestEnergyPixel])
                    lowestEnergyPixel = i;

            }

            //printf("%d\n", lowestEnergyPixel);

            for (int i = lowestEnergyPixel; ; ) {

                int column = i % pic[2].width;

                for (int j = 0; j < auxWidth - column; j++) {

                    pic[2].img[i + j] = pic[2].img[i + j + 1];
                    pic[1].img[i + j] = pic[1].img[i + j + 1];

                }

                pic[2].img[(i - column) + auxWidth - 1].r = 0;
                pic[2].img[(i - column) + auxWidth - 1].g = 0;
                pic[2].img[(i - column) + auxWidth - 1].b = 0;

                if (i - pic[2].width < 0)
                    break;

                int lowestPath = i - pic[2].width;

                int lowestEnergy = energy[i - pic[2].width];

                if (i % pic[2].width != 0 && energy[i - pic[2].width - 1] < lowestEnergy) {

                    lowestEnergy = energy[i - pic[2].width - 1];
                    lowestPath = i - pic[2].width - 1;

                }

                if (i % pic[2].width != (auxWidth - 1) && energy[i - pic[2].width + 1] < lowestEnergy) {

                    lowestEnergy = energy[i - pic[2].width + 1];
                    lowestPath = i - pic[2].width + 1;

                }

                i = lowestPath;

            }

        free(energy);
        auxWidth--;

        }


        // Chame uploadTexture a cada vez que mudar
        // a imagem (pic[2])
        uploadTexture();
        free(maskAux);

    }
    glutPostRedisplay();
}

// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        pic[2].width, pic[2].height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, pic[2].img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Preto
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255);  // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(0,0);

    glTexCoord2f(1,0);
    glVertex2f(pic[sel].width,0);

    glTexCoord2f(1,1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0,1);
    glVertex2f(0,pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}

int energyCalculator(RGB previousPixel, RGB nextPixel, RGB topPixel, RGB lowerPixel) {

    return (previousPixel.r - nextPixel.r) * (previousPixel.r - nextPixel.r) + (previousPixel.g - nextPixel.g) * (previousPixel.g - nextPixel.g) + (previousPixel.b - nextPixel.b) * (previousPixel.b - nextPixel.b)
    + (topPixel.r - lowerPixel.r) * (topPixel.r - lowerPixel.r) + (topPixel.g - lowerPixel.g) * (topPixel.g - lowerPixel.g) + (topPixel.b - lowerPixel.b) * (topPixel.b - lowerPixel.b);
}

int accumulatedEnergy(int* energy, int maxWidth, int actualWidth, int i) {

    int accumulatedEnergy = energy[i - maxWidth];

    if (i % maxWidth != 0 && energy[i - maxWidth - 1] < accumulatedEnergy)
        accumulatedEnergy = energy[i - maxWidth - 1];

    if (i % maxWidth != (actualWidth - 1) && energy[i - maxWidth + 1] < accumulatedEnergy)
        accumulatedEnergy = energy[i - maxWidth + 1];

    return accumulatedEnergy;

}
