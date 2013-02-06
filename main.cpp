#if defined(__APPLE_CC__)
#include<OpenGL/gl.h>
#include<OpenGL/glu.h>
#include<GLUT/glut.h>
#elif defined(WIN32)
#include<windows.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#else
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#include<stdint.h>
#endif

#include<iostream>
#include<stdlib.h>

void incrementLookatVar(int x);
void decrementLookatVar(int x);
void lookAtMovement(int current_window);
void relativeMovement(int current_window);
void drawShip();
bool invert_pose( float *m );
void drawCannon();
void drawWing();
void drawShip(int slices);
void relChange();
void loadDefault(int current_window);
void drawPlanet(int planetIndex, float colorR, float colorG, float colorB, float colorA);
void drawSolarSystem();
void drawSun();
void drawEarth();
void drawSaturn();
void drawPluto();
void rotateInSpace(int arrayIndex);
void geoSyncLock(int current_window);
void resetGeoSyncVars();

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Global State Variables ///////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// time increment between calls to idle() in ms,
// currently set to 30 FPS
float dt = 1000.0f*1.0f/30.0f;

// flag to indicate that we should clean up and exit
bool quit = false;

// window handles for mother ship and scout ship
int mother_window, scout_window;
bool onMotherShip = true;

// display width and height
int disp_width=512, disp_height=512;

// 16 slot arrays, which are how openGL represents matrices
// falcoLast and peppyLast are for relative mode, and the geosync
// matrices are for geosync mode. Separated for clarity in code.
float lastShip[16];
float falcoLast[16];
float peppyLast[16];
float geosyncTargetFalco[16];
float geosyncTargetPeppy[16];
float geoSyncFalco[16];
float geoSyncPeppy[16];

// boolean to control pausing the orbits.
bool isPaused = false;

// Boolean to check if the mode has changed, and a counter to ensure that the window is redrawn twice
// to reset the default view.
bool hasModeChanged = true;
int modeChangedCounter = 0;
// Planet numbers stored in a 2d-array
// first column is rotation in degrees
// second column is amount to increment during orbit
// third column is size of planet
float planets[10][3] =
{
	{0,1,0.7},   // Sun
	{0,1.2,0.18},   // Mercury
	{0,1.1,0.25},   // Venus
	{0,1,0.25},   // Earth
	{0,1.7,0.22},   // Mars
	{0,1.3,0.45},   // Jupiter
	{0,1.4,0.23},   // Saturn
	{0,1.3,0.24},   // Uranus
	{0,1.0,0.22},   // Neptune
	{0,0.78,0.13}    // Pluto
};


// Absolute look-at variables
// Default mode is lookat
bool inLookatMode = true;
// First column is Mothership absolute CURRENT points
// Second column is Scoutship absolute CURRENT points
// Third column is value to use for increment/decrement
// Fourth column is Mothership absolute DEFAULT points
// Fifth column is Scoutship absolute DEFAULT points
float absoluteVars[9][5] =
{
	{0,0,0.1,0,0},              // eyePointX
	{10,8,0.1,10,8},              // eyePointY
	{20,16,0.1,20,16},          // eyePointZ
	{0,0,0.1,0,0},              // lookatPointX
	{2,2.3,0.1,2,2.3},          // lookatPointY
	{-1,-1,0.1,-1,-1},          // lookatPointZ
	{0,0,0.1,0,0},              // upVectorX
	{1,1,0.1,1,1},              // upVectorY
	{0,0,0.1,0,0}               // upVectorZ
};

// Relative mode variables
// yaw, pitch, roll, forward/backward, increase/decrease speed
float relativeVars[5] = {2.0f, 2.0f, 2.0f, 0.1f, 0.1f};
// Flag is true when a change in a relative variable is requested with a keypress
bool relativeFlag = false;
// relVal determines which relative variable to increment
int relVal = 0;
// Determines whether we are incrementing or decrementing the variable
int upOrDown = 0;
// Set to true when we enter relative mode
bool inRelativeMode = false;

