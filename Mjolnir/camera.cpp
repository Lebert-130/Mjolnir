#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void CCamera::Init()
{
	m_yaw = 0.0f;
	m_pitch = 0.0f;
}

void CCamera::Refresh()
{
	m_lx = cos(m_yaw) * cos(m_pitch);
	m_ly = sin(m_pitch);
	m_lz = sin(m_yaw) * cos(m_pitch);

	m_strafe_lx = cos(m_yaw - M_PI/2);
	m_strafe_lz = sin(m_yaw - M_PI/2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_x, m_y, m_z, m_x + m_lx, m_y + m_ly, m_z + m_lz, 0.0, 1.0, 0.0);
}

void CCamera::SetPos(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
	Refresh();
}

void CCamera::GetPos(float &x, float &y, float &z)
{
	x = m_x;
	y = m_y;
	z = m_z;
}

void CCamera::Move(float speed, bool positive)
{
	if(!positive)
		speed = -speed;

	float lx = cos(m_yaw)*cos(m_pitch);
    float ly = sin(m_pitch);
    float lz = sin(m_yaw)*cos(m_pitch);

	m_x = m_x + speed * lx;
	m_y = m_y + speed * ly;
	m_z = m_z + speed * lz;
}

void CCamera::Strafe(float speed, bool positive)
{
	if(!positive)
		speed = -speed;

	m_x = m_x + speed * m_strafe_lx;
	m_z = m_z + speed * m_strafe_lz;
}

void CCamera::RotateYaw(float angle)
{
	m_yaw += angle;
}

void CCamera::RotatePitch(float angle)
{
	const float limit = 89.0 * M_PI / 180.0;

	m_pitch -= angle;

    if(m_pitch < -limit)
        m_pitch = -limit;

    if(m_pitch > limit)
        m_pitch = limit;
}

void CCamera::SetYaw(float angle)
{
    m_yaw = angle;
    Refresh();
}

float CCamera::GetYaw(){
    return m_yaw;
}

void CCamera::SetPitch(float angle)
{
    m_pitch = angle;
    Refresh();
}

float CCamera::GetPitch(){
    return m_pitch;
}