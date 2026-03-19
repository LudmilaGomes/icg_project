#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"
#include <GL/glut.h>

int largura = 1280;
int altura = 720;
float pos_x = 0.0;
float pos_y = 10.0;
float pos_z = 100.0;

//====================CARREGA MODELO====================
tinyobj_attrib_t attrib;
tinyobj_shape_t *shapes = NULL;
size_t num_shapes;
tinyobj_material_t *materials = NULL;
size_t num_materials;
int modelo_pronto = 0;

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

    // aloca memória (tamanho +1 para o caractere nulo '\0' da string)
    *buffer = (char *)malloc(*len + 1);
    if (*buffer)
    {
        fread(*buffer, 1, *len, f);
        (*buffer)[*len] = '\0';
    }

    fclose(f);
}

int carregaObj()
{
    // inicializa campos
    attrib.vertices = NULL;
    attrib.num_vertices = 0;
    attrib.normals = NULL;
    attrib.num_normals = 0;
    attrib.texcoords = NULL;
    attrib.num_texcoords = 0;
    attrib.faces = NULL;
    attrib.num_faces = 0;
    attrib.face_num_verts = NULL;
    attrib.num_face_num_verts = 0;
    attrib.material_ids = NULL;

    // leitura de .obj e .mtl - tinyobjloader-c
    const char *caminho_obj = "My_stagecoach.obj";
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes,
                                &materials, &num_materials,
                                caminho_obj, file_reader, NULL,
                                TINYOBJ_FLAG_TRIANGULATE);

    if (ret == TINYOBJ_SUCCESS)
    {
        modelo_pronto = 1;
        return 0;
    }
    return -1;
}

void renderizaModeloCarregado()
{
    if (modelo_pronto != 1 || attrib.material_ids == NULL)
        return;

    // se houver hierarquia de partes do modelo, percorremos todos os componentes
    for (size_t s = 0; s < num_shapes; s++)
    {
        glBegin(GL_TRIANGLES);

        // percorremos todos os poligonos pertencentes ao shape s
        for (size_t f = 0; f < shapes[s].length; f++)
        {
            // offset para shape atual + n do poligono atual = indice 'global' do poligono atual no array
            size_t global_face_id = shapes[s].face_offset + f;
            // cada poligono tem um material correspondente
            int matid = attrib.material_ids[global_face_id];

            // verifica se matid eh valido
            if (matid >= 0 && matid < num_materials)
            {
                // usa iluminacao difusa armazenada em materials[matid].diffuse[0 - R, 1 - G, 2 - B]
                glColor3f(materials[matid].diffuse[0],
                          materials[matid].diffuse[1],
                          materials[matid].diffuse[2]);
            }
            else
            {
                printf("matid (ID do material) nao foi encontrado!\n");
                return;
            }

            // acessa os tres vertices do poligono atual
            for (size_t v = 0; v < 3; v++)
            {
                // indice para vertices do poligono atual
                size_t face_vertex_id = (shapes[s].face_offset + f) * 3 + v; // cada poligono tem tres vertices
                // attrib.faces guarda os indices de v (x, y, z), vn (x, y, z), vt (u, v) em id
                tinyobj_vertex_index_t id = attrib.faces[face_vertex_id];

                if (id.vn_idx >= 0)
                {
                    glNormal3f(attrib.normals[3 * id.vn_idx + 0],
                               attrib.normals[3 * id.vn_idx + 1],
                               attrib.normals[3 * id.vn_idx + 2]);
                }
                if (id.v_idx >= 0)
                {
                    glVertex3f(attrib.vertices[3 * id.v_idx + 0],
                               attrib.vertices[3 * id.v_idx + 1],
                               attrib.vertices[3 * id.v_idx + 2]);
                }
            }
        }
        glEnd();
    }
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

    float tam_x = 10.0;
    float tam_Z = 100.0;
    float aux = 0.01; // Improviso para evitar z-fighting com o chao

    glBegin(GL_QUADS);
    glVertex3f(-tam_x, aux, -tam_Z);
    glVertex3f(-tam_x, aux, tam_Z);
    glVertex3f(tam_x, aux, tam_Z);
    glVertex3f(tam_x, aux, -tam_Z);
    glEnd();
}

