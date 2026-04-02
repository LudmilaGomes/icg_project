#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glut.h>

GLuint tex_chao;
GLuint tex_caminho;
GLuint tex_ceu;
GLuint tex_feno;
int largura = 1280;
int altura = 720;
float pos_x = 0.0;
float pos_y = 4.0;
float pos_z = 140.0;
float yaw = 0.0;
float dir_x = 0.0;
float dir_y = 0.0;
float dir_z = -1.0;
float t_bola = 0.0;

//Calculo da spline de bezier para trajetoria da bola de feno
void bezierFeno(float t, float *p0, float *p1, float *p2, float *p3, float *pos)
{
    float u = 1.0 - t;

    //Formulas para calculo da spline de bezier
    float b0 = pow(u, 3);
    float b1 = 3 * pow(u, 2) * t;
    float b2 = 3 * u * pow(t, 2);
    float b3 = pow(t, 3);

    pos[0] = b0*p0[0] + b1*p1[0] + b2*p2[0] + b3*p3[0]; //Coordenada x
    pos[1] = b0*p0[1] + b1*p1[1] + b2*p2[1] + b3*p3[1]; //Coordenada y
    pos[2] = b0*p0[2] + b1*p1[2] + b2*p2[2] + b3*p3[2]; //Coordenada z
}

//====================ESTRUTURA PARA MODELOS====================
typedef struct {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes;
    size_t num_shapes;
    tinyobj_material_t *materials;
    size_t num_materials;
    int pronto;
    GLuint displayList;
} Modelo3D;

//====================CARREGA MODELO====================
static void file_reader(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buffer, size_t *len)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", filename);
        *buffer = NULL;
        *len = 0;
        return;
    }

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer = (char *)malloc(*len + 1);
    if (*buffer)
    {
        fread(*buffer, 1, *len, f);
        (*buffer)[*len] = '\0';
    }

    fclose(f);
}

//Carrega as partes do modelo 3D importado
int carregaObjModelo(Modelo3D *modelo, const char *caminho_obj)
{
    modelo->attrib.vertices = NULL;
    modelo->attrib.num_vertices = 0;
    modelo->attrib.normals = NULL;
    modelo->attrib.num_normals = 0;
    modelo->attrib.texcoords = NULL;
    modelo->attrib.num_texcoords = 0;
    modelo->attrib.faces = NULL;
    modelo->attrib.num_faces = 0;
    modelo->attrib.face_num_verts = NULL;
    modelo->attrib.num_face_num_verts = 0;
    modelo->attrib.material_ids = NULL;

    modelo->shapes = NULL;
    modelo->num_shapes = 0;
    modelo->materials = NULL;
    modelo->num_materials = 0;
    modelo->pronto = 0;
    modelo->displayList = 0;

    int ret = tinyobj_parse_obj(&modelo->attrib, &modelo->shapes, &modelo->num_shapes,
                                &modelo->materials, &modelo->num_materials,
                                caminho_obj, file_reader, NULL,
                                TINYOBJ_FLAG_TRIANGULATE);

    if (ret == TINYOBJ_SUCCESS)
    {
        modelo->pronto = 1;
        printf("Modelo carregado com sucesso: %s\n", caminho_obj);
        return 0;
    }
    else
    {
        fprintf(stderr, "Falha ao carregar o modelo: %s\n", caminho_obj);
        return -1;
    }
}