// Geosync mode variables
// Set to true when we enter relative mode
bool inGeosyncMode = false;
// orbitPlanet is the ship that Falco will orbit,
// orbitPlanet2 is the ship that Peppy will orbit
int orbitPlanet = 3;
int orbitPlanet2 = 3; 
// geoSyncDistance is the variable that moves the ships towards and away from the planets
float geoSyncDistanceFalco = -1.3;
float geoSyncDistancePeppy = -1.3;
// The speed at which the ships move
float geoSyncSpeed = 0.1;
// flag set to true when one ship starts orbitting a planet. Necessary to keep it orbitting
// if the other ship also starts orbitting.
bool otherShipOrbiting = false;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Initialization/Setup and Teardown ////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// set up opengl state, allocate objects, etc.  This gets called
// ONCE PER WINDOW, so don't allocate your objects twice!
void init(){
	/////////////////////////////////////////////////////////////
	/// TODO: Put your initialization code here! ////////////////
	/////////////////////////////////////////////////////////////
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

	glViewport( 0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_NORMALIZE );

	// lighting stuff
	GLfloat ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[] = {0.9, 0.9, 0.9, 1.0};
	GLfloat specular[] = {0.4, 0.4, 0.4, 1.0};
	GLfloat position0[] = {1.0, 1.0, 1.0, 0.0};
	glLightfv( GL_LIGHT0, GL_POSITION, position0 );
	glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
	GLfloat position1[] = {-1.0, -1.0, -1.0, 0.0};
	glLightfv( GL_LIGHT1, GL_POSITION, position1 );
	glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
	glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
	glLightfv( GL_LIGHT1, GL_SPECULAR, specular );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );
	glEnable( GL_COLOR_MATERIAL );

}

