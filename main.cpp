#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include <deque>
#include "FreeImage.h"
#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <map>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <SDL2/SDL_ttf.h>
#else
#include <conio.h>
#include "SDL_ttf.h"
#include <GL/glu.h>
#endif

using namespace std;

enum ModoCamara { ORIGINAL, PERSONAJE, LIBRE };
ModoCamara vistaActual = ORIGINAL;

float pitch = 16.7f, yaw = 0, radio = sqrt(50 * 50 + 15 * 15);
float camX = 0, camY = 15, camZ = 50;

void luzDifusa(float r, float g, float b) {
	GLfloat diffuse[] = {r, g, b, 1.0f};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
}

void luzAmbiente(float r, float g, float b) {
	GLfloat ambient[] = {r, g, b, 1.0f};
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
}

struct Particula {
	float x, y, z;
	float vx, vy, vz;
	float vida; // Tiempo restante de vida
	float r, g, b;
};

class SistemaFuegos {
private:
	vector<Particula> particulas;
	static SistemaFuegos* instancia;
	SistemaFuegos() {}

public:
	static SistemaFuegos* getInstance() {
		if (instancia == nullptr) instancia = new SistemaFuegos();
		return instancia;
	}

	void crearExplosion(float x, float y, float z) {
		for (int i = 0; i < 100; i++) { // 100 partículas por explosión
			Particula p;
			p.x = x; p.y = y; p.z = z;

			// Velocidad aleatoria tipo explosión
			p.vx = ((rand() % 100) - 50) / 10.0f;
			p.vy = ((rand() % 100) / 10.0f) + 2.0f; // Siempre hacia arriba
			p.vz = ((rand() % 100) - 50) / 10.0f;

			p.vida = 1.0f + (rand() % 100) / 100.0f; // 1 a 2 segundos

			// Color Naranja (R=1.0, G=0.5, B=0.0) con ligera variación
			p.r = 1.0f;
			p.g = 0.4f + (rand() % 30) / 100.0f;
			p.b = 0.0f;

			particulas.push_back(p);
		}
	}

	void actualizar(float dt) {
		for (auto it = particulas.begin(); it != particulas.end();) {
			it->vida -= dt;
			if (it->vida <= 0) {
				it = particulas.erase(it);
			}
			else {
				// Aplicar física básica
				it->x += it->vx * dt;
				it->y += it->vy * dt;
				it->z += it->vz * dt;
				it->vy -= 9.8f * dt; // Gravedad
				it++;
			}
		}
	}

	void dibujar() {
		glDisable(GL_LIGHTING); // Los fuegos artificiales emiten luz, no la reciben
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		for (const auto& p : particulas) {
			glColor4f(p.r, p.g, p.b, p.vida); // El alpha depende de la vida restante
			glVertex3f(p.x, p.y, p.z);
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}
};

SistemaFuegos* SistemaFuegos::instancia = nullptr;

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

void dibujarCuadradoconTextura(float w, float y, float d) {
	float x = w / 2.0f, z = d / 2.0f;

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, y, -z); // Abajo-Izquierda de la foto
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, z);  // Arriba-Izquierda de la foto
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);   // Arriba-Derecha de la foto
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, -z);  // Abajo-Derecha de la foto
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

void dibujarCancha(GLuint textura) {
	glEnable(GL_TEXTURE_2D); // Asegurar que esté activa por si acaso
	glBindTexture(GL_TEXTURE_2D, textura);

	// Setear material neutral para que la textura muestre sus colores reales bajo la luz
	GLfloat blanco[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blanco);
	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();
	dibujarCuadradoconTextura(30, 0, 50);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0); // Buena práctica: desvincular
	glDisable(GL_TEXTURE_2D);
}

class Puntaje {
private:
	int goles;
	int fueras;
	static Puntaje *puntaje;

	Puntaje() {
		goles = fueras = 0;
	}
public:
	static Puntaje *getInstance() {
        if (puntaje == nullptr)
        	puntaje = new Puntaje;
        return puntaje;
    }

	void gol() {
		goles++;
		cout << "Goles: " << goles << endl;
	}

	void fuera() {
		fueras++;
		cout << "Fueras: " << fueras << endl;
	}

	int getGoles() {
		return goles;
	}

	int getFueras() {
		return fueras;
	}
};

Puntaje *Puntaje::puntaje = nullptr;

