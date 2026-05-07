#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include "FreeImage.h"
#include <stdio.h>
#include <cmath>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <conio.h>
#include <GL/glu.h>
#endif

using namespace std;
GLuint textura;

enum ModoCamara { ORIGINAL, PERSONAJE, LIBRE };
ModoCamara vistaActual = ORIGINAL;

float pitch = 0.0f, yaw = -90.0f;
float camX = 0, camY = 15, camZ = 50;

void luzDifusa(float r, float g, float b) {
	GLfloat diffuse[] = {r, g, b, 1.0f};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
}

void luzAmbiente(float r, float g, float b) {
	GLfloat ambient[] = {r, g, b, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
}

void dibujarCubo(float w, float h, float d) {
	float x = w / 2.0f, y = h / 2.0f, z = d / 2.0f;

	glBegin(GL_QUADS);
	// Front
	glNormal3f(0, 0, 1); glVertex3f(-x, -y, z); glVertex3f(x, -y, z); glVertex3f(x, y, z); glVertex3f(-x, y, z);
	// Back
	glNormal3f(0, 0, -1); glVertex3f(-x, -y, -z); glVertex3f(-x, y, -z); glVertex3f(x, y, -z); glVertex3f(x, -y, -z);
	// Top
	glNormal3f(0, 1, 0); glVertex3f(-x, y, -z); glVertex3f(-x, y, z); glVertex3f(x, y, z); glVertex3f(x, y, -z);
	// Bottom
	glNormal3f(0, -1, 0); glVertex3f(-x, -y, -z); glVertex3f(x, -y, -z); glVertex3f(x, -y, z); glVertex3f(-x, -y, z);
	// Right
	glNormal3f(1, 0, 0); glVertex3f(x, -y, -z); glVertex3f(x, y, -z); glVertex3f(x, y, z); glVertex3f(x, -y, z);
	// Left
	glNormal3f(-1, 0, 0); glVertex3f(-x, -y, -z); glVertex3f(-x, -y, z); glVertex3f(-x, y, z); glVertex3f(-x, y, -z);
	glEnd();
}

void dibujarCuboconTextura(float w, float h, float d) {
	float x = w / 2.0f, y = h / 2.0f, z = d / 2.0f;

	glBegin(GL_QUADS);
	// CARA SUPERIOR (La que sería la cancha)
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, y, -z); // Abajo-Izquierda de la foto
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, -z);  // Abajo-Derecha de la foto
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);   // Arriba-Derecha de la foto
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, z);  // Arriba-Izquierda de la foto
	// Back
	glNormal3f(0, 0, -1); glVertex3f(-x, -y, -z); glVertex3f(-x, y, -z); glVertex3f(x, y, -z); glVertex3f(x, -y, -z);
	// Top
	glNormal3f(0, 1, 0); glVertex3f(-x, y, -z); glVertex3f(-x, y, z); glVertex3f(x, y, z); glVertex3f(x, y, -z);
	// Bottom
	glNormal3f(0, -1, 0); glVertex3f(-x, -y, -z); glVertex3f(x, -y, -z); glVertex3f(x, -y, z); glVertex3f(-x, -y, z);
	// Right
	glNormal3f(1, 0, 0); glVertex3f(x, -y, -z); glVertex3f(x, y, -z); glVertex3f(x, y, z); glVertex3f(x, -y, z);
	// Left
	glNormal3f(-1, 0, 0); glVertex3f(-x, -y, -z); glVertex3f(-x, -y, z); glVertex3f(-x, y, z); glVertex3f(-x, y, -z);
	glEnd();
}

void dibujarArco() {
	float altura = 5,
	      largo = 12,
	      ancho = 0.5;

	glPushMatrix();
	glTranslatef(.0, 0, -25.);

	glPushMatrix();
	glTranslatef(-largo / 2.0, (altura - ancho) / 2.0, 0.);
	dibujarCubo(ancho, altura, ancho);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(.0, altura - ancho, 0.);
	dibujarCubo(largo, ancho, ancho);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(largo / 2.0, (altura - ancho) / 2.0, 0.);
	dibujarCubo(ancho, altura, ancho);
	glPopMatrix();

	glPopMatrix();
}