// free any allocated objects and return
void cleanup(){
	/////////////////////////////////////////////////////////////
	/// TODO: Put your teardown code here! //////////////////////
	/////////////////////////////////////////////////////////////
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Callback Stubs ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// window resize callback
void resize_callback( int width, int height ){
	/////////////////////////////////////////////////////////////
	/// TODO: Put your resize code here! ////////////////////////
	/////////////////////////////////////////////////////////////
	disp_width = width;
	disp_height = height;
	glViewport(0,0,width,height);
}

// keyboard callback
void keyboard_callback( unsigned char key, int x, int y ){
	switch( key ){
	case 27:
		quit = true;
		break;
	case 'x':
		if (inLookatMode)
			incrementLookatVar(0);
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 2;
			upOrDown = 0;
		}
		break;
	case 'X':
		if (inLookatMode)
			decrementLookatVar(0);
		break;
	case 'y':
		if (inLookatMode)
			incrementLookatVar(1);
		break;
	case 'Y':
		if (inLookatMode)
			decrementLookatVar(1);
		break;
	case 'z':
		if (inLookatMode)
			incrementLookatVar(2);
		break;
	case 'Z':
		if (inLookatMode)
			decrementLookatVar(2);
		break;
	case 'a':
		if (inLookatMode)
			incrementLookatVar(3);
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 1;
			upOrDown = 0;
		}
		break;
	case 'A':
		if (inLookatMode)
			decrementLookatVar(3);
		break;
	case 'b':
		if (inLookatMode)
			incrementLookatVar(4);
		break;
	case 'B':
		if (inLookatMode)
			decrementLookatVar(4);
		break;
	case 'c':
		if (inLookatMode)
			incrementLookatVar(5);
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 2;
			upOrDown = 1;
		}
		break;
	case 'C':
		if (inLookatMode)
			decrementLookatVar(5);
		break;
	case 'd':
		if (inLookatMode)
			incrementLookatVar(6);
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 1;
			upOrDown = 1;
		}
		break;
	case 'D':
		if (inLookatMode)
			decrementLookatVar(6);
		break;
	case 'e':
		if (inLookatMode)
			incrementLookatVar(7);
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 0;
			upOrDown = 1;
		}
		break;
	case 'E':
		if (inLookatMode)
			decrementLookatVar(7);
		break;
	case 'f':
		if (inLookatMode)
			incrementLookatVar(8);
		break;
	case 'F':
		if (inLookatMode)
			decrementLookatVar(8);
		break;
	case '<':
		if (inGeosyncMode && onMotherShip)
			otherShipOrbiting = true;
		onMotherShip = false;
		break;
	case '>':
		if (inGeosyncMode && !onMotherShip)
			otherShipOrbiting = true;
		onMotherShip = true;
		break;
	case 'l':
		if (!inLookatMode) {
			inLookatMode = true;
			inRelativeMode = false;
			hasModeChanged = true;
			inGeosyncMode = false;
			modeChangedCounter = 0;
		}
		break;
	case 'r':
		if (!inRelativeMode) {
			inRelativeMode = true;
			inLookatMode = false;
			inGeosyncMode = false;
			hasModeChanged = true;
			modeChangedCounter = 0;
		}
		break;
	case 'w':
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 3;
			upOrDown = 0;
		}
		if (inGeosyncMode) {
			if (onMotherShip)
				geoSyncDistanceFalco += geoSyncSpeed;
			else
				geoSyncDistancePeppy += geoSyncSpeed;
			if (geoSyncDistanceFalco > -1)
				geoSyncDistanceFalco = -1;
			if (geoSyncDistancePeppy > -1)
				geoSyncDistancePeppy = -1;
		}
		break;
	case 's':
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 3;
			upOrDown = 1;
		}
		if (inGeosyncMode)
			if (onMotherShip)
				geoSyncDistanceFalco -= geoSyncSpeed;
			else
				geoSyncDistancePeppy -= geoSyncSpeed;
		break;
	case 'q':
		if (inRelativeMode) {
			relativeFlag = true;
			relVal = 0;
			upOrDown = 0;
		}
		break;
	case 'g':
		if (!inGeosyncMode) {
			inGeosyncMode = true;
			inRelativeMode = false;
			inLookatMode = false;
			hasModeChanged = true;
			resetGeoSyncVars();
			orbitPlanet = 3;
		}
		break;
	case '1':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 1;
			else
				orbitPlanet2 = 1;
			resetGeoSyncVars();
		}
		break;
	case '2':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 2;
			else
				orbitPlanet2 = 2;
			resetGeoSyncVars();
		}
		break;
	case '3':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 3;
			else
				orbitPlanet2 = 3;
			resetGeoSyncVars();
		}
		break;
	case '4':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 4;
			else
				orbitPlanet2 = 4;
			resetGeoSyncVars();
		}
		break;
	case '5':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 5;
			else
				orbitPlanet2 = 5;
			resetGeoSyncVars();
		}
		break;
	case '6':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 6;
			else
				orbitPlanet2 = 6;
			resetGeoSyncVars();
		}
		break;
	case '7':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 7;
			else
				orbitPlanet2 = 7;
			resetGeoSyncVars();
		}
		break;
	case '8':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 8;
			else
				orbitPlanet2 = 8;
			resetGeoSyncVars();
		}
		break;
	case '9':
		if (inGeosyncMode) {
			if (onMotherShip)
				orbitPlanet = 9;
			else
				orbitPlanet2 = 9;
			resetGeoSyncVars();
		}
		break;
	case '=':
		if (inGeosyncMode)
			geoSyncSpeed += 0.1;
		if (inRelativeMode) {
			for (int i = 0; i < 3; i++)
				relativeVars[i] += relativeVars[4];
			relativeVars[3] += 0.03;
		}
		if (inLookatMode) {
			for (int i = 0; i < 9; i++)
				absoluteVars[i][2] += 0.05;
		}
		break;
		break;
	case '-':
		if (inGeosyncMode) {
			geoSyncSpeed -= 0.1;
			if (geoSyncSpeed <= 0.1)
				geoSyncSpeed = 0.1;
		}
		if (inRelativeMode) {
			for (int i = 0; i < 3; i++)
				if (relativeVars[i] > 0.2)
					relativeVars[i] -= relativeVars[4];
			if (relativeVars[3] > 0.06)
				relativeVars[3] -= 0.03;
		}
		if (inLookatMode) {
			for (int i = 0; i < 9; i++)
				if (absoluteVars[i][2] > 0.1)
					absoluteVars[i][2] -= 0.05;
		}
		break;
	case 'p':
		isPaused = true;
		break;
	case 'P':
		isPaused = false;
	case 'm':
		break;
	default:
		break;
	}

	/////////////////////////////////////////////////////////////
	/// TODO: Put your keyboard code here! //////////////////////
	/////////////////////////////////////////////////////////////

}