class Objeto {
protected:
	float x, y, z, ancho, largo, alto, vx, vz;
	int display_list;

public:
	virtual ~Objeto() = default;

	float getX() {
		return x;
	}

	float getZ() {
		return z;
	}

	float getVelocidadX() {
		return vx;
	}

	float getVelocidadZ() {
		return vz;
	}

	float getLargo() {
		return largo;
	}

	float getAncho() {
		return ancho;
	}

	bool interseccionX(float x2, float z2, float radio) {
		float dz = max(0.f, abs(z - z2) - ancho / 2);

	    // Distancia horizontal a cada cara lateral
	    float dx_izq = abs((x - largo / 2) - x2);
	    float dx_der = abs((x + largo / 2) - x2);

	    // Intersecta si alcanza alguna de las dos caras y el z está en rango
	    return (dx_izq * dx_izq + dz * dz <= radio * radio)
	        || (dx_der * dx_der + dz * dz <= radio * radio);
	}

	bool interseccionZ(float x2, float z2, float radio) {
	    float dx = max(0.f, abs(x - x2) - largo / 2);

	    float dz_frente = abs((z - ancho / 2) - z2);
	    float dz_atras  = abs((z + ancho / 2) - z2);

	    return (dx * dx + dz_frente * dz_frente <= radio * radio)
	        || (dx * dx + dz_atras  * dz_atras  <= radio * radio);
	}

	void dibujar()  {
		glPushMatrix();
		glTranslatef(x, y, z);
		glCallList(display_list);
		glPopMatrix();
	}
};

class Golero: public Objeto {
private:
	float velocidad;
	int direccion; // 1 = Derecha, -1 = Izquierda
	const float limite = 4.5f; // El arco mide 12, el golero se mueve en este rango

public:
	Golero() {
		x = 0.0f;
		y = 1.5f;    // Altura media para que toque el suelo
		z = -24.0f;  // Un poquito adelante del arco (que está en -25)
		largo = 2.0;
		alto = 3.0;
		ancho = 1.0;
		vx = velocidad = 8.0f;
		vz = 0;
		direccion = 1;

		display_list = glGenLists(1);

		glNewList(display_list, GL_COMPILE);
		dibujarCubo(largo, alto, ancho);
		glEndList();
	}

	void mover(float dt) {
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
		vx = velocidad * direccion;
	}
};

class Plataforma: public Objeto {
private:
	float v, max_x;
public:
	explicit Plataforma(float max_x): max_x(max_x) {
		display_list = glGenLists(1);

		largo = 5;
		alto = 1;
		ancho = 1;
		x = 0;
		y = alto / 2;
		z = 22;
		vx = v = 15;
		vz = 0;

		glNewList(display_list, GL_COMPILE);
		dibujarCubo(largo, alto, ancho);
		glEndList();
	}

	void mover(float dt, float direccion) {
		x += v * dt * direccion;
		x = min(x, max_x - largo / 2);
		x = max(x, -max_x + largo / 2);
		vx = v * direccion;
	}
};

class Defensa: public Objeto {
public:
	Defensa(float x, float z) {
		display_list = glGenLists(1);

		largo = 3;
		alto = 1;
		ancho = 1;
		this->x = x;
		this->z = z;
		y = alto / 2;
		vx = vz = 0;

		glNewList(display_list, GL_COMPILE);
		dibujarCubo(largo, alto, ancho);
		glEndList();
	}
};

class ListaObjetos {
private:
	vector<Objeto *> objetos;
	static ListaObjetos *listaObjetos;

	ListaObjetos() {}
public:
	static ListaObjetos *getInstance() {
        if (listaObjetos == nullptr)
        	listaObjetos = new ListaObjetos;
        return listaObjetos;
    }

	void agregar(Objeto *o) {
		objetos.push_back(o);
	}

	void borrar(Objeto *o) {
		objetos.erase(remove(objetos.begin(), objetos.end(), o), objetos.end());
		free(o);
	}

	vector<Objeto *> lista() {
		return objetos;
	}
};

ListaObjetos *ListaObjetos::listaObjetos = nullptr;

struct PuntoEstela {
	float x, y, z;
	float vida; // De 1.0 a 0.0
	bool salto;
};

class Pelota {
private:
	float radio;
	float x, y, z, vx, vz, max_x, max_z;
	int display_list;
	ListaObjetos* objetos;
	Puntaje* puntaje;
	std::deque<PuntoEstela> estela;