//Constroi/renderiza o modelo de fato a partir de seus atributos carregados na funcao anterior
void renderizaModelo(Modelo3D *modelo)
{
    if (!modelo->pronto || modelo->attrib.material_ids == NULL)
        return;

    for (size_t s = 0; s < modelo->num_shapes; s++)
    {
        glBegin(GL_TRIANGLES);
        for (size_t f = 0; f < modelo->shapes[s].length; f++)
        {
            size_t global_face_id = modelo->shapes[s].face_offset + f;
            int matid = modelo->attrib.material_ids[global_face_id];

            if (matid >= 0 && matid < modelo->num_materials)
            {
                glColor3f(modelo->materials[matid].diffuse[0],
                          modelo->materials[matid].diffuse[1],
                          modelo->materials[matid].diffuse[2]);

                glMaterialfv(GL_FRONT, GL_AMBIENT, modelo->materials[matid].ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, modelo->materials[matid].diffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, modelo->materials[matid].specular);
                glMaterialf(GL_FRONT, GL_SHININESS, modelo->materials[matid].shininess);
            }

            for (size_t v = 0; v < 3; v++)
            {
                size_t face_vertex_id = (modelo->shapes[s].face_offset + f) * 3 + v;
                tinyobj_vertex_index_t id = modelo->attrib.faces[face_vertex_id];

                if (id.vn_idx >= 0)
                {
                    glNormal3f(modelo->attrib.normals[3 * id.vn_idx + 0],
                               modelo->attrib.normals[3 * id.vn_idx + 1],
                               modelo->attrib.normals[3 * id.vn_idx + 2]);
                }
                if (id.v_idx >= 0)
                {
                    glVertex3f(modelo->attrib.vertices[3 * id.v_idx + 0],
                               modelo->attrib.vertices[3 * id.v_idx + 1],
                               modelo->attrib.vertices[3 * id.v_idx + 2]);
                }
            }
        }
        glEnd();
    }
}

//Funcao para carregar as texturas 
GLuint loadTexture(const char *filename)
{
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height,
                                  &nrChannels, 0);
  if (data)
  {

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    // Load the texture data ( check if it 's RGB or RGBA )
    if (nrChannels == 3)
    {
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width,
                        height, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4)
    {
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width,
                        height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);

    return texture;
  }
  else
  {
    printf("Failed to load texture.\n");
    return -1;
  }
}

//====================CRIA DISPLAY LIST====================
//Grava o objeto ja renderizado na memoria do OpenGL, permitindo sua reutilizacao
//Em vez de recalcular vertices, normais e faces a cada frame, o modelo permanece pronto na memoria
//Assim, basta chama-lo sempre que necessario, o que melhora o desempenho da renderizacao
//Alem disso, permite criar multiplas instancias do mesmo objeto em diferentes posicoes da cena
void criaDisplayListModelo(Modelo3D *modelo)
{
    if (!modelo->pronto) return;

    modelo->displayList = glGenLists(1);
    glNewList(modelo->displayList, GL_COMPILE);
    renderizaModelo(modelo);
    glEndList();
}

//====================OBJETOS====================
void chao()
{
    glColor3f(0.8, 0.7, 0.5);
    glBindTexture(GL_TEXTURE_2D, tex_chao);
    float tam_x = 200.0;
    float tam_z = 200.0;
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_x, 0.0, -tam_z);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-tam_x, 0.0, tam_z);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(tam_x, 0.0, tam_z);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(tam_x, 0.0, -tam_z);
    glEnd();
}

void caminho()
{
    glColor3f(0.7, 0.6, 0.4);
    glBindTexture(GL_TEXTURE_2D, tex_caminho);
    float tam_x = 15.0;   //Largura
    float tam_z = 150.0;  //Comprimento
    float aux = 0.01;

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_x, aux, -tam_z);
    glTexCoord2f(0.0, 5.0);
    glVertex3f(-tam_x, aux, tam_z + 50.0);
    glTexCoord2f(1.0, 5.0);
    glVertex3f(tam_x, aux, tam_z + 50.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(tam_x, aux, -tam_z);
    glEnd();
}

void caminho2()
{
    glPushMatrix();
    glTranslatef(0.0, 0.0, -50.0); //Move para o final do caminho 1
    glBindTexture(GL_TEXTURE_2D, tex_caminho);
    glColor3f(0.7, 0.6, 0.4);

    float tam_x = 200.0;
    float tam_z = 15.0;
    float aux = 0.02;

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_x, aux, -tam_z);
    glTexCoord2f(0.0, 2.0);
    glVertex3f(-tam_x, aux, tam_z);
    glTexCoord2f(20.0, 2.0);
    glVertex3f(tam_x, aux, tam_z);
    glTexCoord2f(20.0, 0.0);
    glVertex3f(tam_x, aux, -tam_z);
    glEnd();

    glPopMatrix();
}

