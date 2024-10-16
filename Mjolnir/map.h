#ifndef MAP_H
#define MAP_H

typedef struct {
	PointClass pointclass;
	Vector pos;
} Entity;

typedef struct {
	Vector min;
	Vector max;
	GLuint texture;
} Brush;

extern bool enterKeyCaptured;

#endif