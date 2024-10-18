#pragma warning(disable : 4786)
#include <vector>
#include <unordered_map>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/listbox.h>
#include <wx/dnd.h>
#include <wx/mdi.h>
#include <wx/glcanvas.h>
#include <wx/splitter.h>
#include <wx/splash.h>
#include <wx/aui/aui.h>
#include <wx/bookctrl.h>
#include <wx/propdlg.h>
#include <GL/gl.h>
#include <GL/GLU.h>

#include "mjolnir.h"
#include "vector.h"
#include "camera.h"
#include "fgd.h"
#include "map.h"
#include "wad3.h"

#define ROTATION_SPEED M_PI/180*0.2
#define SPEED 1.8f
#define ZOOM_SPEED 1.0f
#define GRID_SIZE 256.0f
#define SCALE 8.0f

std::vector<Entity> entities;
std::vector<Brush> brushes;
std::unordered_map<std::string, GLuint> texturesGL;
GLuint currentTextureGL;

CCamera camera;

float gridX, gridY;
Vector2D selectionMin, selectionMax;
bool selectionStarted;
bool enterKeyCaptured;
int lastRendermode;

GLuint Image2Texture(unsigned int width, unsigned int height, unsigned char* data)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	GLenum error;

	if (textureID == 0 || (error = glGetError()) != GL_NO_ERROR) {
		char message[64];
		sprintf(message, "Error generating texture! OpenGL Error code: 0x%x.", error);
		MessageBox(NULL, message, "OpenGL Error", MB_ICONERROR | MB_OK);
		exit(-1);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

	return textureID;
}

GLuint GetTexture(const int& index)
{
	Texture texture = textures[index];

	if (!texturesGL.count(texture.name)){
		GLuint tmp_gltexture = Image2Texture(texture.guiImage.GetWidth(), texture.guiImage.GetHeight(), texture.guiImage.GetData());
		texturesGL[texture.name] = tmp_gltexture;
		return texturesGL[texture.name];
	}
	else {
		return texturesGL[texture.name];
	}
}

MapView3D::MapView3D(wxSplitterWindow* parent)
: wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"))
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	movementTimer = new wxTimer(this);
	camera.Init();
	movementTimer->Start(1);
}

MapView3D::~MapView3D()
{
	if(movementTimer){
		movementTimer->Stop();
		delete movementTimer;
	}
}

