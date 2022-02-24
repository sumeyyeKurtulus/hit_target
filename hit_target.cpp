/*********
CTIS164 - Template Source Program
----------
STUDENT : Sumeyye Kurtulus
SECTION : 02
HOMEWORK: 03
----------
PROBLEMS: Pressing F1, a new game does not start since it is just paused.
----------
ADDITIONAL FEATURES: 1. The ballons are generated at a random location, with a random colour
					 2. Counts the score relative to the hit object:  if a ballon is hit -> the score is incremented by 10
																	  if the bee is hit -> the score is incremented by 15
																	  if a cloud is hit -> the score is incremented by 20
																						   (you have to hit the balloon in the
																						    middle)
					 3. The clouds are also target which are sizable: pressing s -> smaller
																	  pressing b -> bigger
					 4. If you reach exact 100 points, 'WOW' message is displayed
					 5. If you reach exact 200 points, 'Congrats' message is displayed
					 6. If you press F5, the game is terminated and the score is lost. If you press F2 afterwards, a new game starts and new score counting, too. 
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

#define TIMER_PERIOD  13 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false, spacebar = false;
int  winWidth, winHeight; // current Window width and height

//variables holding the state of the game
bool resume = false;
#define START 0
#define RESUME 1
#define OVER 2
int state = START;

//structures for managing the targets and the weapon
typedef struct{
	float x, y;
}point_t;

typedef struct{
	float r, g, b;
}color_t;

typedef struct{
	point_t pos;
	double angle;
}weapon_t;

typedef struct{
	point_t pos;
	double angle;
	bool active;
}fire_t;

typedef struct{
	point_t pos;
	double angle;
	float radius;
	color_t color;
}baloon_t;

typedef struct{
	point_t pos;
	double angle;
	float r1, r2, r3;
	bool hit;
}cloud_t;

typedef struct{
	point_t pos;
	float angle;
	bool hit;
}bee_t;

float A = 100, fq = 1, C = 0, B = 0;
bee_t bee = { { 0, 0 }, 0, false };

float func(float x){
	return A * sin(fq * (x + C) * D2R) + B;
}

weapon_t weapon = { { -350, -350 }, 90 };
baloon_t baloons;  
cloud_t cloud = { { -350, 350 }, 90, 20, 25, 20, false};

#define B_RADIUS 30
#define MAX 30
#define FIRE_FREQ 10
float fire_freq = 0;
fire_t fire[MAX];

int score = 0;


//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void *font, const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void transform(point_t original, point_t transformed, double angle){
	double xp = original.x *cos(angle) - original.y * sin(angle) + transformed.x;
	double yp = original.x *sin(angle) + original.y * cos(angle) + transformed.y;
	glVertex2d(xp, yp);
}

void designBG(){

	glColor3ub(32, 97, 125);
	glBegin(GL_QUADS);
	glVertex2f(-winWidth / 2, winHeight / 2);
	glVertex2f(winWidth / 2, winHeight / 2);
	glColor3ub(132, 194, 220);
	glVertex2f(winWidth / 2, -winHeight / 2);
	glVertex2f(-winWidth / 2, -winHeight / 2);
	glEnd();

}

void designWeapon(weapon_t w){

	float x = w.pos.x;
	float y = w.pos.y;

	glColor4f(26.0 / 255.0, 102.0 / 255.0, 59.0 / 255.0, 0.5);
	glBegin(GL_QUADS);
	glVertex2f(x - 20, y + 20);
	glVertex2f(x + 20, y + 20);
	glVertex2f(x + 50, y - 30);
	glVertex2f(x - 50, y - 30);
	glEnd();

	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x - 20, y + 20);
	glVertex2f(x + 20, y + 20);
	glVertex2f(x + 50, y - 30);
	glVertex2f(x - 50, y - 30);
	glEnd();

	glColor3f(0, 0, 0);
	circle(x - 30, y - 33, 10);
	glColor3ub(205, 197, 148);
	circle(x - 30, y - 33, 5);

	glColor3f(0, 0, 0);
	circle(x + 30, y - 33, 10);
	glColor3ub(205, 197, 148);
	circle(x + 30, y - 33, 5);

	glColor3ub(164, 30, 30);
	glBegin(GL_TRIANGLES);
	transform({ 0, 0 }, w.pos, w.angle*D2R);
	transform({ 40, -20 }, w.pos, w.angle*D2R);
	transform({ 40, 20 }, w.pos, w.angle*D2R);
	glEnd();

	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	transform({ 0, 0 }, w.pos, w.angle*D2R);
	transform({ 40, -20 }, w.pos, w.angle*D2R);
	transform({ 40, 20 }, w.pos, w.angle*D2R);
	glEnd();

	glColor3f(0, 0, 0);
	circle(x, y, 8);
	glColor3ub(205, 197, 148);
	circle(x, y, 4);

}

void pickColor(){

	baloons.color.r = rand() % 256;
	baloons.color.g = rand() % 256;
	baloons.color.b = rand() % 256;
}

void designFires(){

	for (int i = 0; i < MAX; i++){
		
		float x = fire[i].pos.x;
		float y = fire[i].pos.y;

		if (fire[i].active && !resume){
			glColor3ub(172, 117, 146);
			glBegin(GL_TRIANGLES);
			glVertex2f(x, y + 15);
			glVertex2f(x - 15, y - 7);
			glVertex2f(x + 15, y - 7);

			glVertex2f(x - 15, y + 7);
			glVertex2f(x + 15, y + 7);
			glVertex2f(x, y - 15);
			glEnd();

			glColor3ub(0, 0, 0);
			glBegin(GL_LINE_LOOP);
			glVertex2f(x, y + 15);
			glVertex2f(x + 5, y + 7);
			glVertex2f(x + 15, y + 7);
			glVertex2f(x + 8, y);
			glVertex2f(x + 15, y - 7);
			glVertex2f(x + 5, y - 7);
			glVertex2f(x, y - 15);
			glVertex2f(x - 5, y - 7);
			glVertex2f(x - 15, y - 7);
			glVertex2f(x - 8, y);
			glVertex2f(x - 15, y + 7);
			glVertex2f(x - 5, y + 7);
			glEnd();

		}
		
	}

}

void desingBallons(baloon_t b){
	
	if (!resume){
		glColor4f(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 0.7);
		glBegin(GL_LINES);
		glVertex2f(b.pos.x, b.pos.y - 35);
		glVertex2f(b.pos.x, b.pos.y - 70);
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex2f(b.pos.x, b.pos.y - 25);
		glVertex2f(b.pos.x - 10, b.pos.y - 35);
		glVertex2f(b.pos.x + 10, b.pos.y - 35);
		glEnd();

		glColor4f(b.color.r / 255.0, b.color.g / 255.0, b.color.b / 255.0, 0.7);
		circle(b.pos.x, b.pos.y, b.radius);
		glColor4f(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 0.7);
		circle(b.pos.x - 10, b.pos.y - 10, 8);
	}
	
	
}

void harmonicBee(){
	float h = fabs(A);
	bee.pos.x = h * cos(bee.angle * D2R);
	bee.pos.y = h * sin(bee.angle * D2R);

	glColor4f(255.0 / 255.0, 230.0 / 255.0, 0.0 / 255.0, 0.7);
	circle(bee.pos.x, bee.pos.y, 20);
	glColor4f(215.0 / 255.0, 199.0 / 255.0, 0.0 / 55.0, 0.7);
	circle(bee.pos.x + 18, bee.pos.y + 13, 18);

	glColor3f(0, 0, 0);
	circle(bee.pos.x + 25, bee.pos.y + 15, 4);
	glColor3ub(247, 242, 190);
	circle(bee.pos.x + 25, bee.pos.y + 15, 2);

	glColor3f(0, 0, 0);
	circle(bee.pos.x + 23, bee.pos.y + 33, 3);
	circle(bee.pos.x + 23, bee.pos.y + 36, 2);
	circle(bee.pos.x + 15, bee.pos.y + 33, 3);
	circle(bee.pos.x + 15, bee.pos.y + 36, 2);

	circle(bee.pos.x - 20, bee.pos.y, 4);
	circle(bee.pos.x - 24, bee.pos.y, 3);
	circle(bee.pos.x - 27, bee.pos.y, 2);
	circle(bee.pos.x - 29, bee.pos.y, 1);

	glColor3f(0, 0, 0);
	circle_wire(bee.pos.x, bee.pos.y, 20);
	circle_wire(bee.pos.x + 18, bee.pos.y + 13, 18);

}

void designClouds(cloud_t c){
	
	glColor3f(1, 1, 1);
	circle(c.pos.x, c.pos.y, c.r1);
	circle(c.pos.x + 25, c.pos.y, c.r2);
	circle(c.pos.x + 50, c.pos.y, c.r3);

}

void displayWow(){

	glColor3ub(32, 23, 95);
	glRectf(-50, 50, 50, -50);

	glColor3ub(161, 27, 76);
	glRectf(-40, 40, 40, -40);

	glColor3ub(32, 23, 95);
	glRectf(-30, 30, 30, -30);

	glColor3ub(161, 27, 76);
	glRectf(-20, 20, 20, -20);

	glColor3ub(32, 23, 95);
	vprint2(-15, 0, 0.1, "WOW");
}

void displayCongrats(){

	glColor3ub(111, 23, 55);
	glRectf(-50, 50, 50, -50);

	glColor3ub(203, 119, 150);
	glRectf(-40, 40, 40, -40);

	glColor3ub(111, 23, 55);
	glRectf(-30, 30, 30, -30);

	glColor3ub(203, 119, 150);
	vprint2(-25, 0, 0.08, "CONGRATS");
}

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	designBG();
	designWeapon(weapon);

	if (state == START){

		if (!resume){

			designFires();
			desingBallons(baloons);

			if (!bee.hit){
				harmonicBee();
			}

			designClouds(cloud);

			vprint(-380, 340, GLUT_BITMAP_8_BY_13, "<F1: pause / restart>");
			vprint(-380, 320, GLUT_BITMAP_8_BY_13, "<F5: terminate>");
			vprint(-380, 300, GLUT_BITMAP_9_BY_15, "Score: %d", score);

			if (score == 100)
				displayWow();

			if (score == 200)
				displayCongrats();

		}
		else{
			glColor3ub(51, 0, 51);
			vprint2(-210, 0, 0.35, "==GAME RESUMED==");
			vprint2(-155, -50, 0.25, "Your current score: %d", score);
			vprint2(-140, -100, 0.20, "<Press F1 to restart>");
		}

	}
	else if (state == OVER){

		designBG();
		designWeapon(weapon);

		glColor3ub(51, 0, 51);
		vprint2(-200, 0, 0.35, "==GAME OVER==");
		vprint2(-220, -50, 0.20, "<Press F2 to start a new game>");

	}
	

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = true;

	if (key == 's'){
		if (cloud.r1 > 13 && cloud.r2 > 18 && cloud.r3 > 13){
			cloud.r1 -= 4;
			cloud.r2 -= 5;
			cloud.r3 -= 4;
		}
	}

	if (key == 'b'){
		if (cloud.r1 < 23 && cloud.r2 < 28 && cloud.r3 < 23){
			cloud.r1 += 4;
			cloud.r2 += 5;
			cloud.r3 += 4;
		}
	}

	

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = false;
	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	}

	if (key == GLUT_KEY_F1){
		resume = !resume;
	}
	
	if (key == GLUT_KEY_F5){
		state = OVER;
		score = 0;
	}

	if (key == GLUT_KEY_F2){
		state = START;
		bee.hit = false;
	}
		


	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	//glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.



	// to refresh the window it calls display() function
	//glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	//glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	//glutPostRedisplay();
}

void turnWeapon(weapon_t *p, double turn) {
	p->angle += turn;
	if (p->angle < 0) p->angle += 360;
	if (p->angle >= 360) p->angle -= 360;
}

void moveWeapon(weapon_t *p, double move){
	p->pos.x += move;
}

void refreshBaloon(){
	float yPos = rand() % 350;
	baloons.pos = { -350, yPos };
	pickColor();
	baloons.radius = B_RADIUS;
}

void refreshCloud(){
	cloud.pos = { -350, 350 };
}

bool ifHits(fire_t f, baloon_t b){
	float dx = b.pos.x - f.pos.x;
	float dy = b.pos.y - f.pos.y;
	float dist = sqrt(pow(dx, 2) + pow(dy, 2));

	if (dist <= b.radius)
		return true;
	else
		return false;
}

bool ifHitsBee(fire_t f, bee_t b){
	float dx = b.pos.x - f.pos.x;
	float dy = b.pos.y - f.pos.y;
	float dist = sqrt(pow(dx, 2) + pow(dy, 2));

	if (dist <= 20)
		return true;
	else
		return false;
}

bool ifHitsCloud(fire_t f, cloud_t c){
	float dx = c.pos.x - f.pos.x;
	float dy = c.pos.y - f.pos.y;
	float dist = sqrt(pow(dx, 2) + pow(dy, 2));

	if (dist <= c.r2)
		return true;
	else
		return false;
}

int doesFireExist(){
	for (int i = 0; i < MAX; i++){
		if (fire[i].active == false)
			return i;
	}
	return -1;
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	if (up)
		turnWeapon(&weapon, 3);

	if (down)
		turnWeapon(&weapon, -3);

	if (right && weapon.pos.x < 350)
		moveWeapon(&weapon, 3);

	if (left && weapon.pos.x > -350)
		moveWeapon(&weapon, -3);


	if (spacebar && fire_freq == 0){
		int ready = doesFireExist();
		if (ready != -1){
			fire[ready].pos = weapon.pos;
			fire[ready].angle = weapon.angle;
			fire_freq = FIRE_FREQ;
			fire[ready].active = true;
		}
		
	}

	if (fire_freq > 0)
		fire_freq--;

	for (int i = 0; i < MAX; i++){
	
		if (fire[i].active){
			fire[i].pos.x += 10 * cos(fire[i].angle * D2R);
			fire[i].pos.y += 10 * sin(fire[i].angle * D2R);

			if (fire[i].pos.x > 400 || fire[i].pos.y > 400 ||
				fire[i].pos.x < -400 || fire[i].pos.y < -400){
				fire[i].active = false;
			}

			if (ifHits(fire[i], baloons)){
				fire[i].active = false;
				refreshBaloon();
				score += 10;
			}

			if (ifHitsBee(fire[i], bee)){
				fire[i].active = false;
				bee.hit = true;
				score += 15;
			}
			
			if (ifHitsCloud(fire[i], cloud)){
				fire[i].active = false;
				cloud.hit = true;
				refreshCloud();
				score += 5;
			}
		}
	}
	

	if (!resume){
		baloons.pos.x += 3;
		cloud.pos.x += 3;
	}

	if (baloons.pos.x > 400){
		refreshBaloon();
	}

	if (cloud.pos.x > 400){
		refreshCloud();
	}

	bee.angle += 2;
	if (bee.angle > 360)
		bee.angle -= 360;

	bee.pos.x = -360;
	bee.pos.y = func(bee.angle);



	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	refreshBaloon();
	refreshCloud();
}

void main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Sumeyye Kurtulus | Assignment No: 3 | Weapon-Target Game");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();
	
	glutMainLoop();
}