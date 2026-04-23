#include <algorithm>

class Plataforma {
private:
	float largo, alto, ancho;
	float x, y, z, v, max_x;
	int display_list;
public:
	explicit Plataforma(float max_x);

	void mover(float dt, float direccion);
	void dibujar();
};