void paredesHorizonte()
{
    //glColor3f(0.0, 0.6, 1.0);
    glBindTexture(GL_TEXTURE_2D, tex_ceu);
    float tam_X = 200.0;
    float tam_y = 200.0;
    float tam_Z = 200.0;

    glBegin(GL_QUADS);
    //Fundo
    glNormal3f(0.0, 0.0, 1.0); //Normal apontando para frente
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-tam_X, 0.0, -tam_Z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(tam_X, 0.0, -tam_Z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(tam_X, tam_y, -tam_Z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_X, tam_y, -tam_Z);

    //Frente
    glNormal3f(0.0, 0.0, -1.0); //Normal apontando para trás
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_X, 0.0, tam_Z);
    glTexCoord2f(0.0, 0.5);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glTexCoord2f(0.5, 0.5);
    glVertex3f(tam_X, tam_y, tam_Z);
    glTexCoord2f(0.5, 0.0);
    glVertex3f(tam_X, 0.0, tam_Z);

    //Esquerda
    glNormal3f(1.0, 0.0, 0.0); //Normal apontando para direita

    glTexCoord2f(0.7, 0.7);
    glVertex3f(-tam_X, 0.0, -tam_Z);
    glTexCoord2f(0.7, 0.0);
    glVertex3f(-tam_X, tam_y, -tam_Z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glTexCoord2f(0.0, 0.7);
    glVertex3f(-tam_X, 0.0, tam_Z);

    //Direita
    glNormal3f(-1.0, 0.0, 0.0); //Normal apontando para esquerda
    glTexCoord2f(0.7, 0.7);
    glVertex3f(tam_X, 0.0, -tam_Z);
    glTexCoord2f(0.0, 0.7);
    glVertex3f(tam_X, 0.0, tam_Z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(tam_X, tam_y, tam_Z);
    glTexCoord2f(0.7, 0.0);
    glVertex3f(tam_X, tam_y, -tam_Z);

    //Teto
    glNormal3f(0.0, 1.0, 0.0); //Normal apontando para cima
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-tam_X, tam_y, -tam_Z);
    glTexCoord2f(0.20, 0.0);
    glVertex3f(tam_X, tam_y, -tam_Z);
    glTexCoord2f(0.20, 0.20);
    glVertex3f(tam_X, tam_y, tam_Z);
    glTexCoord2f(0.0, 0.20);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glEnd();
}

void bolaDeFeno()
{
    //Pontos de controle para spline de bezier
    float p0[3] = {-90.0, 0.0, -50.0};
    float p1[3] = {-30.0, 0.0, -55.0};
    float p2[3] = {30.0, 0.0, -45.0};
    float p3[3] = {90.0, 0.0, -50.0};

    float pos[3];

    bezierFeno(t_bola, p0, p1, p2, p3, pos);

    //Efeito de "quique"
    float freq_salto = 40;
    float altura_Salto = 1.2;
    //Descreve um movimento senoidal para dar o efeito de "quique" para a bola
    //fabs garante que seja retornado apenas o valor absoluto, evitando que a bola atravesse o solo
    float altura = fabs(sin(t_bola * freq_salto)) * altura_Salto;

    glPushMatrix();
    glTranslatef(pos[0], 1.0 + altura, pos[2]); //Atualiza a posicao da bola na cena a partir dos valores calculados
    //glColor3f(0.8, 0.9, 0.3);
    glBindTexture(GL_TEXTURE_2D, tex_feno);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere (quad, 1.2, 20, 20);
    glPopMatrix();
}

//Funcao usada para desenhar os objetos, sendo necessario passar a posicao no mundo, o modelo a ser construido
//sua angulacao e componentes da escala
//O modelo deve estar carregado e pronto na memoria para poder ser utilizado
void desenhaModeloInstancia(float x, float y, float z, Modelo3D *modelo, float angulo_y, float sx, float sy, float sz)
{
    if (!modelo->pronto || modelo->displayList == 0){
        return;
    }

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angulo_y, 0.0, 1.0, 0.0);
    glScalef(sx, sy, sz);
    glCallList(modelo->displayList);
    glPopMatrix();
}

