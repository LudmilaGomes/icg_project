#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"
#include <GL/glut.h>

int largura = 1280;
int altura = 720;
float pos_x = 0.0;
float pos_y = 4.0;
float pos_z = 100.0;
float yaw = 0.0f;
float dir_x = 0.0f;
float dir_y = 0.0f;
float dir_z = -1.0f;

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

//====================RENDERIZA MODELO====================
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

//====================CRIA DISPLAY LIST====================
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
    float tam_x = 100.0;
    float tam_z = 100.0;
    glBegin(GL_QUADS);
    glVertex3f(-tam_x, 0.0, -tam_z);
    glVertex3f(-tam_x, 0.0, tam_z);
    glVertex3f(tam_x, 0.0, tam_z);
    glVertex3f(tam_x, 0.0, -tam_z);
    glEnd();
}

void caminho()
{
    glColor3f(0.7, 0.6, 0.4);
    float tam_x = 15.0;   //Largura
    float tam_z = 50.0;   //Comprimento
    float aux = 0.01;
    glBegin(GL_QUADS);
    glVertex3f(-tam_x, aux, -tam_z);
    glVertex3f(-tam_x, aux, tam_z + 50.0);
    glVertex3f(tam_x, aux, tam_z + 50.0);
    glVertex3f(tam_x, aux, -tam_z);
    glEnd();
}

void caminho2()
{
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -50.0f); //Move para o final do caminho 1
    glColor3f(0.7, 0.6, 0.4);

    float tam_x = 100.0;
    float tam_z = 15.0;
    float aux = 0.01;

    glBegin(GL_QUADS);
    glVertex3f(-tam_x, aux, -tam_z);
    glVertex3f(-tam_x, aux, tam_z);
    glVertex3f(tam_x, aux, tam_z);
    glVertex3f(tam_x, aux, -tam_z);
    glEnd();

    glPopMatrix();
}

void paredesHorizonte()
{
    glColor3f(0.0, 0.6, 1.0);
    float tam_X = 100.0;
    float tam_y = 100.0;
    float tam_Z = 100.0;

    glBegin(GL_QUADS);
    //Fundo
    glNormal3f(0.0, 0.0, 1.0); //Normal apontando para frente
    glVertex3f(-tam_X, 0.0, -tam_Z);
    glVertex3f(tam_X, 0.0, -tam_Z);
    glVertex3f(tam_X, tam_y, -tam_Z);
    glVertex3f(-tam_X, tam_y, -tam_Z);

    //Frente
    glNormal3f(0.0, 0.0, -1.0); //Normal apontando para trás
    glVertex3f(-tam_X, 0.0, tam_Z);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glVertex3f(tam_X, tam_y, tam_Z);
    glVertex3f(tam_X, 0.0, tam_Z);

    //Esquerda
    glNormal3f(1.0, 0.0, 0.0); //Normal apontando para direita
    glVertex3f(-tam_X, 0.0, -tam_Z);
    glVertex3f(-tam_X, tam_y, -tam_Z);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glVertex3f(-tam_X, 0.0, tam_Z);

    //Direita
    glNormal3f(-1.0, 0.0, 0.0); //Normal apontando para esquerda
    glVertex3f(tam_X, 0.0, -tam_Z);
    glVertex3f(tam_X, 0.0, tam_Z);
    glVertex3f(tam_X, tam_y, tam_Z);
    glVertex3f(tam_X, tam_y, -tam_Z);

    //Teto
    glNormal3f(0.0, 1.0, 0.0); //Normal apontando para cima
    glVertex3f(-tam_X, tam_y, -tam_Z);
    glVertex3f(tam_X, tam_y, -tam_Z);
    glVertex3f(tam_X, tam_y, tam_Z);
    glVertex3f(-tam_X, tam_y, tam_Z);
    glEnd();
}

void bolaDeFeno_e_NaoDePoeira()
{
    glPushMatrix();
    glTranslatef(8.0, 1.0, -45.0);
    glColor3f(0.8, 0.9, 0.3);
    glutSolidSphere(1.2, 20.0, 20.0);
    glPopMatrix();
}

