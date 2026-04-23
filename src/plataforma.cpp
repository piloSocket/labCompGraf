#include "../include/plataforma.hpp"
#include <stdio.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <conio.h>
#include <GL/glu.h>
#endif
#include <algorithm>

using namespace std;
void dibujarCubo(float w, float h, float d);

Plataforma::Plataforma(float max_x): max_x(max_x) {
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

void Plataforma::mover(float dt, float direccion) {
	x += v * dt * direccion;
	x = min(x, max_x - largo / 2);
	x = max(x, -max_x + largo / 2);
}

void Plataforma::dibujar() {
	glPushMatrix();
	glTranslatef(x, y, z);
	glCallList(display_list);
	glPopMatrix();
}