//====================CIDADE====================
void cidade(Modelo3D *carruagem, Modelo3D *saloon, Modelo3D *casa1, Modelo3D *casa2, Modelo3D *general_store,
            Modelo3D *guns_store, Modelo3D *hotel, Modelo3D *bank, Modelo3D *sheriff, Modelo3D *casa_grande,
            Modelo3D *water_tank, Modelo3D *welcome_placa, Modelo3D *Windmill, Modelo3D *cactus_a, Modelo3D *cactus_b,
            Modelo3D *cactus_c, Modelo3D *cactus_d, Modelo3D *barrel_a, Modelo3D *barrel_b, Modelo3D *dynamite_b,
            Modelo3D *lantern)
{
    float aux = 0.01; //Usado para evitar z-fighting

    //OBJETOS CONSTRUIDOS COM FUNCOES DO PROPRIO OPENGL
    chao();
    caminho();
    caminho2();
    paredesHorizonte();
    bolaDeFeno();

    //AMBIENTACAO
    desenhaModeloInstancia(5.0, aux, 40.0, carruagem, 0.0, 2.2, 2.2, 2.2);
    desenhaModeloInstancia(0.0, aux, 110.0, welcome_placa, -90.0, 5.0, 5.0, 5.0);
    desenhaModeloInstancia(100.0, aux, 150.0, cactus_b, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-120.0, aux, -70.0, cactus_a, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(20.0, aux, 120.0, cactus_c, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-60.0, aux, 80.0, cactus_c, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-70.0, aux, 90.0, cactus_d, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-82.0, aux, 110.0, cactus_a, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(13.0, aux, 60.0, barrel_a, 90.0, 3.0, 8.0, 3.0);
    desenhaModeloInstancia(13.0, aux, 55.0, barrel_b, 90.0, 4.0, 8.0, 4.0);
    desenhaModeloInstancia(-50.0, aux, -90.0, dynamite_b, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-47.0, aux, -90.0, dynamite_b, 90.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-15.0, aux, 112.0, lantern, 120.0, 3.0, 3.0, 3.0);

    //ESQUERDA DA CIDADE
    desenhaModeloInstancia(-140.0, aux, 140.0, cactus_a, 0.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(-160.0, aux, 60.0, cactus_b, 0.0, 3.5, 3.5, 3.5);
    desenhaModeloInstancia(-130.0, aux, -20.0, cactus_c, 0.0, 2.8, 2.8, 2.8);
    desenhaModeloInstancia(-170.0, aux, -100.0, cactus_d, 0.0, 3.2, 3.2, 3.2);

    //DIREITA DA CIDADE
    desenhaModeloInstancia(140.0, aux, 130.0, cactus_b, 0.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(165.0, aux, 40.0, cactus_c, 0.0, 3.5, 3.5, 3.5);
    desenhaModeloInstancia(135.0, aux, -40.0, cactus_a, 0.0, 2.5, 2.5, 2.5);

    //ANTES DA CIDADE
    desenhaModeloInstancia(-100.0, aux, -170.0, cactus_d, 0.0, 3.0, 3.0, 3.0);
    desenhaModeloInstancia(100.0, aux, -180.0, cactus_a, 0.0, 3.0, 3.0, 3.0);

    //DE FRENTE AO SALOON
    desenhaModeloInstancia(-57.0, aux, -34.0, hotel, 90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-85.0, aux, -32.0, casa1, 90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(58.0, aux, -34.0, casa2, 90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(88.0, aux, -33.0, casa1, 90.0, 4.0, 4.0, 4.0);

    //AO LADO DO SALOON/DE FRENTRE PARA A RUA PRINCIPAL
    desenhaModeloInstancia(0.0, aux, -75.0, saloon, -90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(45.0, aux, -70.0, casa_grande, -90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(75.0, aux, -70.0, casa1, -90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-45.0, aux, -70.0, bank, -90.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-78.0, aux, -70.0, sheriff, -90.0, 4.0, 4.0, 4.0);

    //DIREITA DA RUA PRINCIPAL
    desenhaModeloInstancia(20.0, aux, 85.0, casa1, 180.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(20.0, aux, 57.5, casa2, 180.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(20.0, aux, 29.0, general_store, 180.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(20.0, aux, 0.5, guns_store, 180.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(20.0, aux, -27.5, casa1, 180.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(60.0, aux, 50.0, Windmill, 90.0, 5.0, 5.0, 5.0);

    //ESQUERDA DA RUA PRINCIPAL
    desenhaModeloInstancia(-20.0, aux, 85.0, casa1, 0.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-60.0, aux, 57.5, water_tank, 90.0, 5.0, 5.0, 5.0);
    desenhaModeloInstancia(-20.0, aux, 57.5, casa2, 0.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-20.0, aux, 29.0, casa1, 0.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-20.0, aux, 0.5, casa1, 0.0, 4.0, 4.0, 4.0);
    desenhaModeloInstancia(-20.0, aux, -27.5, casa2, 0.0, 4.0, 4.0, 4.0);
}

//====================FUNCOES GLUT====================
void init()
{
    GLfloat light0_ambient[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat light0_diffuse[] = {1.0, 0.97, 0.85, 1.0};
    GLfloat light0_specular[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    //Configuracoes para camera, a qual nao fica presa olhando para a origem
    //O uso de seno e cosseno permitem retornar um valor que representa a direcao que esta sendo olhada
    //Isso se baseia na variavel yaw, a qual e aumentada ou diminuida atraves das teclas Q e E
    //Isso faz com que dir_x e dir_z variem entre -1 e 1 de modo que, por exemplo:
    //Se yaw igual a 90, dir_x sera igual a 1 -> Olhando para a direita
    //Enquanto dir_z será 0 -> nao ha deslocamento, entao esta olhando completamente para a direita
    dir_x = sin(yaw); 
    dir_z = -cos(yaw);
    //No lookat, o uso de posicao + direcao faz com que a camera olhe sempre
    //para frente em relação a sua orientação atual, tornando o movimento mais fluido
    gluLookAt(pos_x, pos_y, pos_z, pos_x + dir_x, pos_y + dir_y, pos_z + dir_z, 0.0, 1.0, 0.0);

    //If utilizado para ajustar a altura ao entrar no saloon, dando o efeito de que o mesmo tem física
    if ((pos_x <= 20 && pos_x >= -20) && ((pos_z <= -65 && pos_z >= -98)))
    {
        pos_y = 7.0;
    }
    else
    {
        pos_y = 4.0;
    }
    
    GLfloat light0_position[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    //Esses extern Modelo3D sao necessarios para sua utilizacao como parametros na funcao cidade
    extern Modelo3D carruagem;
    extern Modelo3D saloon;
    extern Modelo3D hotel;
    extern Modelo3D guns_store;
    extern Modelo3D general_store;
    extern Modelo3D sheriff;
    extern Modelo3D bank;
    extern Modelo3D casa1;
    extern Modelo3D casa2;
    extern Modelo3D casa_grande;
    extern Modelo3D water_tank;
    extern Modelo3D welcome_placa;
    extern Modelo3D Windmill;
    extern Modelo3D cactus_a;
    extern Modelo3D cactus_b;
    extern Modelo3D cactus_c;
    extern Modelo3D cactus_d;
    extern Modelo3D barrel_a;
    extern Modelo3D barrel_b;
    extern Modelo3D dynamite_b;
    extern Modelo3D lantern;

    //Aqui na display basta chamar cidade(), todos os objetos sao instanciados la
    cidade(&carruagem, &saloon, &casa1, &casa2, &general_store, &guns_store, &hotel, &bank, &sheriff, &casa_grande,
           &water_tank, &welcome_placa, &Windmill, &cactus_a, &cactus_b, &cactus_c, &cactus_d, &barrel_a,
           &barrel_b, &dynamite_b, &lantern);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 800.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
    //Define a escala de deslocamento da camera ao se deslocar pelo ambiente
    //Basicamente, a velocidade de movimento
    float vel = 1.0; 

    switch (key)
    {
        case 'w':
            pos_x += dir_x * vel; //Uso de dir garante que a camera se desloque na direcao que a camera esta olhando
            pos_z += dir_z * vel;
            break;

        case 's':
            pos_x -= dir_x * vel;
            pos_z -= dir_z * vel;
            break;

        case 'a':
            pos_x += dir_z * vel;
            pos_z -= dir_x * vel;
            break;

        case 'd':
            pos_x -= dir_z * vel;
            pos_z += dir_x * vel;
            break;

        case 'q':
            yaw -= 0.1;
            break;

        case 'e':
            yaw += 0.1;
            break;

        case 27:
            exit(0);
            break;
    }

    glutPostRedisplay();
}

void velAnimacao(int value)
{
    t_bola += 0.002; //Ajusta a velocidade com a qual a bola de feno percorre a spline

    if (t_bola > 1.0){
        t_bola = 0.0; //Ao chegar no final da spline, reseta o movimento da bola
    }

    glutPostRedisplay();
    glutTimerFunc(16, velAnimacao, 0);
}

//Variáveis usadas para carregar os modelos
Modelo3D carruagem;
Modelo3D saloon;
Modelo3D hotel;
Modelo3D guns_store;
Modelo3D general_store;
Modelo3D sheriff;
Modelo3D bank;
Modelo3D casa1;
Modelo3D casa2;
Modelo3D casa_grande;
Modelo3D water_tank;
Modelo3D welcome_placa;
Modelo3D Windmill;
Modelo3D cactus_a;
Modelo3D cactus_b;
Modelo3D cactus_c;
Modelo3D cactus_d;
Modelo3D barrel_a;
Modelo3D barrel_b;
Modelo3D dynamite_b;
Modelo3D lantern;

void carregaModelos(){
    //Modelos sendo propriamente carregados a partir dos .obj
    carregaObjModelo(&carruagem, "predios/Stagecoach.obj");
    carregaObjModelo(&saloon, "predios/saloon_final.obj");
    carregaObjModelo(&hotel, "predios/hotel.obj");
    carregaObjModelo(&guns_store, "predios/guns_store.obj");
    carregaObjModelo(&general_store, "predios/general_store.obj");
    carregaObjModelo(&sheriff, "predios/sheriff.obj");
    carregaObjModelo(&bank, "predios/bank.obj");
    carregaObjModelo(&casa1, "predios/casa1.obj");
    carregaObjModelo(&casa2, "predios/casa2.obj");
    carregaObjModelo(&casa_grande, "predios/casa_grande.obj");
    carregaObjModelo(&water_tank, "predios/Water Tank.obj");
    carregaObjModelo(&welcome_placa, "predios/welcome_placa.obj");
    carregaObjModelo(&Windmill, "predios/Windmill.obj");
    carregaObjModelo(&cactus_a, "predios/Cactus A.obj");
    carregaObjModelo(&cactus_b, "predios/Cactus B.obj");
    carregaObjModelo(&cactus_c, "predios/Cactus C.obj");
    carregaObjModelo(&barrel_a, "predios/Barrel A.obj");
    carregaObjModelo(&barrel_b, "predios/Barrel B.obj");
    carregaObjModelo(&dynamite_b, "predios/Dynamite B.obj");
    carregaObjModelo(&lantern, "predios/Lantern.obj");

    //Display list armazena o modelo ja compilado pelo OpenGL (evita recalcular vertices a cada frame)
    //Melhora desempenho -> substitui varias chamadas glVertex/glNormal por um unico glCallList()
    //Permite criar multiplas instancias de um mesmo modelo ja carregado
    criaDisplayListModelo(&carruagem);
    criaDisplayListModelo(&saloon);
    criaDisplayListModelo(&hotel);
    criaDisplayListModelo(&guns_store);
    criaDisplayListModelo(&general_store);
    criaDisplayListModelo(&sheriff);
    criaDisplayListModelo(&bank);
    criaDisplayListModelo(&casa1);
    criaDisplayListModelo(&casa2);
    criaDisplayListModelo(&casa_grande);
    criaDisplayListModelo(&water_tank);
    criaDisplayListModelo(&welcome_placa);
    criaDisplayListModelo(&Windmill);
    criaDisplayListModelo(&cactus_a);
    criaDisplayListModelo(&cactus_b);
    criaDisplayListModelo(&cactus_c);
    criaDisplayListModelo(&cactus_d);
    criaDisplayListModelo(&barrel_a);
    criaDisplayListModelo(&barrel_b);
    criaDisplayListModelo(&dynamite_b);
    criaDisplayListModelo(&lantern);

    //Carrega texturas
    tex_chao = loadTexture("./texturas/terra_chao.png");
    if(tex_chao == -1)
        printf("Nao foi possivel carregar texturad o chao.\n");

    tex_caminho = loadTexture("./texturas/terra_caminho.png");
    if(tex_caminho == -1)
        printf("Nao foi possivel carregar texturad o caminho.\n");

    tex_ceu = loadTexture("./texturas/ceu_entardecer.png");
    if(tex_ceu == -1)
        printf("Nao foi possivel carregar texturad o ceu.\n");

    tex_feno = loadTexture("./texturas/feno.png");
    if(tex_feno == -1)
        printf("Nao foi possivel carregar texturad o ceu.\n");
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(largura, altura);
    glutCreateWindow(argv[0]);
    init();
    carregaModelos();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, velAnimacao, 0);
    glutMainLoop();

    return 0;
}