void resetGeoSyncVars() {
	geoSyncSpeed = 0.1;
	geoSyncDistanceFalco = -1.3;
	geoSyncDistancePeppy = -1.3;
}
// Functions that take an integer, x, and updates the corresponding variable
// in the absoluteVars array, whether it be an increment or decrement
void incrementLookatVar(int x) {

	if (onMotherShip)
		absoluteVars[x][0] += absoluteVars[x][2];
	else
		absoluteVars[x][1] += absoluteVars[x][2];
}
void decrementLookatVar(int x) {

	if (onMotherShip)
		absoluteVars[x][0] -= absoluteVars[x][2];
	else
		absoluteVars[x][1] -= absoluteVars[x][2];
}

// display callback
void display_callback( void ){
	int current_window;

	// retrieve the currently active window
	current_window = glutGetWindow();
	// clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/////////////////////////////////////////////////////////////
	/// TODO: Put your rendering code here! /////////////////////
	/////////////////////////////////////////////////////////////
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 70.0f, float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT)), 0.1f, 2000.0f );

	glMatrixMode(GL_MODELVIEW);

	// Draw the ship from the OTHER window -> need to use saved lastShip matrix to get its world coordinates;
	glPushMatrix();
	glMultMatrixf(lastShip);
	drawShip(100);
	glPopMatrix();
	glLoadIdentity();  

	if (hasModeChanged)
		loadDefault(current_window);
	else if (inLookatMode)
		lookAtMovement(current_window);
	else if (inRelativeMode)
		relativeMovement(current_window);
	else if (inGeosyncMode) {
		geoSyncLock(current_window);
	}

	// Move camera to current ship location
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, lastShip);
	glLoadIdentity();
	glMultMatrixf(lastShip);
	invert_pose(lastShip);
	glPopMatrix();

	/*glBegin(GL_LINES);
	glColor3f( 1.0f, 0.0f, 0.0f );
	glVertex3f( 1.0f, 0.0f, 0.0f );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glColor3f( 0.0f, 1.0f, 0.0f );
	glVertex3f( 0.0f, 1.0f, 0.0f );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glColor3f( 0.0f, 0.0f, 1.0f );
	glVertex3f( 0.0f, 0.0f, 1.0f );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();*/

	// Draw Solar System
	drawSolarSystem();
	// swap the front and back buffers to display the scene

	glutSetWindow( current_window );
	glutSwapBuffers();

}

// Function that draws the entire solar system
void drawSolarSystem() {
	drawSun();
	drawPlanet(1,0.5,0.5,0.5,1); // Draw Mercury
	drawPlanet(2,0.8,0.7,0,1);   // Draw Venus
	drawEarth();                 // Draw Earth + moon
	drawPlanet(4,1,0,0,1);       // Draw Moon 
	drawPlanet(5,0.7,0.3,0.5,1); // Draw Jupiter
	drawSaturn();                // Draw Saturn
	drawPlanet(7,0.3,1,1,1);     // Draw Uranus
	drawPlanet(8,0.3,0.7,1,1);   // Draw Neptune
	drawPluto();                 // Draw Pluto
	glPopMatrix();
	glPopMatrix();
}