	// AUMENTADO: De 20 a 45 para tener una cola mucho más larga y visible
	const size_t MAX_PUNTOS_ESTELA = 45;

public:
	// vxi y vzi son las velocidades iniciales en x y en z.
	Pelota(float x, float z, float radio, float vxi, float vzi, float max_x, float max_z)
		: x(x), z(z), radio(radio), vx(vxi), vz(vzi), max_x(max_x), max_z(max_z) {
		display_list = glGenLists(1);
		objetos = ListaObjetos::getInstance();
		puntaje = Puntaje::getInstance();

		GLUquadric* q = gluNewQuadric();
		y = radio;

		glNewList(display_list, GL_COMPILE);
		glPushMatrix();
		gluSphere(q, radio, 20, 20);
		glPopMatrix();
		glEndList();

		gluDeleteQuadric(q);
	}

	void reset() {
		x = 0;
		z = 0;
		estela.clear();
		float angulo_aleatorio = ((rand() % 60) - 30) * M_PI / 180.0f;

		float v = sqrt(vx * vx + vz * vz);
		vx = v * sin(angulo_aleatorio);
		vz = v * cos(angulo_aleatorio);

		if (vz < 0) vz = -vz;
	}

	void mover(float dt) {
		x += dt * vx;
		z += dt * vz;

		bool interseccion = false;

		if (estela.size() < 2) {
			estela.clear();
			estela.push_front({ x, y, z, 1.0f, false });
			estela.push_front({ x, y, z, 1.0f, false });
		}
		else {
			float dx = x - estela[1].x;
			float dz = z - estela[1].z;
			float distSqr = dx * dx + dz * dz;

			if (distSqr > 20.0f) {
				estela.clear();
				estela.push_front({ x, y, z, 1.0f, false });
				estela.push_front({ x, y, z, 1.0f, false });
			}
			// AUMENTADO: Ahora guardamos puntos cuando se separa 0.15f (antes 0.05). 
			// Esto "estira" la estela haciéndola físicamente más larga en el espacio.
			else if (distSqr > 0.15f) {
				estela.push_front({ x, y, z, 1.0f, false });
				if (estela.size() > MAX_PUNTOS_ESTELA) {
					estela.pop_back();
				}
			}
			else {
				estela[0].x = x;
				estela[0].z = z;
				estela[0].vida = 1.0f;
			}
		}

		// DISMINUIDO: Se desvanece más lento (antes 1.5, ahora 0.8)
		for (size_t i = 1; i < estela.size(); i++) {
			estela[i].vida -= dt * 0.8f;
		}

		while (estela.size() > 2 && estela.back().vida <= 0) {
			estela.pop_back();
		}

		// --- DETECCIÓN DE GOL Y BORDES (Intacta) ---
		if (z - radio < -max_z) {
			if (x > -6.0f && x < 6.0f) {
				puntaje->gol();
				SistemaFuegos::getInstance()->crearExplosion(-5.0f, 2.0f, -26.0f);
				SistemaFuegos::getInstance()->crearExplosion(5.0f, 2.0f, -26.0f);
				reset();
				return;
			}
			else {
				z = -max_z + radio;
				vz = -vz;
				interseccion = true;
			}
		}
		if (z + radio > max_z) {
			puntaje->fuera();
			reset();
			return;
		}

		if (max_z < z + radio) {
			z = x = 0;
			interseccion = true;
		}
		else if (z - radio < -max_z) {
			z = -max_z + radio;
			vz = -vz;
			interseccion = true;
		}
		if (max_x < x + radio) {
			x = max_x - radio;
			vx = -vx;
			interseccion = true;
		}
		else if (x - radio < -max_x) {
			x = -max_x + radio;
			vx = -vx;
			interseccion = true;
		}

		for (auto objeto : objetos->lista()) {
			bool inter_objeto = false;
			if (objeto->interseccionX(x, z, radio) && vx * (x - objeto->getX()) <= 0) {
				x -= dt * vx;
				x += dt * objeto->getVelocidadX();
				vx = -vx;
				inter_objeto = interseccion = true;
			}
			if (objeto->interseccionZ(x, z, radio) && vz * (z - objeto->getZ()) <= 0) {
				z -= dt * vz;
				vz = -vz;
				inter_objeto = interseccion = true;
			}

			if (inter_objeto && dynamic_cast<Defensa*>(objeto)) {
				objetos->borrar(objeto);
				break;
			}
		}

		if (interseccion) {
			float a = 0.2 * (float)rand() / RAND_MAX - 0.1;
			float cos_a = cos(a);
			float sin_a = sin(a);
			float nx = vx * cos_a - vz * sin_a;
			float nz = vx * sin_a + vz * cos_a;

			if (nx * vx >= 0 && nz * vz >= 0) {
				vx = nx;
				vz = nz;
			}
		}
	}

