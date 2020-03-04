#include "stack.h"
#include "draw.h"
#include "move.h"
#include "algorithm.h"
#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600

//Callback functions
static void on_display(void);
static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);

void init();
void set_light();
void restart();
void undo_move();
int animation_active();

void show_message();
//Contains message that will be shown on screen;
char message[MAX_SIZE];


int main(int argc, char **argv) {

    //Initializing glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    //Creating window
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(400, 50);
    glutCreateWindow("Towers of Hanoi");

    //Initializing callback functions
    glutDisplayFunc(on_display);
    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);

    //Initializing openGL
    glClearColor(0.1, 0.1, 0.1, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    //Resolving disk number
    //If the player enters custom number of disks
    if(argc > 1) {
        int n = atoi(argv[1]);
        if(n < 1 || n > MAX_DISK_NUM) {
            fprintf(stderr, "Number of disks must be between 1 and %d\n", MAX_DISK_NUM);
            exit(EXIT_FAILURE);
        }
        else {
            DISK_NUM = n;
        }
    }
    else //By default, number of disks is 6
        DISK_NUM = 6;

    init();
    glutMainLoop();

    return 0;
}

//Initialization of the variables
void init() {

    TOWER_HEIGHT = DISK_NUM * DISK_HEIGHT + 0.75;
    initialize_stack();

    //Initializing move variables
    speed = 0.1; //Animation speed
    move_ongoing = 0; //Initialy, disks are not moving
    moving_up = 1; //First, disk is moving up
    moving_down = 0;
    moving_side = 0;
    move_done = 0;
    move_count = 0;
    add_xpos = 0; //x position increment while moving disk, initialy 0

    //Initial values for disk rotation
    rotation = 180;
    rotation_parameter = 0;

    //Initializing Superman move variables
    superman_xpos = 0; //x position increment for Superman translation
    superman_ypos = 0; //y position increment for Superman translation
    left_arm_rotation = 0; //Rotation angle for left arm
    superman_active = 0;

    hanoi_active = 0; //Automatic solving is not active

    message[0] = '\0';

    //Loading textures
    load_background();
    load_platform_tex();
    load_logo_tex();

}

void restart() {
    init();
    glutPostRedisplay();
}

static void on_display(void) {

    //Clearing buffers
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Positioning view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        0, 1, 11,
        0, 0, 0,
        0, 1, 0
    );

    set_light();
    draw_background(); //Drawing background
    draw_towers(); //Drawing towers
    draw_disks(); //Drawing disks
    draw_superman(); //Drawing Superman

    //Write message on the screen
    show_message();

    //Sending buffer to display
    glutSwapBuffers();
}

void set_light(){

    //Light position
    GLfloat light_position_0[] = { 20, 20, 20, 0 };
    GLfloat light_position_1[] = {-20, 20, 20, 0};
    //Ambient light
    GLfloat ambient_light[] = { 0.1, 0.1, 0.1, 1 };
    //Diffuse light
    GLfloat diffuse_light[] = { 0.5, 0.5, 0.5, 1 };
    //Specular light
    GLfloat specular_light[] = {0.2, 0.2, 0.2, 1 };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position_0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position_1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular_light);

}

static void on_keyboard(unsigned char key, int x, int y) {

    switch (key) {
        case 27: //ESC
            glDeleteTextures(1, &bg_tex);
            glDeleteTextures(1, &platform_tex);
            glDeleteTextures(1, &logo_tex);
            exit(0);
            break;
        //Restart
        case 'R':
        case 'r':
            move_ongoing = 0;
            superman_active = 0;
            restart();
            break;
        //Stop automatic solving
        case 'q':
        case 'Q':
            hanoi_active = 0;
            break;
        //Activate puzzle solving by algorithm
        case 'h':
        case 'H':
            if(!hanoi_active) {
                restart();
                hanoi_solve();
            }
            break;

        //Move from A to B
        case 'a':
            if (!animation_active()) {
                src = &A;
                dest = &B;
                initialize_move();
            }
            break;
        //Move from B to A
        case 'A':
            if (!animation_active()) {
                src = &B;
                dest = &A;
                initialize_move();
            }
            break;
        //Move from A to C
        case 's':
            if (!animation_active()) {
                src = &A;
                dest = &C;
                initialize_move();
            }
            break;
        //Move from C to A
        case 'S':
            if (!animation_active()) {
                src = &C;
                dest = &A;
                initialize_move();
            }
            break;
        //Move from B to C
        case 'd':
            if (!animation_active()) {
                src = &B;
                dest = &C;
                initialize_move();
            }
            break;
        //Move from C to B
        case 'D':
            if (!animation_active()) {
                src = &C;
                dest = &B;
                initialize_move();
            }
            break;
        //Undo move
        case 'u':
        case 'U':
            if (!animation_active() && move_count > 0) {
                undo_move();
            }
            break;
    }
}

static void on_reshape(int w, int h) {

    //Setting viewport
    glViewport(0, 0, w, h);
    //Setting projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float) w / h, 1, 15);
}

void show_message() {
    char *c;

    //Writing message on screen
    set_material('b');
    glPushMatrix();
        glRasterPos3f(-4.2, 4.9, 0);

        for (c=message; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    glPopMatrix();

    //Writing tower names on screen
    char letters[] = {'A', 'B', 'C', '\0'};
    float distance = -1.45;
    set_material('d');
    
    glPushMatrix();
        for (c=letters; *c != '\0'; c++){
            glRasterPos3f(distance, -0.05, 5);
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
            distance += 1.4;
        }
    glPopMatrix();
}

void undo_move() {
    //Swap destination and source towers
    Stack *tmp = dest;
    dest = src;
    src = tmp;
    move_count -= 2; // 1 for the wrong move and 1 for correction
    initialize_move();
}

//Check if any animation is active
int animation_active() {
    return hanoi_active || move_ongoing || superman_active;
}