// Draw the sun, and each of the circles around it for the orbits
void drawSun() {

	glPushMatrix();
	glPushMatrix();
	glPushMatrix();
	glRotatef(90,1,0,0);
	glColor4f(1,1,1,1);
	gluDisk(gluNewQuadric(), 0.96, 1, 100, 100);
	gluDisk(gluNewQuadric(), 1.96, 2, 100, 100);
	gluDisk(gluNewQuadric(), 2.96, 3, 100, 100);
	gluDisk(gluNewQuadric(), 3.96, 4, 100, 100);
	gluDisk(gluNewQuadric(), 4.96, 5, 100, 100);
	gluDisk(gluNewQuadric(), 5.96, 6, 100, 100);
	gluDisk(gluNewQuadric(), 6.96, 7, 100, 100);
	gluDisk(gluNewQuadric(), 7.96, 8, 100, 100);
	glRotatef(10,1,1,1);
	gluDisk(gluNewQuadric(), 9.46, 9.5, 100, 100);
	glRotatef(10,-1,-1,-1);
	glPopMatrix();
	glRotatef(planets[0][0],0,1,0);
	glColor4f(0.8,0.3,0,1);
	glutSolidSphere(planets[0][2], 10 , 10);
	glPopMatrix();
}

// Helper function to draw a planet. Takes as input an index in the planet array, and the 
// red, green, and blue parts of the planet color, along with an alpha integer to determine the
// planet's translucency. 
void drawPlanet(int planetIndex, float colorR, float colorG, float colorB, float colorA) {

	glPushMatrix();
	glRotatef(planets[planetIndex][0],0,1,0);
	glTranslatef(planetIndex,0,0);
	glRotatef(planets[planetIndex][0],0,1,0);
	glColor4f(colorR,colorG,colorB,colorA);
	if (orbitPlanet == planetIndex)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetFalco);
	if (orbitPlanet2 == planetIndex)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetPeppy);
	glutSolidSphere(planets[planetIndex][2], 10, 10);
	glPopMatrix();
}

// Function to draw Earth - has it's own function because moon orbits earth, therefore we can't pop the matrix before
// drawing the moon
void drawEarth() {
	// Draw Earth
	glRotatef(planets[3][0],0,1,0);
	glTranslatef(3,0,0);
	glRotatef(planets[3][0],0,1,0);
	glColor4f(0,0,1,1);
	if (orbitPlanet == 3)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetFalco);
	if (orbitPlanet2 == 3)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetPeppy);
	glutSolidSphere(planets[3][2], 10, 10); 
	// Draw Earth's moon
	glColor4f(1,1,1,1);
	glPushMatrix();
	glRotatef(90,1,0,0);
	gluDisk(gluNewQuadric(), 0.514, 0.55, 100, 100);
	glPopMatrix();
	glRotatef(planets[3][0],0,1,0);
	glTranslatef(0.55,0,0);
	glColor4f(0.5,0.5,0.5,1);
	glutSolidSphere(0.1,10,10);
	// glPopMatrix();
	glPopMatrix();
}

// Function to draw Saturn - givne it's own function due to Saturn's rings
void drawSaturn() {  
	// Draw Saturn
	glPushMatrix();
	glRotatef(planets[6][0],0,1,0);
	glTranslatef(6,0,0);
	glRotatef(planets[6][0],0,1,0);
	glColor4f(0.3,0.7,0.5,1);
	if (orbitPlanet == 6)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetFalco);
	if (orbitPlanet2 == 6)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetPeppy);
	glutSolidSphere(planets[6][2], 10, 10);
	// Draw Saturn's rings
	glPushMatrix();
	glRotatef(90,1,0,0);
	glRotatef(30, 1, 1, 0);
	gluDisk(gluNewQuadric(), 0.5, 0.8, 100, 100);
	glPopMatrix();
	glPopMatrix();
}