	void dibujarEstela() {
		if (estela.size() < 2) return;

		glPushMatrix();

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);

		// MEZCLA ADITIVA: Hace que la estela brille intensamente al sumar colores
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glBegin(GL_QUAD_STRIP);
		for (size_t i = 0; i < estela.size(); i++) {
			float factor = (float)i / (float)(estela.size() - 1);

			// ALFA LINEAL FUERTE: Solo decae por posición, garantizando que siempre se vea
			float alfa = (1.0f - factor);
			// Limitamos un poco si la vida es muy baja para limpiar la cola
			if (estela[i].vida < 0.5f) alfa *= (estela[i].vida * 2.0f);

			// Arranca un 20% más ancha que la pelota y termina en punta
			float anchoActual = radio * (1.2f - factor * 1.2f);
			if (anchoActual < 0.05f) anchoActual = 0.05f;

			float yOffset = estela[i].y;

			float dx = 0.0f, dz = 0.0f;
			if (i == 0) {
				dx = estela[0].x - estela[1].x; dz = estela[0].z - estela[1].z;
			}
			else if (i == estela.size() - 1) {
				dx = estela[i - 1].x - estela[i].x; dz = estela[i - 1].z - estela[i].z;
			}
			else {
				dx = estela[i - 1].x - estela[i + 1].x; dz = estela[i - 1].z - estela[i + 1].z;
			}

			float len = sqrtf(dx * dx + dz * dz);
			float px = 1.0f, pz = 0.0f;

			if (len > 0.001f) {
				dx /= len; dz /= len;
				px = -dz; pz = dx;
			}

			// COLORES VIBRANTES (Blanco -> Amarillo -> Naranja -> Rojo Intenso)
			float r = 1.0f;
			float g = 1.0f - (factor * 1.5f); if (g < 0.0f) g = 0.0f;
			float b = 1.0f - (factor * 4.0f); if (b < 0.0f) b = 0.0f;

			glColor4f(r, g, b, alfa * 0.8f);

			glVertex3f(estela[i].x + px * anchoActual, yOffset, estela[i].z + pz * anchoActual);
			glVertex3f(estela[i].x - px * anchoActual, yOffset, estela[i].z - pz * anchoActual);
		}
		glEnd();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);

		glPopMatrix();
	}

	void dibujar() {
		dibujarEstela();
		glPushMatrix();
		glTranslatef(x, y, z);
		glCallList(display_list);
		glPopMatrix();
	}
};

class Fuente {
private:
	map<int, TTF_Font *> fuentes;
	initializer_list<const char *> rutas;
public:
	explicit Fuente(initializer_list<const char *> rutas) : rutas(rutas) {}

	void openSize(int size) {
		fuentes[size] = nullptr;

		for (const char *ruta : rutas) {
			fuentes[size] = TTF_OpenFont(ruta, size);
			if (fuentes[size])
				break;
		}

		if (!fuentes[size]) {
			cerr << "Error crítico: No se pudo cargar " << *(rutas.begin())
				 << " en ninguna ruta conocida." << endl;
			cerr << "Detalle SDL_ttf: " << TTF_GetError() << endl;
			exit(1);
		}
	}

	~Fuente() {
		for (auto fuente: fuentes)
			if (fuente.second)
				TTF_CloseFont(fuente.second);
	}

	TTF_Font *getFont(int size) {
		if (fuentes.count(size) == 0)
			openSize(size);
		
		return fuentes[size];
	}
};

