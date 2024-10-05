#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

class Vector{
public:
	Vector() { coords[0] = 0; coords[1] = 0; coords[2] = 0; }
	Vector(float x, float y, float z) { coords[0] = x; coords[1] = y; coords[2] = z; }

	float& operator[](int index) { return coords[index]; }
	float operator[](int index) const { return coords[index]; }

	float DotProduct(const Vector &v2) { return (coords[0]*v2.coords[0]) + (coords[1]*v2.coords[1]) + (coords[2]*v2.coords[2]); }
	Vector CrossProduct(const Vector &v2) { return Vector(coords[1]*v2.coords[2] - coords[2]*v2.coords[1], coords[2]*v2.coords[0] - coords[0]*v2.coords[2], coords[0]*v2.coords[1] - coords[1]*v2.coords[0]); }

	Vector NormalizeVector() {
		float length = sqrt(coords[0] * coords[0] + coords[1] * coords[1] + coords[2] * coords[2]);
		if(length == 0){
			MessageBox(NULL, "Cannot normalize a zero vector", "Math Error", MB_ICONERROR|MB_OK);
			exit(-1);
		}
		return Vector(coords[0]/length, coords[1]/length, coords[2]/length);
	}

	Vector operator+(const float &f) const { return  Vector(coords[0] + f, coords[1] + f, coords[2] + f); }
	Vector operator+(const Vector &v2) const { return Vector(coords[0] + v2.coords[0], coords[1] + v2.coords[1], coords[2] + v2.coords[2]); }
	
	Vector operator-(const Vector &v2) const { return Vector(coords[0] - v2.coords[0], coords[1] - v2.coords[1], coords[2] - v2.coords[2]); }
	
	Vector operator*(const Vector &v2) const { return Vector(coords[0] * v2.coords[0], coords[1] * v2.coords[1], coords[2] * v2.coords[2]); }
	Vector operator*(const float &f) const { return Vector(coords[0] * f, coords[1] * f, coords[2] * f); }
	
	Vector operator/(const Vector &v2) const { return Vector(coords[0] / v2.coords[0], coords[1] / v2.coords[1], coords[2] / v2.coords[2]); }

	bool operator==(const Vector &v2) const { return coords[0] == v2.coords[0] && coords[1] == v2.coords[1] && coords[2] == v2.coords[2]; }
	bool operator<(const Vector &v2) const { return coords[0] < v2.coords[0] && coords[1] < v2.coords[1] && coords[2] < v2.coords[2]; }
	bool operator>(const Vector &v2) const { return coords[0] > v2.coords[0] && coords[1] > v2.coords[1] && coords[2] > v2.coords[2]; }
	bool operator<=(const Vector &v2) const { return coords[0] <= v2.coords[0] && coords[1] <= v2.coords[1] && coords[2] <= v2.coords[2]; }
	bool operator>=(const Vector &v2) const { return coords[0] >= v2.coords[0] && coords[1] >= v2.coords[1] && coords[2] >= v2.coords[2]; }
private:
	float coords[3];
};

class Vector2D{
public:
	Vector2D() { coords[0] = 0; coords[1] = 0; }
	Vector2D(float x, float y) { coords[0] = x; coords[1] = y; }

	float& operator[](int index) { return coords[index]; }
	float operator[](int index) const { return coords[index]; }

	Vector2D operator+(const Vector2D &v2) const { return Vector2D(coords[0] + v2.coords[0], coords[1] + v2.coords[1]); }

	Vector2D& operator+=(const Vector2D &v2) {
		coords[0] += v2.coords[0];
		coords[1] += v2.coords[1];
		return *this;
	}

	Vector2D operator/(const Vector2D &v2) const { return Vector2D(coords[0] / v2.coords[0], coords[1] / v2.coords[1]); }

	Vector2D& operator/=(const Vector2D &v2) {
		coords[0] /= v2.coords[0];
		coords[1] /= v2.coords[1];
		return *this;
	}
private:
	float coords[2];
};

#endif