// Function to draw Pluto - given it's own function due to unusual orbit.
void drawPluto() {
	glPushMatrix();
	glRotatef(10,1,1,1);
	glRotatef(planets[9][0],0,1,0);
	glTranslatef(9.5,0,0);
	glRotatef(planets[9][0],0,1,0);
	glColor4f(0.5,0.5,0.5,1);
	if (orbitPlanet == 9)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetFalco);
	if (orbitPlanet2 == 9)
		glGetFloatv(GL_MODELVIEW_MATRIX, geosyncTargetPeppy);
	glutSolidSphere(planets[9][2], 10, 10);
	glPopMatrix();
}


// not exactly a callback, but sets a timer to call itself
// in an endless loop to update the program
void idle( int value ){

	// if the user wants to quit the program, then exit the
	// function without resetting the timer or triggering
	// a display update
	if( quit ){
		// cleanup any allocated memory
		cleanup();

		// perform hard exit of the program, since glutMainLoop()
		// will never return
		exit(0);
	}

	/////////////////////////////////////////////////////////////
	/// TODO: Put your idle code here! //////////////////////////
	/////////////////////////////////////////////////////////////

	if (!isPaused) {
		rotateInSpace(0);
		rotateInSpace(1);
		rotateInSpace(2);
		rotateInSpace(3);
		rotateInSpace(4);
		rotateInSpace(5);
		rotateInSpace(6);
		rotateInSpace(7);
		rotateInSpace(8);
		rotateInSpace(9);
	}

	// set the currently active window to the mothership and
	// request a redisplay
	glutSetWindow( mother_window );
	glutPostRedisplay();

	// now set the currently active window to the scout ship
	// and redisplay it as well
	glutSetWindow( scout_window );
	glutPostRedisplay();

	// set a timer to call this function again after the
	// required number of milliseconds
	glutTimerFunc( dt, idle, 0 );
}

// Helper method to rotate a planet. Takes as input an integer which is the index in the planets array.
void rotateInSpace(int arrayIndex) {

	if (planets[arrayIndex][0] >= 360) {
		planets[arrayIndex][0] = planets[arrayIndex][0] - 360 + planets[arrayIndex][1];
	}
	else
		planets[arrayIndex][0] += planets[arrayIndex][1];

}

// Loads the default view into the window
void loadDefault(int current_window) {
	// Need to update both windows, therefore we need to make sure that this loadDefault function runs twice.
	// This is why there is a modeChangedCounter. It is reset to 0 in the keyboard callback if the mode is changed
	if (modeChangedCounter != 2) {
		gluLookAt(absoluteVars[0][current_window+2],
			absoluteVars[1][current_window+2],
			absoluteVars[2][current_window+2],
			absoluteVars[3][current_window+2],
			absoluteVars[4][current_window+2],
			absoluteVars[5][current_window+2],
			absoluteVars[6][current_window+2],
			absoluteVars[7][current_window+2],
			absoluteVars[8][current_window+2]); 
		// reset changed absolute variables to default absolute variables
		for (int i = 0; i < 9; i++)
			absoluteVars[i][current_window-1] = absoluteVars[i][current_window+2];
		modeChangedCounter++;

		if (current_window == 1)  {
			glGetFloatv(GL_MODELVIEW_MATRIX, falcoLast);
			glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncFalco);
		}

		else {
			glGetFloatv(GL_MODELVIEW_MATRIX, peppyLast);
			glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncPeppy);
		}
		//glGetFloatv(GL_MODELVIEW_MATRIX, lastShip);
		//glLoadMatrixf(lastShip);
	}
	else
		hasModeChanged = false;
}

// Updates the eyepoint based on the current window. The correct values will have been updated if necessary
// in the increment/decrement lookatvar function.
void lookAtMovement(int current_window) {
	gluLookAt(absoluteVars[0][current_window-1],
		absoluteVars[1][current_window-1],
		absoluteVars[2][current_window-1],
		absoluteVars[3][current_window-1],
		absoluteVars[4][current_window-1],
		absoluteVars[5][current_window-1],
		absoluteVars[6][current_window-1],
		absoluteVars[7][current_window-1],
		absoluteVars[8][current_window-1]); 
}