void CreateCube(Vector min, Vector max)
{
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(min[0], min[1], max[2]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(min[0], max[1], max[2]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(max[0], max[1], max[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(max[0], min[1], max[2]);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(min[0], min[1], max[2]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(min[0], max[1], max[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(min[0], max[1], min[2]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(min[0], min[1], min[2]);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(max[0], max[1], max[2]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(max[0], min[1], max[2]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(max[0], min[1], min[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(max[0], max[1], min[2]);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(max[0], min[1], max[2]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(min[0], min[1], max[2]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(min[0], min[1], min[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(max[0], min[1], min[2]);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(min[0], max[1], max[2]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(max[0], max[1], max[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(max[0], max[1], min[2]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(min[0], max[1], min[2]);
	glEnd();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(min[0], min[1], min[2]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(min[0], max[1], min[2]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(max[0], max[1], min[2]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(max[0], min[1], min[2]);
	glEnd();
}

void RenderObjects()
{
	for(int i = 0; i < entities.size(); i++){
		Entity entity = entities[i];

		glPushMatrix();

		glTranslatef(entity.pos[0], entity.pos[1], entity.pos[2]);
		glColor3f(entity.pointclass.color[0], entity.pointclass.color[1], entity.pointclass.color[2]);

		glBindTexture(GL_TEXTURE_2D, NULL);
		CreateCube(entity.pointclass.sizeMin, entity.pointclass.sizeMax);

		glColor3f(1.0f, 1.0f, 1.0f);

		glPopMatrix();
	}

	for(int i = 0; i < brushes.size(); i++){
		Brush brush = brushes[i];

		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, brush.texture);
		CreateCube(brush.min, brush.max);

		glPopMatrix();
	}

	glColor3f(1.0f, 1.0f, 1.0f);
}

void MapView3D::OnEraseBackground(wxEraseEvent& event)
{
    event.Skip();
}

void MapView3D::OnMouseMotion(wxMouseEvent& event)
{
	SetFocus();

	//I have no idea why the fuck I can't use EVT_LEFT_DOWN and EVT_LEFT_UP for this.
	//When I try to use both, nothing happens, when I tested with only EVT_LEFT_DOWN,
	//for some strange reason the camera only moves after I release the left mouse button,
	//even if EVT_LEFT_UP was removed.

	//Apparently I have to keep it all in EVT_MOTION... is this truly a bad design of wxWidgets or I'm just a noob at it?

	if(event.LeftIsDown() && tbar->GetToolState(TOOL_CAMERA)){
		if (!HasCapture()) {
            CaptureMouse();
			ShowCursor(0);
        }

		int width, height;
		GetClientSize(&width, &height);

		int centerX = width / 2;
		int centerY = height / 2;

		int mouseX = event.GetX();
		int mouseY = event.GetY();
			
		int dx = mouseX - centerX;
		int dy = mouseY - centerY;

		if(dx != 0 || dy != 0){
			camera.RotateYaw(ROTATION_SPEED * dx);
			camera.RotatePitch(ROTATION_SPEED * dy);

			WarpPointer(centerX, centerY);
		}

		Refresh();
	} else if (HasCapture()){
		ReleaseMouse();
		ShowCursor(1);
	}

	event.Skip();
}

void MapView3D::OnMovementTimer(wxTimerEvent& event)
{
	if(FindFocus() == this){
		if(wxGetKeyState((wxKeyCode)'w') || wxGetKeyState((wxKeyCode)'W'))
			camera.Move(SPEED, true);
		if(wxGetKeyState((wxKeyCode)'s') || wxGetKeyState((wxKeyCode)'S'))
			camera.Move(SPEED, false);
		if(wxGetKeyState((wxKeyCode)'a') || wxGetKeyState((wxKeyCode)'A'))
			camera.Strafe(SPEED, true);
		if(wxGetKeyState((wxKeyCode)'d') || wxGetKeyState((wxKeyCode)'D'))
			camera.Strafe(SPEED, false);

		Refresh();

		event.Skip();
	}
}

void MapView3D::Render(wxPaintEvent& event)
{
    SetCurrent();
    wxPaintDC(this);

	int width, height;
	GetClientSize(&width, &height);

	currentTextureGL = GetTexture(currentTexture);

	glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float ratio = width/height;
	gluPerspective(45, ratio, 0.1, 640);

	camera.Refresh();

	glRotatef(90.0f, -1.0f, 0.0f, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(64.0f, 0.0f, 0.0f);
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,64.0f,0.0f);
	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,64.0f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);

	RenderObjects();

	glDisable(GL_TEXTURE_2D);

    glFlush();
    SwapBuffers();
    event.Skip();
}

BEGIN_EVENT_TABLE(MapView3D, wxGLCanvas)
	EVT_MOTION(MapView3D::OnMouseMotion)
	EVT_TIMER(wxID_ANY, MapView3D::OnMovementTimer)
	EVT_PAINT(MapView3D::Render)
    EVT_ERASE_BACKGROUND(MapView3D::OnEraseBackground)
END_EVENT_TABLE()

MapView2D::MapView2D(wxScrolledWindow* parent)
: wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas")), zoom(1.0f), scrollWin(parent)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	scrollWin->SetScrollbars(1, 1, 1000, 1000);
	SetSize(1000,1000);
	enterKeyHandled = false;
	readyToPlace = false;
	selectionStarted = false;
}

void MapView2D::OnEraseBackground(wxEraseEvent& event)
{
    event.Skip();
}

float Translate(float value, float fromMin, float fromMax, float toMin, float toMax)
{
    float fromSpan = fromMax - fromMin;
    float toSpan = toMax - toMin;

    float valueScaled = (value - fromMin) / fromSpan;

    return toMin + (valueScaled * toSpan);
}

void MapView2D::ConvertWindowToGridCoordinates(int mouseX, int mouseY, float& gridX, float& gridY)
{
	wxSize clientSize = GetClientSize();
    int canvasWidth = clientSize.GetWidth();
    int canvasHeight = clientSize.GetHeight();

	float invertedMouseY = canvasHeight - mouseY;

	float normalizedX = (mouseX / (float)canvasWidth);
    float normalizedY = (invertedMouseY / (float)canvasHeight);

	gridX = Translate(normalizedX * canvasWidth, 0.0f, canvasWidth, offsetX - 10.0 * zoom, offsetX + 10.0 * zoom) / SCALE;
	gridY = Translate(normalizedY * canvasHeight, 0.0f, canvasHeight, offsetY - 10.0 * zoom, offsetY + 10.0 * zoom) / SCALE;

	gridX = floor(gridX) * SCALE;
	gridY = floor(gridY) * SCALE;

	if(gridX >= GRID_SIZE/2)
		gridX = GRID_SIZE/2;
	else if(gridX <= -GRID_SIZE/2)
		gridX = -GRID_SIZE/2;

	if(gridY >= GRID_SIZE/2)
		gridY = GRID_SIZE/2;
	else if(gridY <= -GRID_SIZE/2)
		gridY = -GRID_SIZE/2;
}

void MapView2D::Render()
{
	GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(offsetX - 10.0 * zoom, offsetX + 10.0 * zoom, offsetY - 10.0 * zoom, offsetY + 10.0 * zoom, -640.0, 640.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_LINES);
	for(float i = -GRID_SIZE/2.0f; i <= GRID_SIZE/2.0f; i += SCALE){
		if(i == 0)
			glColor3f(0.000f, 0.000f, 0.804f);
		else
			glColor3f(0.5f, 0.5f, 0.5f);

		glVertex3f(i, -GRID_SIZE/2.0f, 0.0f);
		glVertex3f(i, GRID_SIZE/2.0f, 0.0f);
		glVertex3f(-GRID_SIZE/2.0f, i, 0.0f);
		glVertex3f(GRID_SIZE/2.0f, i, 0.0f);
	}
	glEnd();

	if(readyToPlace && tbar->GetToolState(TOOL_ENTITY)){
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
			glVertex3f(gridX, -GRID_SIZE/2, 0.0f);
			glVertex3f(gridX, GRID_SIZE/2, 0.0f);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(-GRID_SIZE/2, gridY, 0.0f);
			glVertex3f(GRID_SIZE/2, gridY, 0.0f);
		glEnd();
	}

	if(tbar->GetToolState(TOOL_SELECT))
		glColor3f(0.0f, 1.0f, 1.0f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(selectionMin[0], selectionMin[1], 0.0f);
		glVertex3f(selectionMin[0], selectionMax[1], 0.0f);
		glVertex3f(selectionMax[0], selectionMax[1], 0.0f);
		glVertex3f(selectionMax[0], selectionMin[1], 0.0f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	if(rendermode == VIEW_SIDE)
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
	else if(rendermode == VIEW_FRONT){
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, -1.0f);
	}

	if(enterKeyHandled && !enterKeyCaptured){
		if(tbar->GetToolState(TOOL_CBLOCK) && readyToPlace){
			Brush tmp_brush;

			if(lastRendermode == VIEW_TOP){
				tmp_brush.min = Vector(selectionMin[0], selectionMin[1], -8.0f);
				tmp_brush.max = Vector(selectionMax[0], selectionMax[1], 0.0f);
				tmp_brush.texture = currentTextureGL;
			} else if(lastRendermode == VIEW_FRONT){
				tmp_brush.min = Vector(-8.0f, selectionMin[0], selectionMin[1]);
				tmp_brush.max = Vector(0.0f, selectionMax[0], selectionMax[1]);
				tmp_brush.texture = currentTextureGL;
			} else{
				tmp_brush.min = Vector(selectionMin[0], -8.0f, selectionMin[1]);
				tmp_brush.max = Vector(selectionMax[0], 0.0f, selectionMax[1]);
				tmp_brush.texture = currentTextureGL;
			}

			brushes.push_back(tmp_brush);

			enterKeyHandled = false;
			enterKeyCaptured = true;
			readyToPlace = false;
		}
	}

	RenderObjects();

	glFlush();
    SwapBuffers();
}

void MapView2D::OnMouseWheel(wxMouseEvent& event)
{
	int delta = event.GetWheelRotation();

	int mouseX = event.GetX();
	int mouseY = event.GetY();

	ConvertWindowToGridCoordinates(mouseX, mouseY, gridX, gridY);

	float newZoom = zoom + (delta > 0 ? -ZOOM_SPEED : ZOOM_SPEED);
    if (newZoom < 0.1f) newZoom = 0.1f;
    if (newZoom > 50.0f) newZoom = 50.0f;

	float zoomFactor = newZoom / zoom;

	offsetX += (gridX - offsetX) * (1 - zoomFactor);
	offsetY += (gridY - offsetY) * (1 - zoomFactor);

	zoom = newZoom;

	char coords[24];
	sprintf(coords, "%.2f %.2f", gridX, gridY);
	frame->SetStatusText(coords, 1);

	Refresh();
}

void MapView2D::OnMouseMotion(wxMouseEvent& event)
{
    SetFocus();

    int mouseX = event.GetX();
    int mouseY = event.GetY();
    ConvertWindowToGridCoordinates(mouseX, mouseY, gridX, gridY);

    char coords[24];
    sprintf(coords, "%.2f %.2f", gridX, gridY);
    frame->SetStatusText(coords, 1);

    if (tbar->GetToolState(TOOL_SELECT) || tbar->GetToolState(TOOL_CBLOCK)) {
        if (selectionStarted) {
            selectionMax = Vector2D(gridX, gridY);
            if (tbar->GetToolState(TOOL_CBLOCK))
                readyToPlace = true;
        }
    }

    if (tbar->GetToolState(TOOL_ENTITY))
        readyToPlace = true;

    Refresh();
}

void MapView2D::OnMouseDown(wxMouseEvent& event)
{
	if(tbar->GetToolState(TOOL_ENTITY)){
		Entity tmp_entity;
		tmp_entity.pointclass = pcMap[std::string(entityChoice->GetString(entityChoice->GetSelection()))];

		if(rendermode == VIEW_TOP)
			tmp_entity.pos = Vector(gridX,gridY,0.0f);
		else if(rendermode == VIEW_FRONT)
			tmp_entity.pos = Vector(0.0f,gridX,gridY);
		else
			tmp_entity.pos = Vector(gridX,0.0f,gridY);

		entities.push_back(tmp_entity);
	} else if(tbar->GetToolState(TOOL_SELECT) || tbar->GetToolState(TOOL_CBLOCK)){
		selectionMin = Vector2D(gridX, gridY);
		selectionStarted = true;
	}
}

void MapView2D::OnMouseUp(wxMouseEvent& event)
{
	selectionStarted = false;

	if(selectionMin[0] == selectionMax[0] || selectionMin[1] == selectionMax[1]){
		selectionMin = Vector2D(0.0f, 0.0f);
		selectionMax = Vector2D(0.0f, 0.0f);
	}

	lastRendermode = rendermode;

	Refresh();
}

void MapView2D::OnPaint(wxPaintEvent& event)
{
    SetCurrent();
    wxPaintDC(this);

	Render();
    
    event.Skip();
}

BEGIN_EVENT_TABLE(MapView2D, wxGLCanvas)
	EVT_PAINT(MapView2D::OnPaint)
	EVT_MOUSEWHEEL(MapView2D::OnMouseWheel)
	EVT_LEFT_DOWN(MapView2D::OnMouseDown)
	EVT_LEFT_UP(MapView2D::OnMouseUp)
	EVT_MOTION(MapView2D::OnMouseMotion)
    EVT_ERASE_BACKGROUND(MapView2D::OnEraseBackground)
END_EVENT_TABLE()