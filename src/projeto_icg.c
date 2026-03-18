#include <GL/glut.h>

int largura = 1280;
int altura = 720;
float pos_x = 0.0;
float pos_y = 10.0;
float pos_z = 100.0;

//====================OBJETOS====================
void chao(){
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

void caminho(){
    glColor3f(0.7, 0.6, 0.4);

    float tam_x = 10.0;
    float tam_Z = 100.0;
    float aux = 0.01; //Improviso para evitar z-fighting com o chao

    glBegin(GL_QUADS);
    glVertex3f(-tam_x, aux, -tam_Z);
    glVertex3f(-tam_x, aux, tam_Z);
    glVertex3f(tam_x, aux, tam_Z);
    glVertex3f(tam_x, aux, -tam_Z);
    glEnd();
}

void paredesHorizonte(){
    glColor3f(0.0, 0.0, 1.0);

    float tam_X = 100.0;
    float tam_y = 100.0;
    float tam_Z = 100.0;

    glBegin(GL_QUADS);

    //Referencia do ponto de vista -> como se estivesse de frente para a parede

    //Parede do fundo
    glVertex3f(-tam_X, 0.0, -tam_Z); //inferior esquerdo
    glVertex3f(-tam_X, tam_y, -tam_Z); //superior esquerdo
    glVertex3f(tam_X, tam_y, -tam_Z); //superior direito
    glVertex3f(tam_X, 0.0, -tam_Z); //inferior direito

    //Parede da frente
    glVertex3f(-tam_X, 0.0, tam_Z); //inferior esquerdo
    glVertex3f(-tam_X, tam_y, tam_Z); //superior esquerdo
    glVertex3f(tam_X, tam_y, tam_Z); //superior direito
    glVertex3f(tam_X, 0.0, tam_Z); //inferior direito

    //Parede da esq
    glVertex3f(-tam_X, 0.0, -tam_Z); //inferior direito
    glVertex3f(-tam_X, tam_y, -tam_Z); //superior direito
    glVertex3f(-tam_X, tam_y, tam_Z); //superior esquerdo
    glVertex3f(-tam_X, 0.0, tam_Z); //inferior esquerdo

    //Parede da dir
    glVertex3f(tam_X, 0.0, -tam_Z); //inferior esquerdo
    glVertex3f(tam_X, tam_y, -tam_Z); //superior esquerdo
    glVertex3f(tam_X, tam_y, tam_Z); //superior direito
    glVertex3f(tam_X, 0.0, tam_Z); //inferior direito

    //Teto
    glVertex3f(-tam_X, tam_y, tam_Z); //esquerda inferior
    glVertex3f(-tam_X, tam_y, -tam_Z); //esqueda superior
    glVertex3f(tam_X, tam_y, tam_Z); //direita inferior
    glVertex3f(tam_X, tam_y, -tam_Z); //direita superior

    glEnd();
}

void saloon(){
    //Numeros apenas para nao me perder nos push/pop matrix
    glPushMatrix();//1
    glTranslatef(0.0, 0.0, -30.0);

    glColor3f(0.5, 0.3, 0.1);

    glPushMatrix();//2
    glScalef(25.0, 28.0, 12.0);
    glutSolidCube(1.0);
    glPopMatrix();//2

    glPushMatrix();//3
    glTranslatef(0.0, 15.0, 6.1); //6.1, o 0.1 é para evitar z-fighting
    glColor3f(0.7, 0.3, 0.1);

    glPushMatrix();//4
    glScalef(25.0, 8.0, 0.5);
    glutSolidCube(1.0);
    glPopMatrix();//4

    glPopMatrix();//3

    glPopMatrix();//1
}

void bolaDeFeno_e_NaoDePoeira(){
    glPushMatrix();

    glTranslatef(8.0, 1.0, 60.0);
    glColor3f(0.8, 0.9, 0.3);
    glutSolidSphere(1.2, 20.0, 20.0);

    glPopMatrix();
}

void cubo(float x, float y, float z){ //Funcao para ajudar no desenho das casas
    glPushMatrix();
    glScalef(x, y, z);
    glutSolidCube(1.0);
    glPopMatrix();
}

void casa(float x, float z) {
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

//FUNCOES EM SI

void init(){
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(pos_x, pos_y, pos_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //Ainda ajustando o lookfrom_y para parecer a altura de uma pessoa

    chao();
    caminho();
    paredesHorizonte();

    saloon();
    bolaDeFeno_e_NaoDePoeira();
    casa(-25.0, 70.0);
    casa(25.0, 70.0);

    glutSwapBuffers();
}

void reshape(int w, int h){
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 0.1, 400.0); //Valores ajustados para se adequar a cena
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y){

    switch (key) {
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

void velAnimacao(int value){
    glutPostRedisplay();
    glutTimerFunc(16, velAnimacao, 0);
}

int main(int argc, char** argv){

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