// Method that updates the ship's position when it is in relative mode 
void relativeMovement(int current_window) {

	// If no key has been pressed, the relative flag will be false. Therefore we can just load the same matrix that 
	// we used on the last draw.
	if (!relativeFlag) 
		if (current_window == 1) 
			glLoadMatrixf(falcoLast);
		else
			glLoadMatrixf(peppyLast);
	else {
		if (onMotherShip) {
			// If we are on Falco's ship (the mothership), and the current window is 1, update to the new location
			// and save that in falcoLast, otherwise load the last peppyLast (scoutship) value, since we aren't
			// currently controlling it.
			if (current_window == 1) {
				glLoadIdentity();
				relChange();
				glMultMatrixf(falcoLast);
				glGetFloatv(GL_MODELVIEW_MATRIX,falcoLast);
				relativeFlag = false;
			}
			else {
				glLoadMatrixf(peppyLast);
				relativeFlag = true;
			}
		}
		else {
			// If we are on Peppy's ship (the scoutship), and the current window is 2, update to the new location
			// and save that in peppyLast, otherwise load the last falcoLast (mothership) value, since we aren't
			// currently controlling it.
			if (current_window == 2) {
				glLoadIdentity();
				relChange();
				glMultMatrixf(peppyLast);
				glGetFloatv(GL_MODELVIEW_MATRIX,peppyLast);
				relativeFlag = false;
			}
			else {
				glLoadMatrixf(falcoLast);
				relativeFlag = true;
			}	
		}
	}
}

// Helper function to change relative variables. It switches between the global variable, relVal,
// and updates the corresponding variables depending on whether it is increasing or decreasing.
void relChange() {

	switch(relVal){
	case 0:
		if(upOrDown == 0)
			glRotatef(relativeVars[0],0,1,0);
		else
			glRotatef(-relativeVars[0],0,1,0);
		break;
	case 1:
		if(upOrDown == 0)
			glRotatef(relativeVars[1],0,0,1);
		else
			glRotatef(-relativeVars[1],0,0,1);
		break;
	case 2:
		if(upOrDown == 0)
			glRotatef(relativeVars[2],1,0,0);
		else
			glRotatef(-relativeVars[2],1,0,0);
		break;
	case 3:
		if (upOrDown == 0)
			glTranslatef(0,0,relativeVars[3]);
		else
			glTranslatef(0,0,-relativeVars[3]);
		break;
	}
}

// Method that updates the ship's position when it is in geosync mode
void geoSyncLock(int current_window) {

	// If we are on the mothership, and the current window is 1, we need to invert both the lastShip matrix
	// and the geosyncTargetFalco matrix, to get to the current position of the ship/camera.
	if (onMotherShip) {
		if (current_window == 1) {

			invert_pose(lastShip);
			glLoadIdentity();
			invert_pose(geosyncTargetFalco);
			glTranslatef(0,-0.3,geoSyncDistanceFalco);
			glRotatef(10,1,0,0);
			glMultMatrixf(geosyncTargetFalco);
			glMultMatrixf(lastShip);
			glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncFalco);
		}
		else {
			// If the other ship is already orbiting, we need to ensure that it keeps orbiting, so there is a boolean check for that.
			// Otherwise it will just load the default view, which is at geoSyncPeppy initially.
			if (otherShipOrbiting) {
				invert_pose(lastShip);
				glLoadIdentity();
				invert_pose(geosyncTargetPeppy);
				glTranslatef(0,-0.3,geoSyncDistancePeppy);
				glRotatef(10,1,0,0);
				glMultMatrixf(geosyncTargetPeppy);
				glMultMatrixf(lastShip);
				glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncPeppy);
			}
			else
				glLoadMatrixf(geoSyncPeppy);
		}
	}
	else {

		// If we are on the scoutship, and the current window is 2, we need to inver both the lastShip matrix and the
		// geosyncTargetPeppt matrix, to get to the current position of the ship/camera.
		if (current_window == 2) {

			invert_pose(lastShip);
			glLoadIdentity();
			invert_pose(geosyncTargetPeppy);
			glTranslatef(0,-0.3,geoSyncDistancePeppy);
			glRotatef(10,1,0,0);
			glMultMatrixf(geosyncTargetPeppy);
			glMultMatrixf(lastShip);
			glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncPeppy);
		}
		else {
			// Check for other ship orbiting
			if (otherShipOrbiting) {
				invert_pose(lastShip);
				glLoadIdentity();
				invert_pose(geosyncTargetFalco);
				glTranslatef(0,-0.3,geoSyncDistanceFalco);
				glRotatef(10,1,0,0);
				glMultMatrixf(geosyncTargetFalco);
				glMultMatrixf(lastShip);
				glGetFloatv(GL_MODELVIEW_MATRIX, geoSyncFalco);
			}
			else
				glLoadMatrixf(geoSyncFalco);
		}
	}
}