//Funcao usada para desenhar os objetos, sendo necessario passar a posicao no mundo, o modelo a ser construido
//Sua angulacao e componentes da escala
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
            Modelo3D *guns_store, Modelo3D *hotel, Modelo3D *bank, Modelo3D *sheriff)
{
    chao();
    caminho();
    caminho2();
    paredesHorizonte();
    bolaDeFeno_e_NaoDePoeira();

    desenhaModeloInstancia(5.0, 0.01, 40.0, carruagem, 0.0, 2.2, 2.2, 2.2);

    //DE FRENTE AO SALOON
    desenhaModeloInstancia(-57.0, 0.01, -34.0, hotel, 90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-85.0, 0.01, -32.0, casa1, 90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(58.0, 0.01, -34.0, casa2, 90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(88.0, 0.01, -33.0, casa1, 90.0, 4.0, 4.0, 4.0);

    //AO LADO DO SALOON/DE FRENTRE PARA A RUA PRINCIPAL
    desenhaModeloInstancia(0.0, 0.01, -75.0, saloon, -90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(45.0, 0.01, -70.0, hotel, -90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(75.0, 0.01, -70.0, casa1, -90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-45.0, 0.01, -70.0, bank, -90.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-78.0, 0.01, -70.0, sheriff, -90.0, 4.0, 4.0, 4.0);

    //DIREITA DA RUA PRINCIPAL
    desenhaModeloInstancia(20.0, 0.01, 85.0, casa1, 180.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(20.0, 0.01, 57.5, casa2, 180.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(20.0, 0.01, 29.0, general_store, 180.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(20.0, 0.01, 0.5, guns_store, 180.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(20.0, 0.01, -27.5, casa1, 180.0, 4.0, 4.0, 4.0);

    //ESQUERDA DA RUA PRINCIPAL
    desenhaModeloInstancia(-20.0, 0.01, 85.0, casa1, 0.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-20.0, 0.01, 57.5, casa2, 0.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-20.0, 0.01, 29.0, casa1, 0.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-20.0, 0.01, 0.5, casa1, 0.0, 4.0, 4.0, 4.0);

    desenhaModeloInstancia(-20.0, 0.01, -27.5, casa2, 0.0, 4.0, 4.0, 4.0);
}
//====================FUNCOES GLUT====================
void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    //Configuracoes para camera que nao fica presa olhando para a origem (mais proximo do que estamos acostumado em jogos)
    dir_x = sin(yaw);
    dir_z = -cos(yaw);
    gluLookAt(pos_x, pos_y, pos_z, pos_x + dir_x, pos_y + dir_y, pos_z + dir_z, 0.0, 1.0, 0.0);

    //Esses extern Modelo3D sao necessarios para poder passa-los como parametros
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

    //Aqui na display basta chamar cidade(), todos os objetos sao instanciados la
    cidade(&carruagem, &saloon, &casa1, &casa2, &general_store, &guns_store, &hotel, &bank, &sheriff);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 400.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
    float vel = 1.0;

    switch (key)
    {
        case 'w':
            pos_x += dir_x * vel;
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
    glutPostRedisplay();
    glutTimerFunc(16, velAnimacao, 0);
}

//====================MAIN====================

//Necessário cria a variável aqui para carregar o modelo
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

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(largura, altura);
    glutCreateWindow(argv[0]);
    init();

    //Modelos devem ser carregados aqui
    carregaObjModelo(&carruagem, "My_stagecoach.obj");
    carregaObjModelo(&saloon, "saloon.obj");
    carregaObjModelo(&hotel, "hotel.obj");
    carregaObjModelo(&guns_store, "guns_store.obj");
    carregaObjModelo(&general_store, "general_store.obj");
    carregaObjModelo(&sheriff, "sheriff.obj");
    carregaObjModelo(&bank, "bank.obj");
    carregaObjModelo(&casa1, "casa1.obj");
    carregaObjModelo(&casa2, "casa2.obj");
    carregaObjModelo(&casa_grande, "casa_grande.obj");

    //Display list armazena o modelo já compilado pelo OpenGL (evita recalcular vértices a cada frame)
    //Melhora desempenho -> substitui várias chamadas glVertex/glNormal por um único glCallList()
    //Permite criar múltiplas instâncias de um mesmo modelo já carregado
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

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, velAnimacao, 0);
    glutMainLoop();

    return 0;
}