void paredesHorizonte()
{
    glColor3f(0.0, 0.6, 1.0);

    float tam_X = 100.0;
    float tam_y = 100.0;
    float tam_Z = 100.0;

    glBegin(GL_QUADS);

    // Referencia do ponto de vista -> como se estivesse de frente para a parede

    // Parede do fundo
    glVertex3f(-tam_X, 0.0, -tam_Z);   // inferior esquerdo
    glVertex3f(-tam_X, tam_y, -tam_Z); // superior esquerdo
    glVertex3f(tam_X, tam_y, -tam_Z);  // superior direito
    glVertex3f(tam_X, 0.0, -tam_Z);    // inferior direito

    // Parede da frente
    glVertex3f(-tam_X, 0.0, tam_Z);   // inferior esquerdo
    glVertex3f(-tam_X, tam_y, tam_Z); // superior esquerdo
    glVertex3f(tam_X, tam_y, tam_Z);  // superior direito
    glVertex3f(tam_X, 0.0, tam_Z);    // inferior direito

    // Parede da esq
    glVertex3f(-tam_X, 0.0, -tam_Z);   // inferior direito
    glVertex3f(-tam_X, tam_y, -tam_Z); // superior direito
    glVertex3f(-tam_X, tam_y, tam_Z);  // superior esquerdo
    glVertex3f(-tam_X, 0.0, tam_Z);    // inferior esquerdo

    // Parede da dir
    glVertex3f(tam_X, 0.0, -tam_Z);   // inferior esquerdo
    glVertex3f(tam_X, tam_y, -tam_Z); // superior esquerdo
    glVertex3f(tam_X, tam_y, tam_Z);  // superior direito
    glVertex3f(tam_X, 0.0, tam_Z);    // inferior direito

    // Teto
    glVertex3f(-tam_X, tam_y, tam_Z);  // esquerda inferior
    glVertex3f(-tam_X, tam_y, -tam_Z); // esqueda superior
    glVertex3f(tam_X, tam_y, tam_Z);   // direita inferior
    glVertex3f(tam_X, tam_y, -tam_Z);  // direita superior

    glEnd();
}

void saloon()
{
    // Numeros apenas para nao me perder nos push/pop matrix
    glPushMatrix(); // 1
    glTranslatef(0.0, 0.0, -30.0);

    glColor3f(0.5, 0.3, 0.1);

    glPushMatrix(); // 2
    glScalef(25.0, 28.0, 12.0);
    glutSolidCube(1.0);
    glPopMatrix(); // 2

    glPushMatrix();               // 3
    glTranslatef(0.0, 15.0, 6.1); // 6.1, o 0.1 é para evitar z-fighting
    glColor3f(0.7, 0.3, 0.1);

    glPushMatrix(); // 4
    glScalef(25.0, 8.0, 0.5);
    glutSolidCube(1.0);
    glPopMatrix(); // 4

    glPopMatrix(); // 3

    glPopMatrix(); // 1
}

void bolaDeFeno_e_NaoDePoeira() // (¬_¬)
{
    glPushMatrix();

    glTranslatef(8.0, 1.0, 60.0);
    glColor3f(0.8, 0.9, 0.3);
    glutSolidSphere(1.2, 20.0, 20.0);

    glPopMatrix();
}

void cubo(float x, float y, float z)
{ // Funcao para ajudar no desenho das casas
    glPushMatrix();
    glScalef(x, y, z);
    glutSolidCube(1.0);
    glPopMatrix();
}

void casa(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.0, z);
    glColor3f(0.5, 0.3, 0.2);
    cubo(15.0, 12.0, 12.0);

    glPushMatrix();
    glTranslatef(0.0, 7.0, 0.0);
    glColor3f(0.7, 0.5, 0.2);
    cubo(16.0, 3.0, 13.0);
    glPopMatrix();

    glPopMatrix();
}
//==============================================

// FUNCOES EM SI

void init()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(pos_x, pos_y, pos_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // Ainda ajustando o lookfrom_y para parecer a altura de uma pessoa

    chao();
    caminho();
    paredesHorizonte();
    saloon();

    glPushMatrix();
    glTranslatef(0.0, 0.0, 20.0);
    glScalef(3.0, 3.0, 3.0);
    renderizaModeloCarregado();
    glPopMatrix();

    bolaDeFeno_e_NaoDePoeira();
    casa(-25.0, 70.0);
    casa(25.0, 70.0);

    carregaObj("My_stagecoach.obj");

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 400.0); // Valores ajustados para se adequar a cena
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{

    switch (key)
    {
    case 'w':
        pos_z -= 1.0;
        break;
    case 'a':
        pos_x -= 1.0;
        break;
    case 's':
        pos_z += 1.0;
        break;
    case 'd':
        pos_x += 1.0;
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

int main(int argc, char **argv)
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(largura, altura);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, velAnimacao, 0);
    glutMainLoop();

    return 0;
}