void dibujarCancha() {
	glEnable(GL_TEXTURE_2D);       // 1. Activo texturas
	glBindTexture(GL_TEXTURE_2D, textura); // 2. Uso la de la cancha

	glPushMatrix();
	glTranslatef(0, -0.2, 0);
	dibujarCuboconTextura(30, 0.4, 50); // 3. Dibujo
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void dibujarPelota() {
	glPushMatrix();
	glTranslatef(0, 1, 0);
	gluSphere(gluNewQuadric(), 1, 30, 30);
	glPopMatrix();
}
class Golero {
private:
	float x, y, z;
	float velocidad;
	int direccion; // 1 = Derecha, -1 = Izquierda
	const float limite = 4.5f; // El arco mide 12, el golero se mueve en este rango

public:
	Golero() {
		x = 0.0f;
		y = 1.5f;    // Altura media para que toque el suelo
		z = -24.0f;  // Un poquito adelante del arco (que está en -25)
		velocidad = 8.0f;
		direccion = 1;
	}

	void actualizar(float dt) {
		// Movimiento lineal
		x += velocidad * dt * direccion;

		// Si toca los límites del arco, cambia de dirección
		if (x >= limite) {
			x = limite;
			direccion = -1;
		}
		else if (x <= -limite) {
			x = -limite;
			direccion = 1;
		}
	}

	void dibujar() {
		glPushMatrix();
		glTranslatef(x, y, z);

		

		// Cuerpo del golero (ancho 2, alto 3, grosor 1)
		dibujarCubo(2.0f, 3.0f, 1.0f);
		glPopMatrix();
	}
};
class Defensa {
private:
	int display_list;
	float largo, alto, ancho;
	float x, y, z;
public:
	Defensa(float x, float z): x(x), z(z) {
		display_list = glGenLists(2);

		largo = 3;
		alto = 1;
		ancho = 1;
		y = alto / 2;

		glNewList(display_list, GL_COMPILE);
		dibujarCubo(largo, alto, ancho);
		glEndList();
	}

	void dibujar() {
		glPushMatrix();
		glTranslatef(x, y, z);
		glCallList(display_list);
		glPopMatrix();
	}
};

class Plataforma {
private:
	float largo, alto, ancho;
	float x, y, z, v, max_x;
	int display_list;
public:
	explicit Plataforma(float max_x): max_x(max_x) {
		display_list = glGenLists(1);

		largo = 5;
		alto = 1;
		ancho = 1;
		x = 0;
		y = alto / 2;
		z = 22;
		v = 15;

		glNewList(display_list, GL_COMPILE);
		dibujarCubo(largo, alto, ancho);
		glEndList();
	}
	float getX() const { return x; }
	float getZ() const { return z; }

	void mover(float dt, float direccion) {
		x += v * dt * direccion;
		x = min(x, max_x - largo / 2);
		x = max(x, -max_x + largo / 2);
	}

	void dibujar()  {
		glPushMatrix();
		glTranslatef(x, y, z);
		glCallList(display_list);
		glPopMatrix();
	}
};

int main(int argc, char *argv[]) {
	//INICIALIZACION
	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		cerr << "No se pudo iniciar SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	SDL_Window* win = SDL_CreateWindow("Lab 1 Comp. Grafica, Juan Andres Olmedo y Francisco Piloni",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1000, 700, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	glMatrixMode(GL_PROJECTION);

	float color = 0;
	glClearColor(color, color, color, 1);

	gluPerspective(45, 1000 / 700.f, 0.1, 110);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);

	//TEXTURA
	#ifdef __APPLE__
		char archivo[] = "canchaFutbol.jpg";
	#else
		char archivo[] = "../canchaFutbol.jpg";
	#endif

	//CARGAR IMAGEN
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(archivo);
	FIBITMAP* bitmap = FreeImage_Load(fif, archivo);
	bitmap = FreeImage_ConvertTo24Bits(bitmap);
	int w = FreeImage_GetWidth(bitmap);
	int h = FreeImage_GetHeight(bitmap);
	void* datos = FreeImage_GetBits(bitmap);
	//FIN CARGAR IMAGEN

	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, datos);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, datos);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	bool fin = false;
	bool rotate = false;

	SDL_Event evento;

	float x, y, z;

	x = 0;
	y = 15;
	z = 50;
	float degrees = 0;

	GLfloat luz_posicion[4] = { 20, 5, 30, 1 };
	GLfloat luz_posicion1[4] = { 20, 5, -20, 1 };
	GLfloat luz_posicion2[4] = { -15, 5, 30, 1 };
	GLfloat luz_posicion3[4] = { -15, 5, -20, 1 };
	GLfloat colorLuz[4] = { 1, 1, 1, 1 };
	//FIN INICIALIZACION
	bool textOn = true;
	Golero golero;



	Plataforma plataforma(15);
	Defensa d1(-12, -10),
			d2(-8, -10),
			d3(-4, -10),
			d4(0, -10),
			d5(4, -10),
			d6(8, -10),
			d7(12, -10);

	bool left = false, right = false;

	Uint32 last = SDL_GetTicks();

	//LOOP PRINCIPAL
	do{
		Uint32 now = SDL_GetTicks();
		float deltaTime = (now - last) / 1000.0f;
		last = now;
		
		golero.actualizar(deltaTime);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		// --- CALCULO VECTORES DE CÁMARA ---
		float lookX = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
		float lookY = sin(pitch * M_PI / 180.0f);
		float lookZ = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
		
		// --- APLICAR MODO DE CAMARA ---
		float platX = plataforma.getX();
		float platZ = plataforma.getZ();
		switch (vistaActual) {
		case ORIGINAL:
			gluLookAt(0, 15, 50,
				0, 0, 0,
				0, 1, 0);
			break;

		case PERSONAJE:
			gluLookAt(platX, 5, platZ + 10,
				platX + lookX, 5 + lookY, (platZ + 8) + lookZ,
				0, 1, 0);
			break;

		case LIBRE:
			gluLookAt(camX, camY, camZ,
				camX + lookX, camY + lookY, camZ + lookZ,
				0, 1, 0);
			break;
		}
		



		//PRENDO LA LUZ (SIEMPRE DESPUES DEL gluLookAt)
		glEnable(GL_LIGHT0); // habilita la luz 0
		glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);
		
		glEnable(GL_LIGHT1); // habilita la luz 1
		glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuz);
		
		glEnable(GL_LIGHT2); // habilita la luz 2
		glLightfv(GL_LIGHT2, GL_POSITION, luz_posicion2);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, colorLuz);
		
		glEnable(GL_LIGHT3); // habilita la luz 3
		glLightfv(GL_LIGHT3, GL_POSITION, luz_posicion3);
		glLightfv(GL_LIGHT3, GL_DIFFUSE, colorLuz);

		glEnable(GL_LIGHTING);

		dibujarArco();

		luzAmbiente(1, 1, 1);
		dibujarCancha();
		golero.dibujar();
		dibujarPelota();

		if (right)
			plataforma.mover(deltaTime, 1);
		if (left)
			plataforma.mover(deltaTime, -1);
		plataforma.dibujar();
		d1.dibujar();
		d2.dibujar();
		d3.dibujar();
		d4.dibujar();
		d5.dibujar();
		d6.dibujar();
		d7.dibujar();

		glDisable(GL_LIGHTING);

		while (SDL_PollEvent(&evento)) {
			switch (evento.type) {
			case SDL_QUIT:
				fin = true;
				break;

			case SDL_MOUSEMOTION:
				// IMPORTANTE: Solo rotamos si es modo LIBRE
				if (vistaActual == LIBRE) {
					if (evento.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
						// Paneo con click derecho
						float panSpeed = 0.05f;
						float rightX = sin(yaw * M_PI / 180.0f);
						float rightZ = -cos(yaw * M_PI / 180.0f);
						camX += -evento.motion.xrel * rightX * panSpeed;
						camZ += -evento.motion.xrel * rightZ * panSpeed;
						camY += evento.motion.yrel * panSpeed;
					}
					else {
						// Rotación normal
						float sensibilidad = 0.2f;
						yaw += evento.motion.xrel * sensibilidad;
						pitch -= evento.motion.yrel * sensibilidad;
						if (pitch > 89.0f) pitch = 89.0f;
						if (pitch < -89.0f) pitch = -89.0f;
					}
				}
				break;

			case SDL_MOUSEWHEEL:
				if (vistaActual == LIBRE) {
					float scrollSpeed = 3.0f;
					camX += lookX * evento.wheel.y * scrollSpeed;
					camY += lookY * evento.wheel.y * scrollSpeed;
					camZ += lookZ * evento.wheel.y * scrollSpeed;
				}
				break;

			case SDL_KEYDOWN:
				switch (evento.key.keysym.sym) {
				case SDLK_RIGHT: right = true; break;
				case SDLK_LEFT:  left = true;  break;
				case SDLK_v:
					// Ciclo de cámaras
					if (vistaActual == ORIGINAL) {
						vistaActual = PERSONAJE;
						SDL_SetRelativeMouseMode(SDL_FALSE); // No necesitamos atrapar mouse en 1ra persona fija
					}
					else if (vistaActual == PERSONAJE) {
						vistaActual = LIBRE;
						SDL_SetRelativeMouseMode(SDL_TRUE);  // Atrapamos para modo libre
					}
					else {
						vistaActual = ORIGINAL;
						SDL_SetRelativeMouseMode(SDL_FALSE);
					}
					break;
				case SDLK_ESCAPE: fin = true; break;
				}
				break;

			case SDL_KEYUP:
				switch (evento.key.keysym.sym) {
				case SDLK_RIGHT: right = false; break;
				case SDLK_LEFT:  left = false;  break;
				}
				break;
			}
		}
		SDL_GL_SwapWindow(win);
	} while (!fin);

	//FIN LOOP PRINCIPAL
	// LIMPIEZA
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