void renderTexto(const string &texto, int x, int y, TTF_Font *fuente) {
	if (!fuente || texto.empty()) return;

	SDL_Color blanco = { 255, 255, 255, 255 };

	// 1. Renderizamos la superficie original
	SDL_Surface* surfaceOriginal = TTF_RenderText_Blended(fuente, texto.c_str(), blanco);
	if (!surfaceOriginal) return;

	// CAMBIO 1: Convertimos la superficie a un formato fijo (RGBA32).
	// Esto estandariza la memoria para que siempre sea R, G, B, A, sin importar 
	// la arquitectura del sistema, solucionando colores invertidos o alfas que no funcionan.
	SDL_Surface* surface = SDL_ConvertSurfaceFormat(surfaceOriginal, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(surfaceOriginal); // Liberamos la original que ya no precisamos

	GLuint texturaTexto;
	glGenTextures(1, &texturaTexto);
	glBindTexture(GL_TEXTURE_2D, texturaTexto);

	// CAMBIO 2: Alineación de desempaquetado.
	// Le decimos a OpenGL que lea la textura alineada byte a byte (1) en vez de a bloques de 4.
	// Esto arregla el bug donde el texto se ve inclinado o ilegible.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	// CAMBIO 3: Usamos GL_MODULATE y seteamos el color explícitamente a blanco.
	// GL_REPLACE ignora por completo la transparencia si el entorno no está perfecto.
	// Modulate respeta el color y el alfa del texto.
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_QUADS);
	// Las coordenadas de textura estaban bien mapeadas para invertir el eje Y de SDL
	glTexCoord2f(0, 1); glVertex2i(x, y);
	glTexCoord2f(1, 1); glVertex2i(x + surface->w, y);
	glTexCoord2f(1, 0); glVertex2i(x + surface->w, y + surface->h);
	glTexCoord2f(0, 0); glVertex2i(x, y + surface->h);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texturaTexto);

	// CAMBIO 4: Restauramos la alineación por defecto para no afectar
	// la carga de otras texturas (como la cancha) más adelante.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	SDL_FreeSurface(surface);
}