// Method to draw a ship
void drawShip(int slices){
	glRotatef(180,0,1,0);
	glScalef(0.1,0.1,0.1);
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_SMOOTH);
	gluQuadricTexture(qobj, true);
	glTranslatef(0,0,-1.5f);
	glPushMatrix();
	glScaled(1, 1, 4);
	gluCylinder(qobj, 0.7, 0.3, 1.0, slices, 5);
	glPopMatrix();
	glPushMatrix();
	glRotatef(-10, 0, 0, 1);
	drawWing();
	glRotatef(30, 0, 0, 1);
	drawWing();
	glRotatef(-180, 0, 0, 1);
	drawWing();
	glRotatef(-30, 0, 0, 1);
	drawWing();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 0, 4);
	gluCylinder(qobj, 0.3, 0, 0.4, slices, 5);
	glPopMatrix();
}

// Method to draw a wing of the ship
void drawWing(){
	glPushMatrix();
	glScaled(2.5, 0.1, 1);
	glTranslatef(0.5, 0, 0.5);
	glutSolidCube(1);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(2.5, 0, 0);
	drawCannon();
	glPopMatrix();
}

// Method to draw a cannon on the ship
void drawCannon(){
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricTexture(qobj, true);
	glPushMatrix();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluCylinder(qobj, 0.1, 0.1, 1.2, 10, 5);
	gluCylinder(qobj, 0.05, 0.05, 2.4, 10, 5);
	glPopMatrix();
}




// inversion routine originally from MESA
bool invert_pose( float *m ){
	float inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		m[i] = inv[i] * det;

	return true;
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/// Program Entry Point //////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int main( int argc, char **argv ){
	// initialize glut
	glutInit( &argc, argv );

	// use double-buffered RGB+Alpha framebuffers with a depth buffer.
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

	// initialize the mothership window
	glutInitWindowSize( disp_width, disp_height );
	glutInitWindowPosition( 0, 100 );
	mother_window = glutCreateWindow( "Falco" );
	glutKeyboardFunc( keyboard_callback );
	glutDisplayFunc( display_callback );
	glutReshapeFunc( resize_callback );

	// initialize the scout ship window
	glutInitWindowSize( disp_width, disp_height );
	glutInitWindowPosition( disp_width+50, 100 );
	scout_window = glutCreateWindow( "Peppy" );
	glutKeyboardFunc( keyboard_callback );
	glutDisplayFunc( display_callback );
	glutReshapeFunc( resize_callback );

	glutSetWindow( mother_window );
	init();
	glutSetWindow( scout_window );
	init();

	// start the idle on a fixed timer callback
	idle( 0 );

	// start the blug main loop
	glutMainLoop();

	return 0;
}