void dibujarHUD(Uint32 tiempoTranscurrido, bool pausa, Puntaje *puntaje, Fuente *fuente, float velocidad) {
	glDisable(GL_LIGHTING); 
	glDisable(GL_DEPTH_TEST);// El HUD no necesita luces
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1000, 0, 700); // Proyección 2D (ancho y alto de tu ventana)

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);

	// Dibujar contenido
	if (pausa)
		renderTexto("Pausa", 450, 660, fuente->getFont(24));

	renderTexto("Goles: " + to_string(puntaje->getGoles()), 20, 660, fuente->getFont(24));
	renderTexto("Tiempo: " + to_string(tiempoTranscurrido) + "s", 850, 660, fuente->getFont(24));

	
	// --- NUEVO: Render de velocidad ---
	// Limitamos a un decimal para que quede estético (ej: "Velocidad: 1.2x")
	char velTexto[20];

	// ¡Esta es la línea que faltaba! Rellena el array con el texto formateado
	snprintf(velTexto, sizeof(velTexto), "Velocidad: %.1fx", velocidad);

	renderTexto(velTexto, 430, 20, fuente->getFont(24));
	

	// Restaurar estados para el próximo frame
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void dibujarMenu(Uint32 tiempoTranscurrido, Fuente *fuente) {
	glDisable(GL_LIGHTING); 
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1000, 0, 700); // Proyección 2D (ancho y alto de tu ventana)

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);
	renderTexto("PILONOID", 230, 500, fuente->getFont(100));
	glColor3f(1.0f, 1.0f, 1.0f);
	if (((int) tiempoTranscurrido) % 2 == 0)
		renderTexto("presiona p para jugar", 380, 400, fuente->getFont(20));
	renderTexto("Juan Andrés Olmedo  -  Francisco Piloni", 270, 100, fuente->getFont(20));

	// Restaurar estados para el próximo frame
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	//INICIALIZACION
	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		cerr << "No se pudo iniciar SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	// Cargar las fuentes con búsqueda en múltiples rutas (para macOS/Windows)
	if (TTF_Init() == -1) {
		cerr << "Error SDL_ttf: " << TTF_GetError() << endl;
		exit(1);
	}

	map<string, Fuente *> fuentes;

	fuentes["arial"] = new Fuente({"arial.ttf", "../arial.ttf", "C:/Windows/Fonts/arial.ttf"});
	fuentes["audiowide"] = new Fuente({"audiowide.ttf", "../audiowide.ttf"});

	SDL_Window* win = SDL_CreateWindow("Lab 1 Comp. Grafica, Juan Andres Olmedo y Francisco Piloni",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1000, 700, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	glMatrixMode(GL_PROJECTION);

	glClearColor(0, 0, 0, 1);

	gluPerspective(45, 1000 / 700.f, 0.1, 110);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);

	// Activar Backface Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Activar iluminación
	glEnable(GL_LIGHTING);

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

	GLuint textura;
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

	// Variables de control y configuración
	bool fin = false,
		 menu = true,
		 pausa = false,
		 texturas = true,
		 wireframe = false;

	// Teclas izquierda/derecha apretadas
	bool left = false,
		 right = false;

	Uint32 tiempoInicial = SDL_GetTicks(),
		   tiempoEnPausa = 0;

	SDL_Event evento;

	float multiplicadorVelocidad = 1.0f;
	const float VEL_MAXIMA = 3.0f;
	const float VEL_MINIMA = 0.2f;
	const float PASO_VELOCIDAD = 0.1f;

	GLfloat luz_posicion[4] = { 20, 5, 30, 1 };
	GLfloat luz_posicion1[4] = { 20, 5, -20, 1 };
	GLfloat luz_posicion2[4] = { -15, 5, 30, 1 };
	GLfloat luz_posicion3[4] = { -15, 5, -20, 1 };
	GLfloat colorLuz[4] = { 1, 1, 1, 1 };
	
	// Variables de juego
	Golero golero;
	Puntaje *puntaje = Puntaje::getInstance();

	Plataforma plataforma(13);
	ListaObjetos *listaObjetos = ListaObjetos::getInstance();

	listaObjetos->agregar(new Defensa(-12, -10));
	listaObjetos->agregar(new Defensa(-8, -10));
	listaObjetos->agregar(new Defensa(-4, -10));
	listaObjetos->agregar(new Defensa(0, -10));
	listaObjetos->agregar(new Defensa(4, -10));
	listaObjetos->agregar(new Defensa(8, -10));
	listaObjetos->agregar(new Defensa(12, -10));
	listaObjetos->agregar(&plataforma);
	listaObjetos->agregar(&golero);

	Pelota pelota(0, 0, 1, 20, 20, 15, 25);

	Uint32 last = SDL_GetTicks();

	// LOOP PRINCIPAL
	do {
		Uint32 now = SDL_GetTicks();
		float deltaTimeReal = (now - last) / 1000.0f;

		// Evitamos picos extraños en el delta time si la ventana se congela un milisegundo
		if (deltaTimeReal > 0.1f) deltaTimeReal = 0.1f;

		// Calculamos el delta escalado para la lógica de movimiento
		float dtEscalado = deltaTimeReal * multiplicadorVelocidad;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		// Actualizar posición de la cámara según modo de vista (Asegúrate de tener definidas estas variables globales)
		float yaw_rad = yaw * M_PI / 180.0f;
		float pitch_rad = pitch * M_PI / 180.0f;

		camX = radio * cos(pitch_rad) * sin(yaw_rad);
		camY = radio * sin(pitch_rad);
		camZ = radio * cos(pitch_rad) * cos(yaw_rad);

		float platX = plataforma.getX();
		float platZ = plataforma.getZ();

		switch (vistaActual) {
		case ORIGINAL:
			gluLookAt(0, 15, 50, 0, 0, 0, 0, 1, 0);
			break;
		case PERSONAJE:
			gluLookAt(platX, 5, platZ + 15, platX, 5, platZ, 0, 1, 0);
			break;
		case LIBRE:
			gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);
			break;
		}

		// Habilitar cuatro luces en las esquinas de la cancha
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);

		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuz);

		glEnable(GL_LIGHT2);
		glLightfv(GL_LIGHT2, GL_POSITION, luz_posicion2);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, colorLuz);

		glEnable(GL_LIGHT3);
		glLightfv(GL_LIGHT3, GL_POSITION, luz_posicion3);
		glLightfv(GL_LIGHT3, GL_DIFFUSE, colorLuz);

		// Configuración de Wireframes (on/off)
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Configuración de texturas (on/off)
		if (texturas)
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);

		luzAmbiente(1, 1, 1);

		if (menu) {
			dibujarMenu((now - tiempoInicial) / 1000, fuentes["audiowide"]);
		}
		else {
			if (pausa) {
				tiempoEnPausa += now - last;
			}
			else {
				// Corregido: Usamos dtEscalado para que la velocidad afecte a todo el entorno uniformemente
				pelota.mover(dtEscalado);
				SistemaFuegos::getInstance()->actualizar(dtEscalado);

				if (right)
					plataforma.mover(dtEscalado, 1);
				else if (left)
					plataforma.mover(dtEscalado, -1);
				else
					plataforma.mover(dtEscalado, 0);

				golero.mover(dtEscalado);
			}


			dibujarCancha(textura);
			dibujarArco();
			
			for (auto objeto : listaObjetos->lista())
				objeto->dibujar();

			golero.dibujar();
			pelota.dibujar();
			SistemaFuegos::getInstance()->dibujar();

			

			// El HUD ya se encarga de renderizar de manera limpia los goles y el tiempo corregido por pausas
			dibujarHUD(((now - tiempoInicial) - tiempoEnPausa) / 1000, pausa, puntaje, fuentes["arial"], multiplicadorVelocidad);
		}
		// ==========================================
		// CONTROL DE EVENTOS DE SDL (Estructura arreglada)
		// ==========================================
		while (SDL_PollEvent(&evento)) {
			switch (evento.type) {
			case SDL_QUIT:
				fin = true;
				break;

			case SDL_MOUSEMOTION:
				if (vistaActual == LIBRE) {
					if (evento.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
						radio += evento.motion.yrel * 0.05f;
						if (radio < 0.5f) radio = 0.5f;
					}
					else {
						// Órbita
						float sensibilidad = 0.2f;
						// Mantener yaw entre -90 y 90, y el pitch mayor a 10
						yaw = max(-90.f, min(yaw + evento.motion.xrel * sensibilidad, 90.f));
						pitch = max(pitch - evento.motion.yrel * sensibilidad, 10.f);
						if (pitch > 89.0f) pitch = 89.0f;
						if (pitch < -89.0f) pitch = -89.0f;
					}
				}
				break;

			case SDL_MOUSEWHEEL:
				if (vistaActual == LIBRE) {
					float scrollSpeed = .5f;
					// Mantener el radio entre 10 y 70
					radio = max(10.f, min(radio - evento.wheel.y * scrollSpeed, 70.f));
				}
				break;

			case SDL_KEYDOWN:
				switch (evento.key.keysym.sym) {
				case SDLK_RIGHT: right = true; break;
				case SDLK_LEFT:  left = true;  break;

					// Controles de velocidad con flechas
				case SDLK_UP:
					multiplicadorVelocidad = min(VEL_MAXIMA, multiplicadorVelocidad + PASO_VELOCIDAD);
					break;
				case SDLK_DOWN:
					multiplicadorVelocidad = max(VEL_MINIMA, multiplicadorVelocidad - PASO_VELOCIDAD);
					break;
				case SDLK_v:
					// Ciclo de cámaras
					if (vistaActual == ORIGINAL) {
						vistaActual = PERSONAJE;
						SDL_SetRelativeMouseMode(SDL_FALSE);
					}
					else if (vistaActual == PERSONAJE) {
						vistaActual = LIBRE;
						SDL_SetRelativeMouseMode(SDL_TRUE);  // Atrapamos cursor en modo libre
					}
					else {
						vistaActual = ORIGINAL;
						SDL_SetRelativeMouseMode(SDL_FALSE);
					}
					break;
				case SDLK_ESCAPE:
				case SDLK_q:
					fin = true;
					break;
				case SDLK_p:
					if (menu) {
						menu = false;
						tiempoInicial = SDL_GetTicks();
						tiempoEnPausa = 0; // Resetear tiempos de partidas previas
					}
					else {
						pausa = !pausa;
					}
					break;
				case SDLK_t:
					if (menu)
						texturas = !texturas;
					break;
				case SDLK_w:
					if (menu)
						wireframe = !wireframe;
					break;
				}
				break;

			case SDL_KEYUP:
				switch (evento.key.keysym.sym) {
				case SDLK_RIGHT:
					right = false;
					break;
				case SDLK_LEFT:
					left = false;
					break;
				}
				break;
			}
		}

		SDL_GL_SwapWindow(win);
		last = now;

	} while (!fin);
	//FIN LOOP PRINCIPAL

	// Limpieza de memoria para punteros creados con 'new'
	for (auto& par : fuentes) {
		delete par.second;
	}
	fuentes.clear();

	TTF_Quit();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}